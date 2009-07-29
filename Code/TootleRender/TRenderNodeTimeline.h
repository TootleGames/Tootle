/*------------------------------------------------------

	Render node with an automaticly updated timeline attached
	to it

-------------------------------------------------------*/
#pragma once

#include "TRenderNode.h"
#include <TootleGame/TTimeline.h>


namespace TLRender
{
	class TRenderNodeTimeline;
}


//----------------------------------------------------
//	
//----------------------------------------------------
class TLRender::TRenderNodeTimeline : public TLRender::TRenderNode
{
public:
	TRenderNodeTimeline(TRefRef RenderNodeRef,TRefRef TypeRef);

	void					SetTimeline(TRefRef NewTimeline);			//	create/change timeline

protected:
	virtual void			Initialise(TLMessaging::TMessage& Message);
	virtual void			Shutdown()									{	m_pTimeline = NULL;	TLRender::TRenderNode::Shutdown();	}
	virtual void			SetProperty(TLMessaging::TMessage& Message);
	virtual void			UpdateNodeData();
	virtual void			ProcessMessage(TLMessaging::TMessage& Message);

public:
	TRef					m_TimelineRef;			//	ref of the timeline
	Bool					m_Playing;				//	simple ability to stop/start the timeline

	TPtr<TLAnimation::TTimelineInstance>	m_pTimeline;	//	instance of timeline
};


