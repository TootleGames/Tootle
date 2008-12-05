
#include "TSceneNode_Camera.h"

using namespace TLScene;

TSceneNode_Camera::TSceneNode_Camera(TRefRef NodeRef,TRefRef TypeRef) :
  TSceneNode_Transform(NodeRef,TypeRef),
  m_fTargetDistance(0.0f),		// Calculated distance to target
  m_fMagnification(0.0f),		// Zoom
  m_fLensSize(35.0f),			// Lens size - 35mm etc 
  m_fFNumber(0),				// f-number - ratio of focal length to aperture width
  m_fFocalLength(50.0f),		// The focal length
  m_fAperture(0),				// f-Stop value
  m_Mode(Manual)				// Camera mode - auto, manual
{
	// Initialise the FOV and DOF
	CalculateFOV();
	CalculateDOF();
}

void TSceneNode_Camera::Update(float fTimeStep)
{
	if(m_Mode == Auto)
	{
	}
}

void TSceneNode_Camera::ProcessMessage(TPtr<TLMessaging::TMessage>& pMessage)
{
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
TPtr<TLScene::TSceneNode_Transform> TSceneNode_Camera::GetTargetobject()
{
	return TPtr<TLScene::TSceneNode_Transform>(NULL);
}
