


#pragma once

#include <TootleCore/TManager.h>

#include <TootleAsset/TAssetScript.h>


namespace TLAnimation
{
	class TAssetScriptInstance;
	class TAssetScriptManager;

	extern TPtr<TAssetScriptManager> g_pAssetScriptManager;
}

class TLAnimation::TAssetScriptInstance
{
public:
	TAssetScriptInstance() :
		m_fTime(0.0f)
	{
	}

	void	Update(float fTimestep);

private:
	TLAsset::TAssetScript*	GetAssetScript();

	void					ProcessKeyframes(const TLAsset::TKeyframe* pKeyframeFrom, const TLAsset::TKeyframe* pKeyframeTo, float& fTimestep);


private:
	TRef	m_AssetScriptRef;		// Ref of the Asset script object loaded from the XML data that we are using
	float	m_fTime;				// Current time of the asset script instance
};

class TLAnimation::TAssetScriptManager : public TManager
{
public:
	TAssetScriptManager(TRefRef ManagerRef) :
	  TManager(ManagerRef)
	{
	}

	Bool		BindToScript(TRefRef AssetScriptRef, TLAnimation::TAssetScriptInstance* pInstance);		// Creates an insstance of a TAssetScriptInstance and binds to the object calling the method

private:

	TArray<TLAnimation::TAssetScriptInstance>	m_ScriptInstances;
};