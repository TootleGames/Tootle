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




TAssetScript::TAssetScript(const TRef& AssetRef) :
TAsset	( "AScript", AssetRef )
{
}



//-------------------------------------------------------
//	load asset data out binary data
//-------------------------------------------------------
SyncBool TAssetScript::ImportData(TBinaryTree& Data)		
{
	u32 uKeyframeCount = 0;

	// Read how many keyframes we have
	if(!Data.Read(uKeyframeCount))
	{
		TLDebug_Print("Failed to read keyframe count from TAS asset file");
		return SyncFalse;
	}

	// Read each keyframe
	for(u32 uCount = 0; uCount < uKeyframeCount; uCount++)
	{
		float fTime = 0.0f;
		// Write the keyframe time
		if(!Data.Read(fTime))
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

		u32 uCommandCount = 0;

		// Read how many commands we have
		if(!Data.Read(uCommandCount))
		{
			// Failed to read data
			TLDebug_Print("Failed to read command count from TAS asset file");
			return SyncFalse;
		}

		if(uCommandCount > 0)
		{
			TArray<TAssetScriptCommand>& cmds = pKeyframe->GetCommands();

			// Allocate the number of commands we need
			cmds.SetAllocSize(uCommandCount);

			// Now write out each command
			for(u32 uCount2 = 0; uCount2 < uCommandCount; uCount2++)
			{
				TAssetScriptCommand cmd;

				if(!Data.Read(cmd))
				{
					// Failed to read command data
					TLDebug_Print("Failed to read command data from TAS asset file");
					return SyncFalse;
				}

				// Add the command to the array
				cmds.Add(cmd);
			}
		}
	}

	return SyncTrue;

}


//-------------------------------------------------------
//	save asset data to binary data
//-------------------------------------------------------
SyncBool TAssetScript::ExportData(TBinaryTree& Data)				
{	
	// Write out how many keyframes we have
	Data.Write(m_Keyframes.GetSize());

	// Write each keyframe out
	for(u32 uCount = 0; uCount < m_Keyframes.GetSize(); uCount++)
	{
		TKeyframe&	keyframe = m_Keyframes.ElementAt(uCount);

		// Write the keyframe time
		Data.Write(keyframe.GetTime());

		TArray<TAssetScriptCommand>& cmds = keyframe.GetCommands();

		// Write out how many commands we have
		Data.Write(cmds.GetSize());

		// Now write out each command
		for(u32 uCount2 = 0; uCount2 < cmds.GetSize(); uCount2++)
		{
			TAssetScriptCommand& cmd = cmds.ElementAt(uCount2);

			Data.Write(cmd);
		}
	}

	return SyncTrue;
}	


Bool TAssetScript::GetKeyframes(const float& m_fTime,const float& fTimeStep, TArray<TKeyframe*>& pKeyframes)
{
	return FALSE;
}

