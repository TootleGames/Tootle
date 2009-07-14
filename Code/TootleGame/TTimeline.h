#pragma once

#include <TootleCore/TFlags.h>
#include <TootleCore/TKeyArray.h>
#include <TootleAsset/TAssetTimeline.h>
#include <TootleCore/TRelay.h>
#include <TootleCore/TCoreManager.h>
#include <TootleCore/TGraphBase.h>


namespace TLAnimation
{
	class TTimelineInstance;

	// Special commands
	const TRef TimeJumpRef	= TRef_Static(T,i,m,e,j);	//"Timejump" command used in timelines
};



class TLAnimation::TTimelineInstance : public TLMessaging::TPublisherSubscriber
{
public:
	enum TimelineFlags
	{
		AutoUpdate = 0,
		Pause,
	};

public:
	TTimelineInstance(TRefRef AssetScriptRef) :
		m_AssetScriptRef(AssetScriptRef),
		m_fTime(0.0f),
		m_fPlaybackRateModifier(1.0f)
	{
		TLAsset::LoadAsset( m_AssetScriptRef );
	}

	void Initialise(TLMessaging::TMessage& InitMessage);

	FORCEINLINE SyncBool	Update(float fTimestep)			{	return DoUpdate(fTimestep, FALSE);	}
	FORCEINLINE void		BindTo(TRefRef NodeRef)			{ 	MapNodeRef(TRef("this"), NodeRef);	}
	FORCEINLINE void		MapNodeRef(TRefRef FromRef, TRefRef ToRef);

	// Set the time
	void					SetTime(float fTime);
	FORCEINLINE float		GetTime()			const	{ return m_fTime; }

	// Set the playback rate modifier
	FORCEINLINE void		SetPlaybackRateModifier(const float& fRateModifier)	{ m_fPlaybackRateModifier = fRateModifier; }
	FORCEINLINE float		GetPlaybackRateModifier()					const	{ return m_fPlaybackRateModifier; }

	//	valid asset?
	FORCEINLINE Bool		GetAssetExists() const								{	return TLAsset::LoadAsset( m_AssetScriptRef, TRUE, "Timeline" ).IsValid();	}

protected:
	virtual void		ProcessMessage(TLMessaging::TMessage& Message);

private:
	SyncBool			DoUpdate(float fTimestep, Bool bForced);

	TLAsset::TAssetTimeline*			GetAssetTimeline();
	FORCEINLINE TArray<TRef>*			GetGraphNodeRefArray(TRefRef GraphRef);
	FORCEINLINE TLGraph::TGraphBase*	GetGraph(TRefRef GraphRef);

	// Keyframe processing
	Bool					ProcessKeyframes(const TLAsset::TTempKeyframeData& KeyframeFrom, const TLAsset::TTempKeyframeData& KeyframeTo, float& fTimestep);
	Bool					ProcessFinalKeyframe(const TLAsset::TTempKeyframeData& Keyframe);

	// Command handling
	Bool					SendCommandAsMessage(TLAsset::TAssetTimelineCommand* pFromCommand, TLAsset::TAssetTimelineCommand* pToCommand, TRef NodeGraphRef, TRef NodeRef, float fPercent = 0.0f, Bool bTestDataForInterp = FALSE);

	void					AttachInterpedDataToMessage(TPtr<TBinaryTree>& pFromData, TPtr<TBinaryTree>& pToData, TRefRef InterpMethod, const float& fPercent, TLMessaging::TMessage& Message);

	// Events
	void					OnEndOfTimeline();
	void					OnTimeSet();

private:
	TRef						m_AssetScriptRef;			// Ref of the Asset script object loaded from the XML data that we are using
	float						m_fTime;					// Current time of the asset script instance
	float						m_fPlaybackRateModifier;	// Playback rate modifier. Allows you to pause, play forward and backward at any speed

	TFlags<TimelineFlags>		m_Flags;					// Optional flags for the instance

	TKeyArray<TRef, TRef>		m_NodeRefMap;				// Node ref mapping - used for when nodes are added via the timeline commands
	TArray<TRef>				m_CreatedSceneNodes;		// List of scene nodes created
	TArray<TRef>				m_CreatedRenderNodes;		// List of render nodes created
	TArray<TRef>				m_CreatedPhysicsNodes;		// List of physics nodes created
	TArray<TRef>				m_CreatedAudioNodes;		// List of audio nodes created
};



FORCEINLINE void TLAnimation::TTimelineInstance::MapNodeRef(TRefRef FromRef, TRefRef ToRef)	
{ 
	TRef* pNodeRef = m_NodeRefMap.Find(FromRef);
	if(pNodeRef == NULL)
	{
		m_NodeRefMap.Add(FromRef, ToRef);
	}
	else
	{
		// Alter the node ref to the one being passed in
		*pNodeRef = ToRef;
	}
}


FORCEINLINE TArray<TRef>* TLAnimation::TTimelineInstance::GetGraphNodeRefArray(TRefRef GraphRef)
{
	switch ( GraphRef.GetData() )
	{
		case TRef_Static(R,e,n,d,e):	return &m_CreatedRenderNodes;
		case TRef_Static(P,h,y,s,i):	return &m_CreatedPhysicsNodes;
		case TRef_Static(A,u,d,i,o):	return &m_CreatedAudioNodes;

		default:	//	gr: old code defaulted to scene node... I assume this is as designed instead of throwing up an "unknown graph" error...
		case TRef_Static(S,c,e,n,e):	return &m_CreatedSceneNodes;
	};
}

FORCEINLINE TLGraph::TGraphBase* TLAnimation::TTimelineInstance::GetGraph(TRefRef GraphRef)
{
	TLGraph::TGraphBase* pGraph = TLCore::g_pCoreManager->GetManager<TLGraph::TGraphBase>( GraphRef );
	return pGraph;
}
