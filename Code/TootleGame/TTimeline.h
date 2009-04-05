#pragma once

#include <TootleCore/TFlags.h>
#include <TootleCore/TKeyArray.h>
#include <TootleAsset/TAssetTimeline.h>
#include <TootleCore/TPublisher.h>

namespace TLAnimation
{
	class TTimelineInstance;
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
		m_fTime(0.0f)
	{
	}

	void	Update(float fTimestep);

	FORCEINLINE void	BindTo(TRefRef NodeRef)	
	{ 
		MapNodeRef(TRef("this"), NodeRef);
	}

	FORCEINLINE void	MapNodeRef(TRefRef FromRef, TRefRef ToRef)	
	{ 
		TRef* pNodeRef = m_NodeRefMap.Find(FromRef);
		if(pNodeRef == NULL)
			m_NodeRefMap.Add(FromRef, ToRef);
		else
		{
			// Alter the node ref to the one being passed in
			*pNodeRef = ToRef;
		}
	}


	

private:
	TLAsset::TAssetTimeline*	GeTAssetTimeline();

	// Keyframe processing
	Bool					ProcessKeyframes(const TLAsset::TTempKeyframeData& KeyframeFrom, const TLAsset::TTempKeyframeData& KeyframeTo, float& fTimestep);
	Bool					ProcessFinalKeyframe(const TLAsset::TTempKeyframeData& Keyframe);

	// Command handling
	Bool					SendCommandAsMessage(TLAsset::TAssetTimelineCommand& Command, TRef NodeGraphRef, TRef NodeRef);
	Bool					SendInterpedCommandAsMessage(TLAsset::TAssetTimelineCommand& FromCommand, TLAsset::TAssetTimelineCommand& ToCommand, TRef NodeGraphRef, TRef NodeRef, TRefRef MessageRef, float fPercent);

	void					AttachInterpedDataToMessage(TPtr<TBinaryTree>& pFromData, TPtr<TBinaryTree>& pToData, TRefRef InterpMethod, const float& fPercent, TLMessaging::TMessage& Message);

	// Events
	void					OnEndOfTimeline();

private:
	TRef						m_AssetScriptRef;		// Ref of the Asset script object loaded from the XML data that we are using
	float						m_fTime;				// Current time of the asset script instance

	TFlags<TimelineFlags>		m_Flags;				// Optional flags for the instance

	TKeyArray<TRef, TRef>		m_NodeRefMap;			// Node ref mapping - used for when nodes are added via the timeline commands
};
