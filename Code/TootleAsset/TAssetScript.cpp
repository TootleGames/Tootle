/*
 *  TAssetScript.cpp
 *  TootleAsset
 *
 *  Created by Duane Bradbury on 15/03/2009.
 *  Copyright 2009 Tootle. All rights reserved.
 *
 */

#include "TAssetScript.h"


using namespace TLAsset;



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
			TPtr<TLAsset::TAssetScriptCommandList> pCommandList = new TLAsset::TAssetScriptCommandList();

			if(!pCommandList || (Add(pCommandList) == -1))
			{
				TLDebug_Print("Failed to create new command list for TAS asset file");
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
			TPtr<TAssetScriptCommandList>& cmdslist = ElementAt(uIndex);

			SyncBool CmdRes = cmdslist->ExportData(*pNodeTree);

			if(CmdRes != SyncTrue)
				return CmdRes;
		}
	}

	return SyncTrue;
}




SyncBool TAssetScriptCommandList::ImportData(TBinaryTree& Data)
{

	// Read the Node ref
	if(!Data.Read(m_NodeRef))
	{
		// Failed to read data
		TLDebug_Print("Failed to read node ref from TAS asset file");
		return SyncFalse;
	}

	// Read the graph ref
	if(!Data.Read(m_NodeGraphRef))
	{
		// Failed to read data
		TLDebug_Print("Failed to read node graph ref from TAS asset file");
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


			TRef CommandRef;
			pCommand->Read(CommandRef);

			// Create the command
			TLAsset::TAssetScriptCommand cmd(CommandRef);

			// Read hte interp method
			TLAsset::TAssetScriptCommand::InterpMethod im;
			pCommand->Read(im);
			cmd.SetInterpMethod(im);

			// Get the data type hint
			TRef DataTypeHint;
			pCommand->Read(DataTypeHint);
			cmd.SetDataTypeHint(DataTypeHint);

			// Read the data? Not sure this is correct for the TBinaryTree?
			pCommand->Read(cmd.GetData());
			
			//cmd.ImportData(*pCommandData);

			// Add the command to the array
			m_Commands.Add(cmd);
		}
	}


	return SyncTrue;
}

SyncBool TAssetScriptCommandList::ExportData(TBinaryTree& Data)
{
	Data.Write(m_NodeRef);
	Data.Write(m_NodeGraphRef);

	TPtr<TBinaryTree> pCommandData = Data.AddChild("Commands");

	if(pCommandData)
	{
		for(u32 uIndex = 0; uIndex < m_Commands.GetSize(); uIndex++)
		{
			TAssetScriptCommand& cmd = m_Commands.ElementAt(uIndex);
			
			TPtr<TBinaryTree> pCommand = pCommandData->AddChild("Command");
			pCommand->Write(cmd.GetMessageRef());
			pCommand->Write(cmd.GetInterpMethod());
			pCommand->Write(cmd.GetDataTypeHint());
			pCommand->Write(cmd.GetData());
			
			//cmd.ExportData(*pCommandData);
		}
	}

	return SyncTrue;
}



/*
SyncBool TAssetScriptCommand::ImportData(TBinaryTree& Data)
{
	Data.Read(m_uInterpMethod);

	/*
	TPtr<TBinaryTree> pCommandData = Data.GetChild("Command");

	CopyDataTree(pCommandData);
	//*

	return SyncTrue;
}

SyncBool TAssetScriptCommand::ExportData(TBinaryTree& Data)
{
	Data.Write(m_uInterpMethod);

	/*
	TPtr<TBinaryTree> pCommandData = Data.AddChild("Command");

	pCommandData->CopyDataTree(this);
	//*

	return SyncTrue;
}
*/


TAssetScript::TAssetScript(const TRef& AssetRef) :
TAsset	( "AScript", AssetRef )
{
}



//-------------------------------------------------------
//	load asset data out binary data
//-------------------------------------------------------
SyncBool TAssetScript::ImportData(TBinaryTree& Data)		
{
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
			TLDebug_Print("Failed to read time from TAS asset file");
			return SyncFalse;
		}

		TKeyframe* pKeyframe = AddKeyframe(fTime);

		if(pKeyframe == NULL)
		{
			TLDebug_Print("Failed to create keyframe from TAS asset file");
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
SyncBool TAssetScript::ExportData(TBinaryTree& Data)				
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

	return SyncTrue;
}	


Bool TAssetScript::GetKeyframes(const float& fTimeFrom,const float& fTimeStep, TArray<TTempKeyframeData>& pKeyframes)
{
	// No timestep?  Then ther's no need to process the keyframes
	if(fTimeStep == 0.0f)
		return FALSE;

	// Do we have any keyframes to check?
	if(m_Keyframes.GetSize() == 0)
		return FALSE;

	float fTimeTo = fTimeFrom + fTimeStep;

	// Check the timestep sign - are we looking forwards or backwards?
	// If forwards use the forward find routine, otherwise use the backwards find routine

	if(fTimeStep > 0.0f)
		return GetKeyframes_Forward(fTimeFrom, fTimeTo, pKeyframes);
	else
		return GetKeyframes_Backward(fTimeTo, fTimeFrom, pKeyframes);

}

Bool TAssetScript::GetKeyframes_Forward(const float& fTimeFrom,const float& fTimeTo, TArray<TTempKeyframeData>& pKeyframes)
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
		if(fTime > fTimeTo)
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
				data.m_pKeyframe = Pair.m_Item.GetObject();

				pKeyframes.Add(data);
			}

			return TRUE;
		}
		else if (fTime == fTimeFrom)
		{
			// First keyframe
			bAddFirstKey = FALSE;

			// Add the keyframe temp data
			TTempKeyframeData data;
			data.m_fTime = fTime;
			data.m_pKeyframe = Pair.m_Item.GetObject();

			pKeyframes.Add(data);
		}
		else if (fTime == fTimeTo)
		{
			// Last keyframe
			bAddLastKey = FALSE;

			// Add the keyframe temp data
			TTempKeyframeData data;
			data.m_fTime = fTime;
			data.m_pKeyframe = Pair.m_Item.GetObject();

			pKeyframes.Add(data);

			// We can retunr now as now more keys will match
			return TRUE;
		}
		else if(fTime > fTimeFrom && fTime < fTimeTo)
		{
			// Keyframe in between to and from time range
			// Add the keyframe temp data
			TTempKeyframeData data;
			data.m_fTime = fTime;
			data.m_pKeyframe = Pair.m_Item.GetObject();

			pKeyframes.Add(data);
		}
		else if(fTime < fTimeFrom)
		{
			// Potential first key
			firstkey.m_fTime = fTime;
			firstkey.m_pKeyframe = Pair.m_Item.GetObject();
		}

	}

	return FALSE;
}


Bool TAssetScript::GetKeyframes_Backward(const float& fTimeFrom,const float& fTimeTo, TArray<TTempKeyframeData>& pKeyframes)
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
			data.m_pKeyframe = Pair.m_Item.GetObject();

			pKeyframes.Add(data);
			return TRUE;
		}

		if(fTime <= fTimeFrom && fTime >= fTimeTo)
		{
			// Add the keyframe temp data
			TTempKeyframeData data;
			data.m_fTime = fTime;
			data.m_pKeyframe = Pair.m_Item.GetObject();

			pKeyframes.Add(data);
		}

	}


	return FALSE;
}


