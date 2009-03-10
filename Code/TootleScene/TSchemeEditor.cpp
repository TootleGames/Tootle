#include "TSchemeEditor.h"

#include <TootleInput/TUser.h>
#include <TootleCore/TLMaths.h>
#include <TootleCore/TEventChannel.h>

#include <TootleRender/TScreenManager.h>
#include <TootleRender/TRenderTarget.h>

#include "TSceneNode_Transform.h"

namespace TLScene
{
	TPtr<TSchemeEditor> g_pSchemeEditor = NULL;
}

using namespace TLScene;



TSchemeEditor::TSchemeEditor(TRef refManagerID) :
  TManager(refManagerID),
  m_uTransformMode(Translate),
  m_bEnabled(TRUE)
{
}


SyncBool TSchemeEditor::Initialise() 
{	
	TPtr<TLRender::TScreen> pScreen = TLRender::g_pScreenManager->GetInstance("Screen");

	if(!pScreen)
		return SyncWait;

	// Do we have an editor 'window'?  If not create one and initialise it
	if(pScreen->GetRenderTarget("Editor") == NULL)
	{
		// Initialise the editor tools window
		TPtr<TLRender::TRenderTarget>	pSchemeEditorWindow = pScreen->CreateRenderTarget( TRef("Editor") );

		if(!pSchemeEditorWindow)
			return SyncFalse;

	//	SyncBool Result = pSchemeEditorWindow->Init();
	//	if ( Result == SyncFalse )
	//		return SyncFalse;
	}

	return SyncTrue; 
}

void TSchemeEditor::OnEventChannelAdded(TRefRef refPublisherID, TRefRef refChannelID)
{
	if(refPublisherID == "USERMANAGER")
	{
		// Subscribe to the input action messages
		if(refChannelID == "Action")
			TLMessaging::g_pEventChannelManager->SubscribeTo(this, refPublisherID, refChannelID); 
	}
	
	// Super event channel routine
	TManager::OnEventChannelAdded(refPublisherID, refChannelID);
}


SyncBool TSchemeEditor::Shutdown()
{ 
	m_SelectedNodes.Empty();

	return SyncTrue; 
}

void TSchemeEditor::ProcessMessage(TLMessaging::TMessage& Message)
{
	TRefRef MessageRef = Message.GetMessageRef();

	if(MessageRef == "Action")
	{
		TRef refInputAction;
		if(Message.Read(refInputAction))
		{
			if(IsEnabled())
			{
				if(refInputAction == "EDSELECT")
				{
					// Clear the selected objects list first
					m_SelectedNodes.Empty();

					int2 CursorPosition;

					if(Message.ImportData("CURSOR", CursorPosition))
					{
						PickObjects(CursorPosition, "Balls");
					}
				}
				else if (refInputAction == "EDS2")
				{
					// Add/Remove nodes
					int2 CursorPosition;

					if(Message.ImportData("CURSOR", CursorPosition))
					{
						AddRemoveObjects(CursorPosition, "Balls");
					}
				}
				else if(refInputAction == "EDYTRANSFORM")
				{
					TransformSelectedNodes(Message, AXIS_Y);
				}
				else if(refInputAction == "EDXTRANSFORM")
				{
					TransformSelectedNodes(Message, AXIS_X);
				}
				else if(refInputAction == "EDDEL")
				{
					if(m_SelectedNodes.GetSize() > 0)
					{
						// Delete selected object(s)
						DeleteSelectedNodes();
					}
				}
			}
			
			if(refInputAction == "EDENABLE")
				ToggleEnabled();

		}
	}

	TManager::ProcessMessage(Message);
}

void TSchemeEditor::PickObjects(Type2<s32> CursorPosition, TRef refRenderTargetID)
{
	TPtr<TLRender::TScreen> pScreen = TLRender::g_pScreenManager->GetInstance("Screen");

	if(!pScreen)
	{
		//	gr: removed break, this kept going off after screen got destroyed
#ifdef _DEBUG
	//	TLDebug_Break("TSchemeEditor::PickObjects - Failed to get screen!");
#endif
		return;
	}

	// Get 3d world pos from cursor pos
	// NOTE: May need multiple window checks/rays if we support 
	// split screen.  At this stage asusme only one window
	TLMaths::TLine WorldRay;
	TPtr<TLRender::TRenderTarget> pRenderTarget = pScreen->GetRenderTarget(refRenderTargetID);
	if ( !pRenderTarget )
		return;

	if ( pScreen->GetWorldRayFromScreenPos( pRenderTarget, WorldRay, CursorPosition ) )
	{
		// Find an object for selection
		TPtrArray<TLScene::TSceneNode_Transform> pNodes;
		if(g_pScenegraph->GetNearestNodes(WorldRay, 1.0f, pNodes))
		{
			// Add to our selected objects list

			for(u32 uIndex = 0; uIndex < pNodes.GetSize(); uIndex++)
			{
				m_SelectedNodes.Add(pNodes.ElementAt(uIndex)->GetNodeRef());

				// Highlight the nodes with a marker that can be used to move the
				// nodes around
				CreateNodeTransformTools();
			}
		}
	}
}


void TSchemeEditor::AddRemoveObjects(Type2<s32> CursorPosition, TRef refRenderTargetID)
{
	TPtr<TLRender::TScreen> pScreen = TLRender::g_pScreenManager->GetInstance("Screen");

	if(!pScreen)
	{
		//	gr: removed break, this kept going off after screen got destroyed
#ifdef _DEBUG
	//	TLDebug_Break("TSchemeEditor::PickObjects - Failed to get screen!");
#endif
		return;
	}


	// Get 3d world pos from cursor pos
	// NOTE: May need multiple window checks/rays if we support 
	// split screen.  At this stage asusme only one window
	TLMaths::TLine WorldRay;
	TPtr<TLRender::TRenderTarget> pRenderTarget = pScreen->GetRenderTarget(refRenderTargetID);
	if ( !pRenderTarget )
		return;

	if ( pScreen->GetWorldRayFromScreenPos( pRenderTarget, WorldRay, CursorPosition ) )
	{
		// Find an object for selection
		TPtrArray<TLScene::TSceneNode_Transform> pNodes;
		if(g_pScenegraph->GetNearestNodes(WorldRay, 1.0f, pNodes))
		{
			// Add to our selected objects list

			for(u32 uIndex = 0; uIndex < pNodes.GetSize(); uIndex++)
			{
				TPtr<TLScene::TSceneNode_Transform> pNode = pNodes.ElementAt(uIndex);

				s32 uNodeIndex = -1;
				for(u32 uIndex2 = 0; uIndex2 < m_SelectedNodes.GetSize(); uIndex2++)
				{
					// Search for the node already being in the list of nodes
					// If it is then set the index and allow it to be removed
					if(m_SelectedNodes.ElementAt(uIndex2) == pNode->GetNodeRef())
					{
						uNodeIndex = (s32) uIndex2;
						break;
					}
				}

				if(uNodeIndex == -1)
				{
					m_SelectedNodes.Add(pNode->GetNodeRef());
				}
				else 
				{
					m_SelectedNodes.Remove(pNode->GetNodeRef());
				}
			}

			// Highlight the nodes with a marker that can be used to move the
			// nodes around
			CreateNodeTransformTools();
		}
	}
}



void TSchemeEditor::CreateNodeTransformTools()
{

}


void TSchemeEditor::RemoveNodeTransformTools()
{

}


void TSchemeEditor::TransformSelectedNodes(TLMessaging::TMessage& Message, TransformAxis uAxis)
{
	if(m_SelectedNodes.GetSize() > 0)
	{
		/*
		//OLD METHOD - 2d transform doesn;t work in 3d
		// Get the amount to transform the nodes by from the message
		float fAmount = 0.0f;

		if(Message.ImportData("RAWDATA", fAmount))
		{
			// NOTE: Scale the value up - axis values are scaled down
			//fAmount *= 100.0f;
			fAmount *= 10.0f;	// Temp scale for now because it kindof works

			// Axis movements are arbitrary relative values that have been normalised to 0.0f-1.0f.
			// Need to convert these into 3d space for the values to have any meaning in 3d

			// Do the transform depending on the current transform mode
			TransformSelectedNodes(fAmount, uAxis);
		}
		*/

		int2	CursorPosition;

		if(Message.ImportData("CURSOR", CursorPosition))
		{
			// Find world pos in 3d from 2d screen pos
			TPtr<TLRender::TScreen> pScreen = TLRender::g_pScreenManager->GetInstance("Screen");

			if(!pScreen)
			{
				//	gr: removed break, this kept going off after screen got destroyed
		#ifdef _DEBUG
			//	TLDebug_Break("TSchemeEditor::PickObjects - Failed to get screen!");
		#endif
				return;
			}


			int2 PreviousCursorPosition = CursorPosition;
			float fAmount = 0.0f;
			if(Message.ImportData("RAWDATA", fAmount))
			{
				fAmount *= 100.0f;

				if(uAxis == AXIS_X)
					PreviousCursorPosition.x -= (int)fAmount;
				else if(uAxis == AXIS_Y)
					PreviousCursorPosition.y -= (int)fAmount;
				//else
				//	PreviousCursorPosition.z -= fAmount;
			}
			// Get 3d world pos from cursor pos
			// NOTE: May need multiple window checks/rays if we support 
			// split screen.  At this stage assume only one window
			TLMaths::TLine WorldRay;

			// TODO: Need to change teh way the render target is specified so it is more dynamic
			TPtr<TLRender::TRenderTarget> pRenderTarget = pScreen->GetRenderTarget("Balls");

			float3 vNewPos;

			if ( pScreen->GetWorldRayFromScreenPos( pRenderTarget, WorldRay, CursorPosition ) )
			{
				float3 vNewPos = WorldRay.GetStart();

				if ( pScreen->GetWorldRayFromScreenPos( pRenderTarget, WorldRay, PreviousCursorPosition ) )
				{
					float3 vPreviousPos = WorldRay.GetStart();

					// Get the difference in 3d space
					if(uAxis == AXIS_X)
						fAmount = vNewPos.x - vPreviousPos.x;
					else if(uAxis == AXIS_Y)
						fAmount = vNewPos.y - vPreviousPos.y;
					//else
					//	fAmount = ;

					// Find change in axis
					TransformSelectedNodes(fAmount, uAxis);
				}
			}
		}

	}
}

void TSchemeEditor::TransformSelectedNodes(float fAmount, TransformAxis uAxis)
{
	switch(m_uTransformMode)
	{
		case Translate:
			TranslateSelectedNodes(fAmount, uAxis);
			break;
		case Rotate:
			RotateSelectedNodes(fAmount, uAxis);
			break;
		case Scale:
			ScaleSelectedNodes(fAmount, uAxis);
			break;
	}
}

void TSchemeEditor::TranslateSelectedNodes(float fAmount, TransformAxis uAxis)
{
	// Calculate the translation
	float3 vTranslation = float3(0,0,0);

	if(uAxis == AXIS_Y)
		vTranslation.y = fAmount;
	else if(uAxis == AXIS_X)
		vTranslation.x = fAmount;
	else
		vTranslation.z = fAmount;

	for(u32 uIndex = 0; uIndex < m_SelectedNodes.GetSize(); uIndex++)
	{
		TRef NodeRef = m_SelectedNodes.ElementAt(uIndex);

		TPtr<TLScene::TSceneNode> pNode = TLScene::g_pScenegraph->FindNode(NodeRef);

		if(pNode && pNode->HasTransform())
		{
			// Send the node a message to say we want to move the node
			TLMessaging::TMessage Message("ReqTransform", "Editor");
			Message.AddChildAndData("Translate", vTranslation);

			// NOTE: Should really send this message via the scenegraph rather than directly.
			pNode->QueueMessage(Message);
		}
	}
}

void TSchemeEditor::RotateSelectedNodes(float fAmount, TransformAxis uAxis)
{
	// Determine pivot point based on the rotation mode
	float3 fPivotPoint = float3(0,0,0);

	TLMaths::TQuaternion qRotation = TLMaths::TQuaternion(1,1,1,1);

	if(uAxis == AXIS_Y)
	{
	}
	else if(uAxis == AXIS_X)
	{
	}
	else
	{
	}

	for(u32 uIndex = 0; uIndex < m_SelectedNodes.GetSize(); uIndex++)
	{
		TRef NodeRef = m_SelectedNodes.ElementAt(uIndex);

		TPtr<TLScene::TSceneNode> pNode = TLScene::g_pScenegraph->FindNode(NodeRef);

		if(pNode && pNode->HasTransform())
		{
			// Send the node a message to say we want to move the node
			TLMessaging::TMessage Message("ReqTransform", "Editor");
			Message.AddChildAndData("Rotation", qRotation);

			// NOTE: Should really send this message via the scenegraph rather than directly.
			pNode->QueueMessage(Message);
		}
	}


}

void TSchemeEditor::ScaleSelectedNodes(float fAmount, TransformAxis uAxis)
{
	// Calculate the scale
	float3 vScale = float3(1,1,1);

	if(uAxis == AXIS_Y)
		vScale.y = fAmount;
	else if(uAxis == AXIS_X)
		vScale.x = fAmount;
	else
		vScale.z = fAmount;

	for(u32 uIndex = 0; uIndex < m_SelectedNodes.GetSize(); uIndex++)
	{
		TRef NodeRef = m_SelectedNodes.ElementAt(uIndex);

		TPtr<TLScene::TSceneNode> pNode = TLScene::g_pScenegraph->FindNode(NodeRef);

		if(pNode && pNode->HasTransform())
		{
			// Send the node a message to say we want to move the node
			TLMessaging::TMessage Message("ReqTransform", "Editor");
			Message.AddChildAndData("Scale", vScale);

			// NOTE: Should really send this message via the scenegraph rather than directly.
			pNode->QueueMessage(Message);
		}
	}
}


void TSchemeEditor::DeleteSelectedNodes()
{
	for(u32 uIndex = 0; uIndex < m_SelectedNodes.GetSize(); uIndex++)
	{
		TRef NodeRef = m_SelectedNodes.ElementAt(uIndex);

		// Tell the scenegraph to remove the node
		TLScene::g_pScenegraph->RemoveNode(NodeRef);
	}

	m_SelectedNodes.Empty();
}

