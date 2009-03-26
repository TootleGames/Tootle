
#include "TAssetScriptManager.h"


// Include the grahs we will need
#include <TootleRender/TRendergraph.h>
#include <TootleScene/TScenegraph.h>



namespace TLAnimation
{
	TPtr<TAssetScriptManager> g_pAssetScriptManager = NULL;
}

using namespace TLAnimation;


void TAssetScriptInstance::Update(float fTimestep)
{

	// Get the current and next keyframes
	TLAsset::TAssetScript* pScript = GetAssetScript();

	if(!pScript)
		return;

	TArray<TLAsset::TTempKeyframeData> pKeyframes;

	// We could cross multiple keyframes in one step so get all keyframes we are going to span
	// So we can send out all changes as required
	if(pScript->GetKeyframes(m_fTime, fTimestep, pKeyframes))
	{
		// Process the keyframes. Keyframes will be in order
		for(u32 uIndex = 0; uIndex < pKeyframes.GetSize(); uIndex++)
		{
			// Get the 'from' keyframe
			const TLAsset::TTempKeyframeData& KeyframeFrom = pKeyframes.ElementAt(uIndex);

			if(uIndex+1 < pKeyframes.GetSize())
			{
				// Get the 'to' keyframe
				const TLAsset::TTempKeyframeData& KeyframeTo = pKeyframes.ElementAt(uIndex+1);

				if(!ProcessKeyframes(KeyframeFrom, KeyframeTo, fTimestep))
				{
					TLDebug_Print("Failed to process keyframes");
					return;
				}
			}
			else
			{
				// On last keyframe or only have one in the array
			}
		}
	}
}


TLAsset::TAssetScript* TAssetScriptInstance::GetAssetScript()
{
	// Get the asset script from the asset system
	TPtr<TLAsset::TAssetScript> AssetScript = TLAsset::GetAsset(m_AssetScriptRef, TRUE);

	if(AssetScript.IsValid())
		return AssetScript.GetObject();

	return NULL;
}


Bool TAssetScriptInstance::ProcessKeyframes(const TLAsset::TTempKeyframeData& KeyframeFrom, const TLAsset::TTempKeyframeData& KeyframeTo, float& fTimestep)
{

	// Just use the first keyframe for now  - no interpolation

	// Get the keyframe from node id
	// Search for the same node id on the keyframe to
	// Only if both exist can values be interped
	// otherwise simply set from the first keyframe

	for(u32 uIndex = 0; uIndex < KeyframeFrom.m_pKeyframe->GetSize(); uIndex++)
	{
		// Get the node list of commands
		TPtr<TLAsset::TAssetScriptCommandList> pFromCmdList = KeyframeFrom.m_pKeyframe->ElementAt(uIndex);

		// Does shte same node ID exist in the to list?
		TPtr<TLAsset::TAssetScriptCommandList> pToCmdList = KeyframeTo.m_pKeyframe->FindPtr(pFromCmdList->GetNodeRef());

		if(pToCmdList.IsValid())
		{
			// Go through the to cmds and see if any of the command id's match
			for(u32 uIndex2 = 0; uIndex2 < pFromCmdList->GetCommands().GetSize(); uIndex2++)
			{
				// Get the command from the 'from' command list
				TLAsset::TAssetScriptCommand& FromCmd = pFromCmdList->GetCommands().ElementAt(uIndex2);

				// Does the same command occur in the 'to' keyframe node command list?
				TLAsset::TAssetScriptCommand* pToCmd = pToCmdList->GetCommands().Find(FromCmd.GetMessageRef());

				if(pToCmd != NULL)
				{
					// Command exists in both lists
					// Check to see if any interping has been selected.
					// If so interp the key data and send a generated message
					// Otherwise simply send the message as is if the keyframe time matches
					if(FromCmd.GetInterpMethod() == TLAsset::TAssetScriptCommand::None)
					{
						// Crossing over keyframe?
						if(m_fTime - KeyframeFrom.m_fTime < fTimestep)
						{
							// On the exact time of the keyframe so send out the command as a message
							if(!SendCommandAsMessage(FromCmd, pFromCmdList->GetNodeGraphRef(), pFromCmdList->GetNodeRef()))
							{
								TLDebug_Print("Failed to send command");
								return FALSE;
							}
						}
					}
					else
					{
						// Determine the percentage of the time we aer betwen the keyframes
						float fPercent = (m_fTime - KeyframeFrom.m_fTime) / (KeyframeTo.m_fTime - KeyframeFrom.m_fTime);

						// Interp the key data
						if(!SendInterpedCommandAsMessage(FromCmd, *pToCmd, pFromCmdList->GetNodeGraphRef(), pFromCmdList->GetNodeRef(), fPercent))
						{
							TLDebug_Print("Failed to send command");
							return FALSE;
						}

					}
				}
				else
				{
					// No equivalent 'to' command so simply send the message if we are going to cross the keyframe
					if(m_fTime - KeyframeFrom.m_fTime < fTimestep)
					{
						// On the exact time of the keyframe so send out the command as a message
						if(!SendCommandAsMessage(FromCmd, pFromCmdList->GetNodeGraphRef(), pFromCmdList->GetNodeRef()))
						{
							TLDebug_Print("Failed to send command");
							return FALSE;
						}
					}

				}
			}
		}
		else
		{
			// Node doesn't have an equivalent command list for the node specified in the 'to' keyframe
			// Simply  go through the commands and send them as messages if we are going to cross the keyframe
			if(m_fTime - KeyframeFrom.m_fTime < fTimestep)
			{
				// On the exact time of the keyframe so send out all commands as messages
				for(u32 uIndex2 = 0; uIndex2 < pFromCmdList->GetCommands().GetSize(); uIndex2++)
				{
					// Get the command from the 'from' command list
					TLAsset::TAssetScriptCommand& FromCmd = pFromCmdList->GetCommands().ElementAt(uIndex2);

					if(!SendCommandAsMessage(FromCmd, pFromCmdList->GetNodeGraphRef(), pFromCmdList->GetNodeRef()))
					{
						TLDebug_Print("Failed to send command");
						return FALSE;
					}
				}
			}
		}
	}

	// Now update the timestep and instance time
	float fKeyframeTimeDelta = KeyframeTo.m_fTime - KeyframeFrom.m_fTime;

	// If the change in time between the keys is greater than or equal to the time step then this should 
	// be the last change so set the timestep to 0
	// Otherwise reduce the timestep by the change in time between the keys
	if(fKeyframeTimeDelta >= fTimestep)
	{
		m_fTime += fTimestep;
		fTimestep = 0.0f;
	}
	else
	{
		fTimestep -= fKeyframeTimeDelta;
		m_fTime += fKeyframeTimeDelta;
	}

	// All done
	return TRUE;
}


Bool TAssetScriptInstance::SendCommandAsMessage(TLAsset::TAssetScriptCommand& Command, TRef NodeGraphRef, TRef NodeRef)
{
	// Check for special 'this' node ref
	if(NodeRef == "this")
	{
		// Can't send the message if we don't have a valid node ref
		if(!m_NodeRef.IsValid())
		{
			TLDebug_Print("Invalid node ref for 'this' on asset script instance");
			return FALSE;
		}

		// Replace the node ref with our instance ndoe ref

		NodeRef = m_NodeRef;

		// The graph ref override is optional
		if(m_NodeGraphRef.IsValid())
			NodeGraphRef = m_NodeGraphRef;
	}


	if(NodeGraphRef == "Render")
	{
		// Send message to the render graph
		return TLRender::g_pRendergraph->SendMessageToNode(NodeRef, Command);
	}
	else if(NodeGraphRef == "Scene")
	{
		// Send message to the scenegraph
		return TLScene::g_pScenegraph->SendMessageToNode(NodeRef, Command);
	}
	else
	{
		// Invalid graph ref? We may need to handle special cases here.

		return FALSE;
	}
}



Bool TAssetScriptInstance::SendInterpedCommandAsMessage(TLAsset::TAssetScriptCommand& FromCommand, TLAsset::TAssetScriptCommand& ToCommand, TRef NodeGraphRef, TRef NodeRef, float fPercent)
{
	// Check for special 'this' node ref
	if(NodeRef == "this")
	{
		// Can't send the message if we don't have a valid node ref
		if(!m_NodeRef.IsValid())
		{
			TLDebug_Print("Invalid node ref for 'this' on asset script instance");
			return FALSE;
		}

		// Replace the node ref with our instance ndoe ref

		NodeRef = m_NodeRef;

		// The graph ref override is optional
		if(m_NodeGraphRef.IsValid())
			NodeGraphRef = m_NodeGraphRef;
	}




	TRef MessageRef = FromCommand.GetMessageRef();

	// Create a new message with the same ref as the from command
	TLMessaging::TMessage Message(MessageRef);

	// Reset the readpos for the commands
	FromCommand.ResetReadPos();
	ToCommand.ResetReadPos();

	// Now get the data from the from command and to command and use the appropraite interp routine to 
	// generate some new data
	if(MessageRef == "Rotate")
	{
		// Get the to command rotate quaternion
		TLMaths::TQuaternion qRotFrom, qRotTo;

		if(FromCommand.Read(qRotFrom))
		{
			if(ToCommand.Read(qRotTo))
			{
				//TODO: Check for different type of interps
				TLMaths::TQuaternion qRot;

				qRot = TLMaths::Interp(qRotFrom, qRotTo, fPercent);

				Message.Write(qRot);
			}
		}
	} 
	else if((MessageRef == "Translate") ||
			(MessageRef == "Scale"))
	{
		// Get the to command translation
		float3 vFrom, vTo;

		if(FromCommand.Read(vFrom))
		{
			if(ToCommand.Read(vTo))
			{
				//TODO: Check for different type of interps
				float3 vector;

				vector = TLMaths::Interp(vFrom, vTo, fPercent);

				Message.Write(vector);
			}
		}
	}
	else
	{
		TLDebug_Break("Unhandled interped command");
	}

	//TODO: Check for the node id being special i.e. "This", == node graph, null...?
	if(NodeGraphRef == "Render")
	{
		// Send message to the render graph
		return TLRender::g_pRendergraph->SendMessageToNode(NodeRef, Message);
	}
	else if(NodeGraphRef == "Scene")
	{
		// Send message to the scenegraph
		return TLScene::g_pScenegraph->SendMessageToNode(NodeRef, Message);
	}
	else
	{
		// Invalid graph ref? We may need to handle special cases here.

		return FALSE;
	}
}








TPtr<TLAnimation::TAssetScriptInstance> TAssetScriptManager::CreateTimeline(TRefRef AssetScriptRef)
{
	TPtr<TLAnimation::TAssetScriptInstance> pInstance = new TLAnimation::TAssetScriptInstance(AssetScriptRef);

	if(!pInstance)
		return TPtr<TLAnimation::TAssetScriptInstance>(NULL);

	if(m_TimelineInstances.Add(pInstance) == -1)
		return TPtr<TLAnimation::TAssetScriptInstance>(NULL);

	// Success
	return pInstance;
}

void TAssetScriptManager::DeleteTimeline(TPtr<TLAnimation::TAssetScriptInstance> pInstance)
{
	//m_TimelineInstances.Remove(pInstance);
}

SyncBool TAssetScriptManager::Shutdown()
{
	// Remove all instances of the timelines
	m_TimelineInstances.Empty(TRUE);

	return TManager::Shutdown();
}

SyncBool TAssetScriptManager::Update(float fTimestep)
{
	return TManager::Update(fTimestep);
}

void TAssetScriptManager::OnEventChannelAdded(TRefRef refPublisherID, TRefRef refChannelID)
{
	TManager::OnEventChannelAdded(refPublisherID, refChannelID);
}


