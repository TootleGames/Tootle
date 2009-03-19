
#include "TAssetScriptManager.h"

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

	TArray<TLAsset::TKeyframe*> pKeyframes;

	// We could cross multiple keyframes in one step so get all keyframes we are going to span
	// So we can send out all changes as required
	if(pScript->GetKeyframes(m_fTime, fTimestep, pKeyframes))
	{
		// Process the keyframes. Keyframes will be in order

		for(u32 uIndex = 0; uIndex < pKeyframes.GetSize(); uIndex++)
		{
			// Get the 'from' keyframe
			TLAsset::TKeyframe* pKeyframeFrom = pKeyframes.ElementAt(uIndex);

			if(uIndex+1 < pKeyframes.GetSize())
			{
				// Get the 'to' keyframe
				TLAsset::TKeyframe* pKeyframeTo = pKeyframes.ElementAt(uIndex);

				ProcessKeyframes(pKeyframeFrom, pKeyframeTo, fTimestep);
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


void TAssetScriptInstance::ProcessKeyframes(const TLAsset::TKeyframe* pKeyframeFrom, const TLAsset::TKeyframe* pKeyframeTo, float& fTimestep)
{
	
}







// Creates an insstance of a TAssetScriptInstance and binds to the object calling the method
Bool TAssetScriptManager::BindToScript(TRefRef AssetScriptRef, TLAnimation::TAssetScriptInstance* pInstance)
{
	return FALSE;
}


