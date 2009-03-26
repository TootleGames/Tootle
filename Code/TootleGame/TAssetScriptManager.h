


#pragma once

#include <TootleCore/TManager.h>
#include <TootleCore/TFlags.h>

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
	enum TimelineFlags
	{
		AutoUpdate = 0,
		Pause,
	};

public:
	TAssetScriptInstance(TRefRef AssetScriptRef) :
		m_AssetScriptRef(AssetScriptRef),
		m_fTime(0.0f)
	{
	}

	void	Update(float fTimestep);

	FORCEINLINE void	BindTo(TRefRef NodeRef, TRefRef NodeGraphRef = TRef())	{ m_NodeRef = NodeRef; m_NodeGraphRef = NodeGraphRef; }

private:
	TLAsset::TAssetScript*	GetAssetScript();

	Bool					ProcessKeyframes(const TLAsset::TTempKeyframeData& KeyframeFrom, const TLAsset::TTempKeyframeData& KeyframeTo, float& fTimestep);
	Bool					SendCommandAsMessage(TLAsset::TAssetScriptCommand& Command, TRef NodeGraphRef, TRef NodeRef);
	Bool					SendInterpedCommandAsMessage(TLAsset::TAssetScriptCommand& FromCommand, TLAsset::TAssetScriptCommand& ToCommand, TRef NodeGraphRef, TRef NodeRef, float fPercent);


private:
	TRef						m_AssetScriptRef;		// Ref of the Asset script object loaded from the XML data that we are using
	float						m_fTime;				// Current time of the asset script instance

	TRef						m_NodeRef;				// Node ref override - for use when "this" is used for the node ref
	TRef						m_NodeGraphRef;			// Node graph ref override - for use when "this" is used for the node ref 

	TFlags<TimelineFlags>		m_Flags;				// Optional flags for the instance
};

class TLAnimation::TAssetScriptManager : public TManager
{
public:
	TAssetScriptManager(TRefRef ManagerRef) :
	  TManager(ManagerRef)
	{
	}

	// Asset script instance access
	TPtr<TLAnimation::TAssetScriptInstance>		CreateTimeline(TRefRef AssetScriptRef);
	void										DeleteTimeline(TPtr<TLAnimation::TAssetScriptInstance>);

protected:
	virtual SyncBool	Shutdown();
	virtual SyncBool	Update(float fTimestep);

	virtual void	OnEventChannelAdded(TRefRef refPublisherID, TRefRef refChannelID);

private:

	TPtrArray<TLAnimation::TAssetScriptInstance>	m_TimelineInstances;
};