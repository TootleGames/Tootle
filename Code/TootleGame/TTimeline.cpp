#include "TTimeline.h"

// Include the graphs we will need
#include <TootleRender/TRendergraph.h>
#include <TootleScene/TScenegraph.h>
#include <TootleAudio/TAudiograph.h>
#include <TootlePhysics/TPhysicsgraph.h>

using namespace TLAnimation;

//#define TEST_REVERSE_PLAYBACK


void TTimelineInstance::Initialise(TLMessaging::TMessage& InitMessage)
{
	float fTime = 0.0f;

	// Get the initial time from the init message
	InitMessage.ImportData("Time", fTime);

	// Initialise the timeline to the current time
	SetTime(fTime);
}


void TTimelineInstance::SetTime(float fTime)
{
	// Check to ensure the time is within range of the assets key frames

	// Always check for less than 0.0f and set to 0 if less than
	if(fTime < 0.0f)
		fTime = 0.0f;
	else if(fTime > 0.0f)
	{
		TLAsset::TAssetTimeline* pAssetTimeline = GetAssetTimeline();

		// If no script then simply set the time to what was requested
		// Otherwise get the last key frame and check the time with that
		if(pAssetTimeline)
		{
			float fLastTime = pAssetTimeline->GetLastKeyFrameTime();

			// Set to the last key frame time if the keyframe time is less that the requested time
			if(fLastTime < fTime)
				fTime = fLastTime;
		}
	}

	// Set the current time of the timeline
	m_fTime = fTime; 
	
	// Update the timeline to be at the specified time
	OnTimeSet();

}

void TTimelineInstance::OnTimeSet()
{
	// Force the timeline to process the time
	DoUpdate(0.0f, TRUE);
}


SyncBool TTimelineInstance::DoUpdate(float fTimestep, Bool bForced)
{
	//TEST Force the timestep for testing purposes
	//fTimestep = 10.0f ;/// 10.0f;
	//TEST


	// Get the current and next keyframes
	TLAsset::TAssetTimeline* pAssetTimeline = GetAssetTimeline();

	// Wait for asset?
	if(!pAssetTimeline)
		return SyncWait;

	TArray<TLAsset::TTempKeyframeData> pKeyframes;

#ifdef TEST_REVERSE_PLAYBACK
	// Manipulate the timestep by the playback rate modifier
	fTimestep *= m_fPlaybackRateModifier;
#endif

	// We could cross multiple keyframes in one step so get all keyframes we are going to span
	// So we can send out all changes as required
	if(pAssetTimeline->GetKeyframes(m_fTime, fTimestep, pKeyframes, bForced))
	{
/*
#ifdef _DEBUG
	TTempString str;
	str.Appendf("Timeline instance processing %d Keyframes (Time %.3f %.3f)", pKeyframes.GetSize(), m_fTime, fTimestep);
	TLDebug_Print(str);
#endif
*/
		if(pKeyframes.GetSize() == 1)
		{
			// If we only have one keyframe then we have effectively come to the end of the timeline.  
			// Process the final keyframe as some commands may not have been processed yet and increase the time 
			// beyond the range so that we don't get any further keyframes from this point onwards (in normal proccessing)
			const TLAsset::TTempKeyframeData& Keyframe = pKeyframes.ElementAt(0);
			if(ProcessFinalKeyframe(Keyframe))
			{
				m_fTime += fTimestep;

				OnEndOfTimeline();

				// Finish
				return SyncFalse;
			}
		}
		else
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
						// Finish
						return SyncFalse;
					}
				}
			}
		}
	}
	else
	{
		// No keyframes so it's the end of the timeline
		OnEndOfTimeline();
		// Finish
		return SyncFalse;	
	}

	// Continue updating
	return SyncTrue;
}


void TTimelineInstance::OnEndOfTimeline()
{
	TLDebug_Print("Reached the end of the timeline");

	// Broadcast a message to say the timeline has finished
	TLMessaging::TMessage Message("OnComplete", m_AssetScriptRef );

	PublishMessage(Message);
}


TLAsset::TAssetTimeline* TTimelineInstance::GetAssetTimeline()
{
	// Get the asset script from the asset system
	TPtr<TLAsset::TAsset>& pAsset = TLAsset::LoadAsset(m_AssetScriptRef, TRUE,"Timeline");

	return pAsset.GetObject<TLAsset::TAssetTimeline>();
}


Bool TTimelineInstance::ProcessFinalKeyframe(const TLAsset::TTempKeyframeData& Keyframe)
{
	// Go through all of the command lists for the keyframe
	for(u32 uIndex = 0; uIndex < Keyframe.m_pKeyframe->GetSize(); uIndex++)
	{
		// Get the list of commands
		TPtr<TLAsset::TAssetTimelineCommandList> pCmdList = Keyframe.m_pKeyframe->ElementAt(uIndex);

		// Send out all commands as messages
		for(u32 uIndex2 = 0; uIndex2 < pCmdList->GetCommands().GetSize(); uIndex2++)
		{
			// Get the command 
			TLAsset::TAssetTimelineCommand& Cmd = pCmdList->GetCommands().ElementAt(uIndex2);

			if(!SendCommandAsMessage(Cmd, pCmdList->GetNodeGraphRef(), pCmdList->GetNodeRef()))
			{
				TLDebug_Print("Failed to send command");
				return FALSE;
			}
		}
	}

	return TRUE;
}

Bool TTimelineInstance::ProcessKeyframes(const TLAsset::TTempKeyframeData& KeyframeFrom, const TLAsset::TTempKeyframeData& KeyframeTo, float& fTimestep)
{/*
#ifdef _DEBUG
	TTempString str;
	str.Appendf("Keyframe %.3f -> %.3f (Time %.3f, Step %.3f)", KeyframeFrom.m_fTime, KeyframeTo.m_fTime, m_fTime, fTimestep);
	TLDebug_Print(str);
#endif
*/

	float fStartTime = m_fTime;
	float fEndTime = m_fTime + fTimestep;

	Bool bCrossingKeyframe = FALSE;
	
	if(fStartTime == fEndTime)
	{
		// Only difference here is that the endtime can be the same as the keyframe time in both cases.
		if((m_fTime <= KeyframeFrom.m_fTime) && (fEndTime >= KeyframeFrom.m_fTime))
			bCrossingKeyframe = TRUE;
	}
	else
	{
		// If current time is less than or equal to the from keyframe and the end time is more than the 
		// from keyframe time then we are crossing the keyframe
		if((m_fTime <= KeyframeFrom.m_fTime) && (fEndTime > KeyframeFrom.m_fTime))
			bCrossingKeyframe = TRUE;
	}


	for(u32 uIndex = 0; uIndex < KeyframeFrom.m_pKeyframe->GetSize(); uIndex++)
	{
		// Get the node list of commands
		TPtr<TLAsset::TAssetTimelineCommandList> pFromCmdList = KeyframeFrom.m_pKeyframe->ElementAt(uIndex);

		// Does shte same node ID exist in the to list?
		TPtr<TLAsset::TAssetTimelineCommandList> pToCmdList = KeyframeTo.m_pKeyframe->FindPtr(pFromCmdList->GetNodeRef());

		if(pToCmdList.IsValid())
		{
			// Go through the to cmds and see if any of the command id's match
			for(u32 uIndex2 = 0; uIndex2 < pFromCmdList->GetCommands().GetSize(); uIndex2++)
			{
				// Get the command from the 'from' command list
				TLAsset::TAssetTimelineCommand& FromCmd = pFromCmdList->GetCommands().ElementAt(uIndex2);

				// Does the same command occur in the 'to' keyframe node command list?
				TLAsset::TAssetTimelineCommand* pToCmd = pToCmdList->GetCommands().Find(FromCmd.GetMessageRef());

				if(pToCmd != NULL)
				{
					TRef MessageRef = FromCmd.GetMessageRef();
					// Command exists in both lists
					// CHeck to se eif this is a command that *might* be interped.
					// If so then go through the interp message sending rather than sending the message as-is
					// Otherwise simply send the message as is
					if(MessageRef == "SetTransform")
					{
						// Determine the percentage of the time we are betwen the keyframes
						float fPercent = (m_fTime - KeyframeFrom.m_fTime) / (KeyframeTo.m_fTime - KeyframeFrom.m_fTime);

						// Interp the key data
						if(!SendInterpedCommandAsMessage(FromCmd, *pToCmd, pFromCmdList->GetNodeGraphRef(), pFromCmdList->GetNodeRef(), MessageRef, fPercent))
						{
							TLDebug_Print("Failed to send command");
							return FALSE;
						}
					}
					else
					{
						// Crossing over keyframe?
						if(bCrossingKeyframe)
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
				else
				{
					// No equivalent 'to' command so simply send the message if we are going to cross the keyframe
					if(bCrossingKeyframe)
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
			if(bCrossingKeyframe)
			{
				// On the exact time of the keyframe so send out all commands as messages
				for(u32 uIndex2 = 0; uIndex2 < pFromCmdList->GetCommands().GetSize(); uIndex2++)
				{
					// Get the command from the 'from' command list
					TLAsset::TAssetTimelineCommand& FromCmd = pFromCmdList->GetCommands().ElementAt(uIndex2);

					if(!SendCommandAsMessage(FromCmd, pFromCmdList->GetNodeGraphRef(), pFromCmdList->GetNodeRef()))
					{
						TLDebug_Print("Failed to send command");
						return FALSE;
					}
				}
			}
		}
	}

	// Now update the timestep and instance time after processing a keyframe
	float fKeyframeTimeDelta = KeyframeTo.m_fTime - m_fTime;

	// If the change in time between the key and the current time is greater than or equal 
	// to the time step then this should be the last change so set the timestep to 0
	// Otherwise reduce the timestep by the change in time between the key and current time
	if(fKeyframeTimeDelta >= fTimestep)
	{
		m_fTime += fTimestep;
		fTimestep = 0.0f;

#ifdef _DEBUG
		if(m_fTime != fEndTime)
		{
			TLDebug_Break("Time not set correctly");
		}
#endif
	}
	else
	{
		fTimestep -= fKeyframeTimeDelta;
		m_fTime += fKeyframeTimeDelta;
	}

	// All done
	return TRUE;
}


Bool TTimelineInstance::SendCommandAsMessage(TLAsset::TAssetTimelineCommand& Command, TRef NodeGraphRef, TRef NodeRef)
{
	TLDebug_Print("Sending as-is command");

	// Check for node mapped ref.  If found replace the ndoe ref with the one for our key array
	if(m_NodeRefMap.GetSize() > 0)
	{
		TRef* pAlternateRef = m_NodeRefMap.Find(NodeRef);

		if(pAlternateRef != NULL)
		{
			NodeRef = *pAlternateRef;
		}
	}

#ifdef _DEBUG
	// Check for special 'this' node ref.  Should have been altered via the noderefmap lookup but
	// if the timeline instance hasn;t had the BindTo routine called then "this" won't have been replaced.
	// We can't really call anything on the graphs using a ref of "this" as it won't do anything so at least 
	// trap this in debug.
	if(NodeRef == TRef("this"))
	{
		TLDebug_Print("Invalid node ref for 'this' on asset script instance");
		return FALSE;
	}
#endif

	TRef MessageRef = Command.GetMessageRef();

	// Handle special create and shutdown commands
	if(MessageRef == TLCore::InitialiseRef)
	{
		// get the paretn ndoe ref
		TRef ParentNodeRef;

		Command.ImportData("ParentRef", ParentNodeRef);

		// Create a new node via the graph and 
		TRef NewNodeRef;

		// Create a node via the graph
		if(NodeGraphRef == "Scene")
			NewNodeRef = TLScene::g_pScenegraph->CreateNode(NodeRef, "Object", ParentNodeRef, &Command);
		else if(NodeGraphRef == "Render")
			NewNodeRef = TLRender::g_pRendergraph->CreateNode(NodeRef, "Render", ParentNodeRef, &Command);
		else if(NodeGraphRef == "Audio")
			NewNodeRef = TLAudio::g_pAudiograph->CreateNode(NodeRef, "Audio", ParentNodeRef, &Command);
		else if(NodeGraphRef == "Physics")
			NewNodeRef = TLPhysics::g_pPhysicsgraph->CreateNode(NodeRef, "Physics", ParentNodeRef, &Command);
		else
		{
			// Invalid graph ref? We may need to handle special cases here.
			return FALSE;
		}


		if(NewNodeRef.IsValid() && (NodeRef != NewNodeRef))
		{
			// Store teh reference in the key array.  This will be used as the alternative node name for 
			// when the same node ID is used in other commands
			m_NodeRefMap.Add(NodeRef, NewNodeRef);
		}
		return (NewNodeRef.IsValid());
	}
	else if (MessageRef == TLCore::ShutdownRef)
	{
		// Remove from our keyarray if it exists
		m_NodeRefMap.RemoveItem(NodeRef);

		// Shutdown a node from the graph
		if(NodeGraphRef == "Scene")
			return TLScene::g_pScenegraph->RemoveNode(NodeRef);
		else if(NodeGraphRef == "Render")
			return TLRender::g_pRendergraph->RemoveNode(NodeRef);
		else if(NodeGraphRef == "Audio")
			return TLAudio::g_pAudiograph->RemoveNode(NodeRef);
		else if(NodeGraphRef == "Physics")
			return TLPhysics::g_pPhysicsgraph->RemoveNode(NodeRef);
		else
		{
			// Invalid graph ref? We may need to handle special cases here.
			return FALSE;
		}

	}
	else if(MessageRef == "TimeJump")
	{
		float fTime;
		
		if(Command.ImportData("Time", fTime))
		{
			// Jump to time immediately?  Or add a command object to this instance 
			// and jump at a 'safe' time?
			// Either way, stop the processing of the timeline instance any further
			m_fTime = fTime;
			return FALSE;
		}

		// Invalid time?
		// Ignore and continue processing
		return TRUE;
	}
	else
	{

		// Send message to the graph
		if(NodeGraphRef == "Scene")
			return TLScene::g_pScenegraph->SendMessageToNode(NodeRef, Command);
		else if(NodeGraphRef == "Render")
			return TLRender::g_pRendergraph->SendMessageToNode(NodeRef, Command);
		else if(NodeGraphRef == "Audio")
			return TLAudio::g_pAudiograph->SendMessageToNode(NodeRef, Command);
		else if(NodeGraphRef == "Physics")
			return TLPhysics::g_pPhysicsgraph->SendMessageToNode(NodeRef, Command);
		else
		{
			// Invalid graph ref
			// Send the command to subscribers
			PublishMessage(Command);

			return TRUE;
		}

	}
}



Bool TTimelineInstance::SendInterpedCommandAsMessage(TLAsset::TAssetTimelineCommand& FromCommand, TLAsset::TAssetTimelineCommand& ToCommand, TRef NodeGraphRef, TRef NodeRef, TRefRef MessageRef, float fPercent)
{
	TLDebug_Print("Sending interped command");

	// Check for node mapped ref.  If found replace the ndoe ref with the one for our key array
	if(m_NodeRefMap.GetSize() > 0)
	{
		TRef* pAlternateRef = m_NodeRefMap.Find(NodeRef);

		if(pAlternateRef != NULL)
		{
			NodeRef = *pAlternateRef;
		}
	}

#ifdef _DEBUG
	// Check for special 'this' node ref.  Should have been altered via the noderefmap lookup but
	// if the timeline instance hasn;t had the BindTo routine called then "this" won't have been replaced.
	// We can't really call anything on the graphs using a ref of "this" as it won't do anything so at least 
	// trap this in debug.
	if(NodeRef == TRef("this"))
	{
		TLDebug_Print("Invalid node ref for 'this' on timline instance");
		return FALSE;
	}
#endif

	// Create a new message with the same ref as the from command
	TLMessaging::TMessage Message(MessageRef);

	// Reset the readpos for the commands
	FromCommand.ResetReadPos();
	ToCommand.ResetReadPos();

	// Now get the data from the 'from' command and the 'to' command and use the appropraite 
	// interp routine to generate some new data
	if(MessageRef == "SetTransform")
	{
		for(u32 uIndex = 0; uIndex < FromCommand.GetChildren().GetSize(); uIndex++)
		{
			TPtr<TBinaryTree>& pFromData = FromCommand.GetChildren().ElementAt(uIndex);

			TRef InterpMethod = "None";

			// Now check to see if the Rotate command needs interping
			if(pFromData->ImportData("InterpMethod", InterpMethod) && (InterpMethod != "None"))
			{
				// Check to see if the ToCommand has the same child data
				TPtr<TBinaryTree>& pToData = ToCommand.GetChild(pFromData->GetDataRef()); 
				
				// If so interp the data otherwise jsut add the data as-is
				if(pToData)
					AttachInterpedDataToMessage(pFromData, pToData, InterpMethod, fPercent, Message);
				else
					Message.AddChild(pFromData);
			}
			else
			{
				// Attach Rotation as-is to the message
				Message.AddChild(pFromData);
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


void TTimelineInstance::AttachInterpedDataToMessage(TPtr<TBinaryTree>& pFromData, TPtr<TBinaryTree>& pToData, TRefRef InterpMethod, const float& fPercent, TLMessaging::TMessage& Message)
{
	pFromData->ResetReadPos();
	pToData->ResetReadPos();

	// Check the data type and use the appropriate interp as required.
	//if(pFromData->GetDataTypeHint() == TLBinary::GetDataTypeRef<TLMaths::TQuaternion>())
	if(pFromData->GetDataRef() == "Rotate")
	{
		// Get the to command rotate quaternion
		TLMaths::TQuaternion qRotFrom, qRotTo;

		if(pFromData->Read(qRotFrom))
		{
			if(pToData->Read(qRotTo))
			{
				TLMaths::TQuaternion qRot;
#ifdef _DEBUG
				TTempString str;

				InterpMethod.GetString(str);
				TLDebug_Print(str);
				str.Empty();

				str.Appendf("Quat From: %.2f %.2f %.2f %.2f", qRotFrom.GetData().x, qRotFrom.GetData().y, qRotFrom.GetData().z, qRotFrom.GetData().w );
				TLDebug_Print(str);
				str.Empty();
				str.Appendf("Quat To: %.2f %.2f %.2f %.2f", qRotTo.GetData().x, qRotTo.GetData().y, qRotTo.GetData().z, qRotTo.GetData().w );
				TLDebug_Print(str);
				str.Empty();
				str.Appendf("Time: %.2f", fPercent );
				TLDebug_Print(str);
				str.Empty();
#endif

				// Do spherical linear interp if specified, otherwise default to linear interp
				if(InterpMethod == STRef(S,p,h,e,r))
				{
					qRot.Slerp(qRotFrom, qRotTo, fPercent);
					qRot.Normalise();
				}
				else
					qRot.Nlerp(qRotFrom, qRotTo, fPercent);
		
#ifdef _DEBUG
				str.Appendf("Quat Lerp: %.2f %.2f %.2f %.2f", qRot.GetData().x, qRot.GetData().y, qRot.GetData().z, qRot.GetData().w );
				TLDebug_Print(str);
#endif
				Message.ExportData(pFromData->GetDataRef(), qRot);
			}
		}
	}
	//else if(pFromData->GetDataTypeHint() == TLBinary::GetDataTypeRef<float3>())
	else
	{
		// Get the to command translation
		float3 vFrom, vTo;

		if(pFromData->Read(vFrom))
		{
			if(pToData->Read(vTo))
			{
				float3 vector;
				
				//TODO: Check for different type of interps
				vector = TLMaths::Interp(vFrom, vTo, fPercent);

				Message.ExportData(pFromData->GetDataRef(), vector);
			}
		}
	}
}



