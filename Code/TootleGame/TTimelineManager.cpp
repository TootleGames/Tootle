
#include "TTimelineManager.h"


namespace TLAnimation
{
	TPtr<TTimelineManager> g_pTimelineManager = NULL;
}

using namespace TLAnimation;



TPtr<TLAnimation::TTimelineInstance> TTimelineManager::CreateTimeline(TRefRef AssetScriptRef)
{
	TPtr<TLAnimation::TTimelineInstance> pInstance = new TLAnimation::TTimelineInstance(AssetScriptRef);

	if(!pInstance)
		return TPtr<TLAnimation::TTimelineInstance>(NULL);

	if(m_TimelineInstances.Add(pInstance) == -1)
		return TPtr<TLAnimation::TTimelineInstance>(NULL);

	// Success
	return pInstance;
}

void TTimelineManager::DeleteTimeline(TPtr<TLAnimation::TTimelineInstance> pInstance)
{
	//m_TimelineInstances.Remove(pInstance);
}

SyncBool TTimelineManager::Shutdown()
{
	// Remove all instances of the timelines
	m_TimelineInstances.Empty(TRUE);

	return TLCore::TManager::Shutdown();
}

SyncBool TTimelineManager::Update(float fTimestep)
{
	return TLCore::TManager::Update(fTimestep);
}

void TTimelineManager::OnEventChannelAdded(TRefRef refPublisherID, TRefRef refChannelID)
{
	TLCore::TManager::OnEventChannelAdded(refPublisherID, refChannelID);
}


