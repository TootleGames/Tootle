
#include "TSceneNode_Camera.h"

#include "TScenegraph.h"

#include <TootleRender/TRenderTarget.h>//TEMP
#include <TootleRender/TCamera.h> // TEMP
#include <TootleRender/TScreenManager.h> // TEMP


using namespace TLScene;

TSceneNode_Camera::TSceneNode_Camera(TRefRef NodeRef,TRefRef TypeRef) :
  TSceneNode_Transform(NodeRef,TypeRef),
  m_fTargetDistance(0.0f),		// Calculated distance to target
  m_fMagnification(0.0f),		// Zoom
  m_fLensSize(35.0f),			// Lens size - 35mm etc 
  m_fFNumber(0),				// f-number - ratio of focal length to aperture width
  m_fFocalLength(50.0f),		// The focal length
  m_fAperture(0)				// f-Stop value
{
	// Initialise the FOV and DOF
	CalculateFOV();
	CalculateDOF();
	
	AddMode<TCameraState_Manual>("Manual");
	AddMode<TCameraState_Auto>("Auto");
	
	//TODO: This need to be a property setup via reflection
	m_RenderTargetRef = "Game";
}


void TSceneNode_Camera::Initialise(TPtr<TLMessaging::TMessage>& pMessage)
{
	TSceneNode_Transform::Initialise(pMessage);
}

void TSceneNode_Camera::Update(float fTimeStep)
{
	TStateMachine::Update();
	
	// Update the camera on the render target
	UpdateRenderTargetCamera();
}

void TSceneNode_Camera::ProcessMessage(TPtr<TLMessaging::TMessage>& pMessage)
{
	TSceneNode_Transform::ProcessMessage(pMessage);
}

/*
	Calculates the field of view from the current camera settings
*/
void TSceneNode_Camera::CalculateFOV()
{
	// First get the image format eg For a 35mm film the format is 35mm x 24mm.

	float fh = 35;		// Horizontal image size
	float fv = 24;		// Vertical image size

	float fd = (fh * fh) + (fv * fv);	// Diagonal image size
	fd = sqrt(fd);

	float f2FocalLength = 2*m_fFocalLength;

	m_fHFOV = 2 * atan( fh / f2FocalLength );
	m_fVFOV = 2 * atan( fv / f2FocalLength );
	m_fDFOV = 2 * atan( fd / f2FocalLength );

	/*
		NOTE: Apart from wide angle lenses these values can be aproximated for each dimension
			  as follows:

		d FOV = d / focal length;
	*/
	
}


/*
	Calculates the depth of field from the current camera settings
*/
void TSceneNode_Camera::CalculateDOF()
{
}

/*
	Get a pointer to the camera's target obejct
*/
TPtr<TLScene::TSceneNode_Transform> TSceneNode_Camera::GetTargetObject()
{
	// No target?
	if(!m_TargetNodeRef.IsValid())
		return TPtr<TLScene::TSceneNode_Transform>(NULL);
	
	// Search graph
	return TLScene::g_pScenegraph->FindNode(m_TargetNodeRef);
}


// NOTE: This should really be done via a 'camera manager'
//		 It needs to take into account multiple cameras, switching to different cameras
//		 and initial render target camera assignment
void TSceneNode_Camera::UpdateRenderTargetCamera()
{
	TPtr<TLRender::TScreen> pScreen = TLRender::g_pScreenManager->GetInstance("Screen",FALSE);
	
	if(!pScreen)
	{
		TLDebug_Break("No screen for camera to use");
		return;
	}
	
	TPtr<TLRender::TRenderTarget> pRenderTarget = pScreen ? pScreen->GetRenderTarget(m_RenderTargetRef) : TPtr<TLRender::TRenderTarget>(NULL);
	
	if(!pRenderTarget)
	{
		return;
	}
	
	float3 NodePos;
	GetPosition( NodePos );
	
	float3 CamPos = pRenderTarget->GetCamera()->GetPosition();
	CamPos.x = -NodePos.x;
	CamPos.y = -NodePos.y;
	pRenderTarget->GetCamera()->SetPosition( CamPos );
	
	// Get the look at position from our target object
	TPtr<TLScene::TSceneNode_Transform> pTarget = GetTargetObject();
	if(pTarget)
	{
		pTarget->GetPosition(NodePos);
	}
	
	
	//TODO: Take into account an offset and interp the look at rather than simple set
	
	float3 CamLookAt = pRenderTarget->GetCamera()->GetLookAt();
	CamLookAt.x = -NodePos.x;
	CamLookAt.y = -NodePos.y;
	pRenderTarget->GetCamera()->SetLookAt( CamLookAt );
}




Bool TSceneNode_Camera::TCameraState_Auto::OnBegin(TRefRef PreviousMode)
{
	return TRUE;
}

TRef TSceneNode_Camera::TCameraState_Auto::Update()
{
	return TRef();
}


Bool TSceneNode_Camera::TCameraState_Manual::OnBegin(TRefRef PreviousMode)
{
	// Subscribe to the usermanager
	TLMessaging::g_pEventChannelManager->SubscribeTo(this, "USERMANAGER", "ACTION"); 
	
	return TRUE;
}

TRef TSceneNode_Camera::TCameraState_Manual::Update()
{
	return TRef();
}


void TSceneNode_Camera::TCameraState_Manual::ProcessMessage(TPtr<TLMessaging::TMessage>& pMessage)
{
	TRef refInputAction;
	if(pMessage->Read(refInputAction))
	{
		TSceneNode_Camera* pCamera = GetStateMachine<TSceneNode_Camera>();

		float fValue;
		
		// move the camera
		if(pMessage->ImportData("RAWDATA", fValue))
		{
			float3 vTranslate = pCamera->GetTranslate();
			
			// Scale back up the raw value of axis movement
			fValue *= 100.0f;

			if(refInputAction == "CAMXTRANSFORM")
			{
				vTranslate.x += fValue;
			}
			else if(refInputAction == "CAMYTRANSFORM")
			{
				vTranslate.y += fValue;
			}
			else if(refInputAction == "CAMZTRANSFORM")
			{
				vTranslate.z += fValue;
			}
			
			pCamera->SetTranslate(vTranslate);
		}		
	}
		
}


void TSceneNode_Camera::TCameraState_Manual::OnEnd(TRefRef NextMode)				
{
	// Unsubscribe from the usermanager
	TLMessaging::g_pEventChannelManager->UnsubscribeFrom(this, "USERMANAGER", "ACTION"); 
}	



