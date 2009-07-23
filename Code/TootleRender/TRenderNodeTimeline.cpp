#include "TRenderNodeTimeline.h"




TLRender::TRenderNodeTimeline::TRenderNodeTimeline(TRefRef RenderNodeRef,TRefRef TypeRef) :
	TRenderNode	( RenderNodeRef, TypeRef ),
	m_Playing	( TRUE )
{
	//	subscribe to updates
	//	gr: change this so the game or something dictates updates - not the core
	this->SubscribeTo( TLCore::g_pCoreManager );
}


//---------------------------------------------------------
//	set properties
//---------------------------------------------------------
void TLRender::TRenderNodeTimeline::SetProperty(TLMessaging::TMessage& Message)
{
	TRef NewTimeline;
	if ( Message.ImportData("Timeline", NewTimeline ) )
		SetTimeline( NewTimeline );

	//	change playing state
	Message.ImportData("Play", m_Playing );

	//	set time on timeline
	float Time;
	if ( Message.ImportData("Time", Time ) )
		if ( m_pTimeline )
			m_pTimeline->SetTime( Time );

	TLRender::TRenderNode::SetProperty(Message);
}


//---------------------------------------------------------
//	
//---------------------------------------------------------
void TLRender::TRenderNodeTimeline::UpdateNodeData()
{
	//	set timeline ref
	GetNodeData().RemoveChild("Timeline");
	GetNodeData().ExportData("Timeline", m_TimelineRef );

	GetNodeData().RemoveChild("Play");
	GetNodeData().ExportData("Play", m_Playing );

	//	export current time for state saving
	GetNodeData().RemoveChild("Time");
	if ( m_pTimeline )
	{
		float Time = m_pTimeline->GetTime();
		GetNodeData().ExportData("Time", Time );
	}

	TLRender::TRenderNode::UpdateNodeData();
}

//---------------------------------------------------------
//	
//---------------------------------------------------------
void TLRender::TRenderNodeTimeline::ProcessMessage(TLMessaging::TMessage& Message)
{
	//	if we get a "reset" message, set the time back to the start
	if ( m_pTimeline && Message.GetMessageRef() == "Reset" )
	{
		m_pTimeline->SetTime(0.f);
		return;
	}
	
	//	update from core
	if ( Message.GetMessageRef() == TLCore::UpdateRef )
	{
		float Timestep = 0.f;
		Message.ResetReadPos();
		if ( Message.Read( Timestep ) )
		{
			//	animate timeline if we have it and it's not paused
			if ( m_pTimeline && m_Playing )
			{
				m_pTimeline->Update( Timestep );
			}
		}
	}

	TLRender::TRenderNode::ProcessMessage(Message);
}

//---------------------------------------------------------
//	create/change the timeline
//---------------------------------------------------------
void TLRender::TRenderNodeTimeline::SetTimeline(TRefRef NewTimeline)
{
	//	no change
	if ( NewTimeline == m_TimelineRef )
		return;

	//	destroy old timeline
	m_pTimeline = NULL;

	//	set new timeline ref
	m_TimelineRef = NewTimeline;

	//	create new instance
	m_pTimeline = new TLAnimation::TTimelineInstance(m_TimelineRef);
	m_pTimeline->Initialise();
	m_pTimeline->BindTo( this->GetNodeRef() );

	this->SubscribeTo( m_pTimeline );
}

