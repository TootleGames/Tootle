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
		//	gr: must faster than using and de-referencing a TPtr all the time
		TPtr<TLAsset::TAssetTimelineCommandList>& pCmdList = Keyframe.m_pKeyframe->ElementAt(uIndex);
		TLAsset::TAssetTimelineCommandList& CmdList = *pCmdList;
		TRefRef GraphRef = CmdList.GetNodeGraphRef();
		TRefRef NodeRef = CmdList.GetNodeRef();

		// Send out all commands as messages
		for(u32 uIndex2 = 0; uIndex2 < CmdList.GetCommands().GetSize(); uIndex2++)
		{
			// Get the command 
			TLAsset::TAssetTimelineCommand& Cmd = CmdList.GetCommands().ElementAt(uIndex2);

			if(!SendCommandAsMessage(&Cmd, NULL, GraphRef, NodeRef ))
			{
				#ifdef _DEBUG
				TTempString Debug_String("Failed to send timeline command ");
				Cmd.GetMessageRef().GetString( Debug_String );
				Debug_String.Append(" to ");
				NodeRef.GetString( Debug_String );
				Debug_String.Append(" in ");
				GraphRef.GetString( Debug_String );
				TLDebug_Print( Debug_String );
				#endif
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
					// Command exists in both lists

					// Determine the percentage of the time we are betwen the keyframes
					float fPercent = (m_fTime - KeyframeFrom.m_fTime) / (KeyframeTo.m_fTime - KeyframeFrom.m_fTime);

					// Interp the key data
					if(!SendCommandAsMessage(&FromCmd, pToCmd, pFromCmdList->GetNodeGraphRef(), pFromCmdList->GetNodeRef(), fPercent, TRUE))
					{
						TLDebug_Print("Failed to send command");
						return FALSE;
					}
				}
				else
				{
					// No equivalent 'to' command so simply send the message if we are going to cross the keyframe
					if(bCrossingKeyframe)
					{
						// On the exact time of the keyframe so send out the command as a message
						if(!SendCommandAsMessage(&FromCmd, NULL, pFromCmdList->GetNodeGraphRef(), pFromCmdList->GetNodeRef()))
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

					if(!SendCommandAsMessage(&FromCmd, NULL, pFromCmdList->GetNodeGraphRef(), pFromCmdList->GetNodeRef()))
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


Bool TTimelineInstance::SendCommandAsMessage(TLAsset::TAssetTimelineCommand* pFromCommand, TLAsset::TAssetTimelineCommand* pToCommand, TRef NodeGraphRef, TRef NodeRef, float fPercent, Bool bTestDataForInterp)
{
	TLDebug_Print("Sending timeline command");

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
	if(NodeRef == TRef_Static4(t,h,i,s))
	{
		TLDebug_Print("Invalid node ref for 'this' on timeline instance");
		return FALSE;
	}
#endif

	TLMessaging::TMessage* pMessage = pFromCommand;

	TRef MessageRef = pFromCommand->GetMessageRef();

	// Handle special create and shutdown commands
	if(MessageRef == TLCore::InitialiseRef)
	{
		TLDebug_Print("Timeline - Create node");

		//	gr: i've removed the refs in the animation namespace - the refs are set in the graph's own
		//		constructor now so they are not changed. If we want some global TRef_Static's for this
		//		then we should define them somewhere more gloablly (not in the animation system)
		TArray<TRef>* pNodeArray = GetGraphNodeRefArray( NodeGraphRef );
		if ( !pNodeArray )
		{
			TLDebug_Break("Node array expected - invalid graph ref?");
			return FALSE;
		}

		if(pNodeArray->Exists(NodeRef))
		{
			// Node has already been created.  Should we allow the timeline
			// to create a new node in this instance? Or should we enforce on ly creation
			// and shutdown for hte same node ref?  The former would allow for 
			// emission type timelines and/or spawning - create a new monster every n seconds
			// and the former would prevent issues with duplicating nodes when we don't want 
			// them to be.  Perhaps flagging the init as 'unique' would help distinguish when 
			// you want the former only?

			// For now prevent duplication.
			TLDebug_Print("Timeline - Node exists, preventing duplication");
			return TRUE;
		}

		// get the paretn ndoe ref
		TRef ParentNodeRef;

		pFromCommand->ImportData("ParentRef", ParentNodeRef);

		// Create a new node via the graph 
		TLGraph::TGraphBase* pGraph = GetGraph( NodeGraphRef );
		if ( !pGraph )
		{
			TLDebug_Break("Graph expected");
			return FALSE;
		}

		// Create a node via the graph
		//	gr: note here, the scene graph had an exception where it created an "Object" type, and not the defualt type...
		//		why is this? the type will HAVE to be able to be specified in the XML anyway for flexibility (eg. creating particle render nodes)...
		//		if you consider the base scene node type useless, we shouldn't have one and the scene node should be a type
		//		which always has render/physics/transform capabilities...
		TRef NewNodeRef;
		TRef NewNodeTypeRef = (NodeGraphRef==TRef_Static(S,c,e,n,e)) ? TRef("Object") : TRef();

		NewNodeRef = pGraph->CreateNode(NodeRef, NewNodeTypeRef, ParentNodeRef, pMessage);
		if ( !NewNodeRef.IsValid() )
			return FALSE;


		if(NodeRef != NewNodeRef)
		{
			// Store the reference in the key array.  This will be used as the alternative node name for 
			// when the same node ID is used in other commands
			m_NodeRefMap.Add(NodeRef, NewNodeRef);
		}

		// Store the created node ref to prevent duplication
		pNodeArray->Add(NewNodeRef);

		// Once we have created a node for a graph we need to process messages
		// for if the node is removed prematurely so we can update our list of 
		// created nodes correctly
		SubscribeTo( pGraph );

		// Success
		return TRUE;
	}
	else if (MessageRef == TLCore::ShutdownRef)
	{
		TLDebug_Print("Timeline - Shutdown node");

		Bool bResult = FALSE;
		TRefRef RemovedNodeRef = NodeRef;

		// Create a new node via the graph 
		TLGraph::TGraphBase* pGraph = GetGraph( NodeGraphRef );
		if ( !pGraph )
		{
			TLDebug_Break("Graph expected");
			return FALSE;
		}

		// Shutdown a node from the graph
		bResult = pGraph->RemoveNode(NodeRef);

		// 
		//if(bResult)
		{
			// Remove from our keyarray if it exists
			m_NodeRefMap.RemoveItem(RemovedNodeRef);

			// Remove from the 'created nodes' list
			TArray<TRef>* pNodeArray = GetGraphNodeRefArray( NodeGraphRef );
			if ( !pNodeArray )
			{
				TLDebug_Break("Node array expected - invalid graph ref?");
				return FALSE;
			}

			pNodeArray->Remove(RemovedNodeRef);
		}

		//if(!bResult)
		//	TLDebug_Break("Failed to shutdown node");

		// Allow timeline to continue even if failed to remove node.
		return TRUE;
	}
	else if(MessageRef == TLAnimation::TimeJumpRef)
	{
		TLDebug_Print("Timeline - Timejump encountered");
		float fTime;
		
		if(pFromCommand->ImportData("Time", fTime))
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
		// Create a new message with the same ref as the from command
		// NOTE: This may not get used but is needed as a local in case we use it
		TLMessaging::TMessage NewMessage(MessageRef);

		// Do test for interped data?
		if(bTestDataForInterp)
		{
			TLDebug_Print("Testing for interped data");

			pMessage = &NewMessage;

			// Reset the readpos for the commands
			pFromCommand->ResetReadPos();
			pToCommand->ResetReadPos();

			// Now get the data from the 'from' command and the 'to' command and use the appropriate 
			// interp routine to generate some new data
			u32 uNumberOfDataElements = pFromCommand->GetChildren().GetSize();
			for(u32 uIndex = 0; uIndex < uNumberOfDataElements; uIndex++)
			{
				TPtr<TBinaryTree>& pFromData = pFromCommand->GetChildren().ElementAt(uIndex);

				TRef InterpMethod = "None";

				// Now check to see if the command data needs interping
				if(pFromData->ImportData("InterpMethod", InterpMethod) && (InterpMethod != "None"))
				{
					// Check to see if the ToCommand has the same child data
					TPtr<TBinaryTree>& pToData = pToCommand->GetChild(pFromData->GetDataRef()); 
					
					// If so interp the data otherwise just add the data as-is
					if(pToData)
						AttachInterpedDataToMessage(pFromData, pToData, InterpMethod, fPercent, NewMessage);
					else
						NewMessage.AddChild(pFromData);
				}
				else
				{
					// Attach data as-is to the message
					NewMessage.AddChild(pFromData);
				}
			}
		}

		// Send message to the graph
		if(NodeGraphRef.IsValid())
		{
			return TLCore::g_pCoreManager->SendMessageTo(NodeRef, NodeGraphRef, *pMessage);
		}
		else
		{
			// Invalid graph ref
			// Send the command to subscribers
			PublishMessage(*pMessage);

			return TRUE;
		}
	}
}


void TTimelineInstance::AttachInterpedDataToMessage(TPtr<TBinaryTree>& pFromData, TPtr<TBinaryTree>& pToData, TRefRef InterpMethod, const float& fPercent, TLMessaging::TMessage& Message)
{
	pFromData->ResetReadPos();
	pToData->ResetReadPos();

	TRef DataType = pFromData->GetDataTypeHint();

	// Check the data type and use the appropriate interp as required.
	if(DataType == TLBinary::GetDataTypeRef<TLMaths::TQuaternion>())
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
	else if(DataType == TLBinary::GetDataTypeRef<TColour>())
	{
		TColour vFrom, vTo;

		if(pFromData->Read(vFrom))
		{
			if(pToData->Read(vTo))
			{
				TColour vector;
				
				//TODO: Check for different type of interps
				vector = TLMaths::Interp(vFrom, vTo, fPercent);

				Message.ExportData(pFromData->GetDataRef(), vector);
			}
		}

	}
	else if(DataType == TLBinary::GetDataTypeRef<float3>())
	{
		// Get the to command vector
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
#ifdef _DEBUG
	else
	{
		// Unhandled type
		TTempString str;
		DataType.GetString(str);
		TTempString errorstr;
		errorstr.Appendf("Unhandled timeline interpolation data type - %s", str.GetData());
		TLDebug_Break(errorstr);
	}
#endif
}


void TTimelineInstance::ProcessMessage(TLMessaging::TMessage& Message)
{
	if(Message.GetMessageRef() == STRef(N,o,d,e,R))	
	{
		// NodeRemoved
		// Check to see if this node was created by the timeline
		TRef ManagerRef = Message.GetSenderRef();

		TRef NodeRef;
		Message.Read(NodeRef);

		if(NodeRef.IsValid())
		{
			TArray<TRef>* pNodeArray = GetGraphNodeRefArray(ManagerRef);
			if ( pNodeArray )
			{
				// Simply try and remove the ref - not worth testing for if it exists.
				pNodeArray->Remove(NodeRef);
			}
		}
	}
}

