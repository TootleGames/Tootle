#pragma once

#include <TootleCore/TFlags.h>
#include <TootleCore/TKeyArray.h>
#include <TootleAsset/TAssetTimeline.h>
#include <TootleCore/TPublisher.h>

namespace TLAnimation
{
	class TTimelineInstance;

	// Graph references
	const TRef ScenegraphRef	= TRef_Static(S,c,e,n,e);	//"Scene" used in timelines
	const TRef RendergraphRef	= TRef_Static(R,e,n,d,e);	//"Render" used in timelines
	const TRef AudiographRef	= TRef_Static(A,u,d,i,o);	//"Audio" used in timelines
	const TRef PhysicsgraphRef	= TRef_Static(P,h,y,s,i);	//"Physics" used in timelines

	// Special commands
	const TRef TimeJumpRef	= TRef_Static(T,i,m,e,j);	//"Timejump" command used in timelines
};



class TLAnimation::TTimelineInstance : public TLMessaging::TPublisher
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

private:

	SyncBool			DoUpdate(float fTimestep, Bool bForced);


	TLAsset::TAssetTimeline*	GetAssetTimeline();

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
