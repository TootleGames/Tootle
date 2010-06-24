/*
 *  TAssetTimline.cpp
 *  TootleAsset
 *
 *  Created by Duane Bradbury on 15/03/2009.
 *  Copyright 2009 Tootle. All rights reserved.
 *
 */

#include "TTimeline.h"


using namespace TLAsset;


// Quick define to be able to trace out the timeline data tree
#ifdef _DEBUG
//	#define DEBUG_PRINT_TIMELINE_TREE
#endif

//#define USE_KEYFRAME_TOLERANCE_TESTS
#define TIMELINE_KEYFRAME_TOLERANCE 0.00001f


SyncBool TKeyframe::ImportData(TBinaryTree& Data)
{
	u32 uCommandListCount = Data.GetChildren().GetSize();

	if(uCommandListCount > 0)
	{
		SetAllocSize(uCommandListCount);

		// Now read each command
		for(u32 uIndex = 0; uIndex < uCommandListCount; uIndex++)
		{
			TPtr<TBinaryTree> pNodeTree = Data.GetChildren().ElementAt(uIndex);
			pNodeTree->ResetReadPos();

			// add a new command list to the keyframe
			TPtr<TLAsset::TTimelineCommandList> pCommandList = new TLAsset::TTimelineCommandList();

			if(!pCommandList || (Add(pCommandList) == -1))
			{
				TLDebug_Print("Failed to create new command list for TTL asset file");
				return SyncFalse;
			}

			SyncBool CmdRes = pCommandList->ImportData(*pNodeTree);

			if(CmdRes != SyncTrue)
				return CmdRes;
		}
	}

	return SyncTrue;
}

SyncBool TKeyframe::ExportData(TBinaryTree& Data)
{
	for(u32 uIndex = 0; uIndex < GetSize(); uIndex++)
	{
		TPtr<TBinaryTree> pNodeTree = Data.AddChild("Node");
		
		if(pNodeTree)
		{
			TPtr<TTimelineCommandList>& cmdslist = ElementAt(uIndex);

			SyncBool CmdRes = cmdslist->ExportData(*pNodeTree);

			if(CmdRes != SyncTrue)
				return CmdRes;
		}
	}

	return SyncTrue;
}




SyncBool TTimelineCommandList::ImportData(TBinaryTree& Data)
{
	// Read the Node ref
	if(!Data.Read(m_NodeRef))
	{
		// Failed to read data
		TLDebug_Print("Failed to read node ref from TTL asset file");
		return SyncFalse;
	}

	// Read the graph ref
	if(!Data.Read(m_NodeGraphRef))
	{
		// Failed to read data
		TLDebug_Print("Failed to read node graph ref from TTL asset file");
		return SyncFalse;
	}

	// Get the commands child
	TPtr<TBinaryTree> pCommandData = Data.GetChild("Commands");
	pCommandData->ResetReadPos();

	if(pCommandData)
	{
		// Now read each command
		for(u32 uIndex = 0; uIndex < pCommandData->GetChildren().GetSize(); uIndex++)
		{
			TPtr<TBinaryTree> pCommand = pCommandData->GetChildren().ElementAt(uIndex);
			pCommand->ResetReadPos();

			TLAsset::TTimelineCommand cmd;

			cmd.CopyDataTree(pCommand);
			//pCommand->Read(cmd);

			// Add the command to the array
			m_Commands.Add(cmd);
		}
	}


	return SyncTrue;
}

SyncBool TTimelineCommandList::ExportData(TBinaryTree& Data)
{
	Data.Write(m_NodeRef);
	Data.Write(m_NodeGraphRef);

	TPtr<TBinaryTree> pCommandData = Data.AddChild("Commands");

	if(pCommandData)
	{
		for(u32 uIndex = 0; uIndex < m_Commands.GetSize(); uIndex++)
		{
			TTimelineCommand& cmd = m_Commands.ElementAt(uIndex);

			TPtr<TBinaryTree> pCommand = pCommandData->AddChild("Command");

			pCommand->CopyDataTree(cmd);
			//pCommand->Write(cmd);
		}
	}

	return SyncTrue;
}


TTimeline::TTimeline(TRefRef AssetRef) :
	TAsset	( GetAssetType_Static(), AssetRef )
{
}



//-------------------------------------------------------
//	load asset data out binary data
//-------------------------------------------------------
SyncBool TTimeline::ImportData(TBinaryTree& Data)		
{
#ifdef DEBUG_PRINT_TIMELINE_TREE
	// Output the binary tree
	Data.Debug_PrintTree();
#endif

	u32 uKeyframeCount = Data.GetChildren().GetSize();

	// Read each keyframe
	for(u32 uIndex = 0; uIndex < uKeyframeCount; uIndex++)
	{
		TPtr<TBinaryTree> pKeyframeTree = Data.GetChildren().ElementAt(uIndex);
		pKeyframeTree->ResetReadPos();

		float fTime = 0.0f;
		// Write the keyframe time
		if(!pKeyframeTree->Read(fTime))
		{
			TLDebug_Print("Failed to read time from TTL asset file");
			return SyncFalse;
		}

		TKeyframe* pKeyframe = AddKeyframe(fTime);

		if(pKeyframe == NULL)
		{
			TLDebug_Print("Failed to create keyframe from TTL asset file");
			return SyncFalse;
		}

		SyncBool KeyRes = pKeyframe->ImportData(*pKeyframeTree);

		if(KeyRes != SyncTrue)
			return KeyRes;

	}

	return SyncTrue;

}


//-------------------------------------------------------
//	save asset data to binary data
//-------------------------------------------------------
SyncBool TTimeline::ExportData(TBinaryTree& Data)				
{	
	// Write each keyframe out
	for(u32 uIndex = 0; uIndex < m_Keyframes.GetSize(); uIndex++)
	{
		const TLKeyArray::TPair<float,TPtr<TKeyframe> >& Pair = m_Keyframes.GetPairAt( uIndex );

		TPtr<TBinaryTree> pKeyframeTree = Data.AddChild("Keyframe");

		// Write the keyframe time
		pKeyframeTree->Write(Pair.m_Key);

		TPtr<TKeyframe> pKeyframe = Pair.m_Item;

		SyncBool KeyRes = pKeyframe->ExportData(*pKeyframeTree);

		if(KeyRes != SyncTrue)
			return KeyRes;
	}

#ifdef DEBUG_PRINT_TIMELINE_TREE
	// Output the binary tree
	Data.Debug_PrintTree();
#endif

	return SyncTrue;
}	


float TTimeline::GetLastKeyFrameTime()
{
	float fTime = 0.0f;

	u32 uKeyframeCount = m_Keyframes.GetSize();
	if(uKeyframeCount > 0)
	{
		TLKeyArray::TPair<float,TPtr<TKeyframe> >& Pair = m_Keyframes.GetPairAt( uKeyframeCount - 1 );

		// Time of the keyframe
		return Pair.m_Key;
	}

	return fTime;
}



Bool TTimeline::GetKeyframes(const float& fTimeFrom,const float& fTimeStep, TArray<TTempKeyframeData>& pKeyframes, Bool bAllowNoTimestep)
{
	// No timestep?  Then ther's no need to process the keyframes
	if((fTimeStep == 0.0f) && (!bAllowNoTimestep))
		return FALSE;

	// Do we have any keyframes to check?
	if(m_Keyframes.GetSize() == 0)
		return FALSE;

	float fTimeTo = fTimeFrom + fTimeStep;

	// Check the timestep sign - are we looking forwards or backwards?
	// If forwards use the forward find routine, otherwise use the backwards find routine

	if(fTimeStep >= 0.0f)
		return GetKeyframes_Forward(fTimeFrom, fTimeTo, pKeyframes);
	else
		return GetKeyframes_Backward(fTimeTo, fTimeFrom, pKeyframes);

}

Bool TTimeline::GetKeyframes_Forward(const float& fTimeFrom,const float& fTimeTo, TArray<TTempKeyframeData>& pKeyframes)
{
	TTempKeyframeData firstkey;
	Bool bAddFirstKey = TRUE;
	Bool bAddLastKey = TRUE;

	// Get the keyframes that the time from and time to will span
	for(u32 uIndex = 0; uIndex < m_Keyframes.GetSize(); uIndex++)
	{
		TLKeyArray::TPair<float,TPtr<TKeyframe> >& Pair = m_Keyframes.GetPairAt( uIndex );

		// Time of the keyframe
		float fTime = Pair.m_Key;

		// If the keyframe time is larger than the time we are searching to then we can 
		// return as no further keyframes will be within range
#ifdef USE_KEYFRAME_TOLERANCE_TESTS		
		if(TLMaths::FloatGreaterThan(fTime, fTimeTo, TIMELINE_KEYFRAME_TOLERANCE))
#else
		if(fTime > fTimeTo)
#endif
		{
			if(bAddFirstKey)
			{
				pKeyframes.InsertAt(0, firstkey);
			}

			if(bAddLastKey)
			{
				// This is the last key we will come across that is beyond the time range so
				// add it to the array as it may be needed for interping to as a last keyframe
				TTempKeyframeData data;
				data.m_fTime = fTime;
				data.m_pKeyframe = Pair.m_Item.GetObjectPointer();

				pKeyframes.Add(data);
			}

			return TRUE;
		}
#ifdef USE_KEYFRAME_TOLERANCE_TESTS		
		else if(TLMaths::FloatEqual(fTime, fTimeFrom, TIMELINE_KEYFRAME_TOLERANCE))
#else
		else if (fTime == fTimeFrom)
#endif
		{
			// First keyframe
			bAddFirstKey = FALSE;

			// Add the keyframe temp data
			TTempKeyframeData data;
			data.m_fTime = fTime;
			data.m_pKeyframe = Pair.m_Item.GetObjectPointer();

			pKeyframes.Add(data);
		}
#ifdef USE_KEYFRAME_TOLERANCE_TESTS		
		else if(TLMaths::FloatEqual(fTime, fTimeTo, TIMELINE_KEYFRAME_TOLERANCE))
#else
		else if (fTime == fTimeTo)
#endif
		{
			if(bAddFirstKey)
			{
				pKeyframes.Add(firstkey);
			}
			
			
			// Last keyframe
			bAddLastKey = FALSE;

			// Add the keyframe temp data
			TTempKeyframeData data;
			data.m_fTime = fTime;
			data.m_pKeyframe = Pair.m_Item.GetObjectPointer();

			pKeyframes.Add(data);

			// We can return now as now more keys will match
			return TRUE;
		}
#ifdef USE_KEYFRAME_TOLERANCE_TESTS		
		else if(TLMaths::FloatGreaterThan(fTime, fTimeFrom, TIMELINE_KEYFRAME_TOLERANCE) && TLMaths::FloatLessThan(fTime, fTimeTo, TIMELINE_KEYFRAME_TOLERANCE))
#else
		else if(fTime > fTimeFrom && fTime < fTimeTo)
#endif
		{
			// Keyframe in between to and from time range
			// Add the keyframe temp data
			TTempKeyframeData data;
			data.m_fTime = fTime;
			data.m_pKeyframe = Pair.m_Item.GetObjectPointer();

			pKeyframes.Add(data);

			// Is this the last keyframe?
			if(uIndex == (m_Keyframes.GetSize()-1))
			{
				// Insert the previous key
				if(bAddFirstKey)
					pKeyframes.InsertAt(0, firstkey);

				return TRUE;
			}
		}
#ifdef USE_KEYFRAME_TOLERANCE_TESTS		
		else if(TLMaths::FloatLessThan(fTime, fTimeFrom, TIMELINE_KEYFRAME_TOLERANCE))
#else
		else if(fTime < fTimeFrom)
#endif
		{
			// Potential first key
			firstkey.m_fTime = fTime;
			firstkey.m_pKeyframe = Pair.m_Item.GetObjectPointer();
		}
		/*
		else if((uIndex == (m_Keyframes.GetSize()-1)) && (fTime > fTimeFrom))
		{
			// Are we going beyond the range of the keyframes?
			// If so this will be the last one to be added
			if(bAddFirstKey)
			{
				pKeyframes.InsertAt(0, firstkey);
			}

			if(bAddLastKey)
			{
				// This is the last key we will come across that is beyond the time range so
				// add it to the array as it may be needed for interping to as a last keyframe
				TTempKeyframeData data;
				data.m_fTime = fTime;
				data.m_pKeyframe = Pair.m_Item.GetObjectPointer();

				pKeyframes.Add(data);
			}

			return TRUE;
		}
		*/

	}

	return (pKeyframes.GetSize() > 0);
}


Bool TTimeline::GetKeyframes_Backward(const float& fTimeFrom,const float& fTimeTo, TArray<TTempKeyframeData>& pKeyframes)
{
	// Get the keyframes that the time from and time to will span
	for(u32 uIndex = (m_Keyframes.GetSize() - 1); uIndex >= 0; uIndex--)
	{
		TLKeyArray::TPair<float,TPtr<TKeyframe> >& Pair = m_Keyframes.GetPairAt( uIndex );

		// Time of the keyframe
		float fTime = Pair.m_Key;

		// If the keyframe time is larger than the time we are searching to then we can 
		// return as no further keyframes will be within range
		if(fTime < fTimeTo)
		{
			// This is the frist and last one we come across that is beyond the time range so
			// add it to the array as it may be needed for interping to as a last keyframe
			TTempKeyframeData data;
			data.m_fTime = fTime;
			data.m_pKeyframe = Pair.m_Item.GetObjectPointer();

			pKeyframes.Add(data);
			return TRUE;
		}

		if(fTime <= fTimeFrom && fTime >= fTimeTo)
		{
			// Add the keyframe temp data
			TTempKeyframeData data;
			data.m_fTime = fTime;
			data.m_pKeyframe = Pair.m_Item.GetObjectPointer();

			pKeyframes.Add(data);
		}

	}


	return FALSE;
}


