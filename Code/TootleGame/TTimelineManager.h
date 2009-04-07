


#pragma once

#include <TootleCore/TManager.h>


#include "TTimeline.h"


namespace TLAnimation
{
	class TTimelineManager;

	extern TPtr<TTimelineManager> g_pTimelineManager;
}


class TLAnimation::TTimelineManager : public TManager
{
public:
	TTimelineManager(TRefRef ManagerRef) :
	  TManager(ManagerRef)
	{
	}

	// Asset script instance access
	TPtr<TLAnimation::TTimelineInstance>		CreateTimeline(TRefRef AssetScriptRef);
	void										DeleteTimeline(TPtr<TLAnimation::TTimelineInstance>);

protected:
	virtual SyncBool	Shutdown();
	virtual SyncBool	Update(float fTimestep);

	virtual void	OnEventChannelAdded(TRefRef refPublisherID, TRefRef refChannelID);

private:

	TPtrArray<TLAnimation::TTimelineInstance>	m_TimelineInstances;
};