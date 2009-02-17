/*------------------------------------------------------

	GUI class. Creates a simple link between screen render
	objects, input and outputs action messages.
	Designed to integrate nicely with menu's or as an input
	device replacement.

	Formely known as "menu renderer"

-------------------------------------------------------*/
#pragma once

#include <TootleCore/TLTypes.h>
#include <TootleCore/TPublisher.h>
#include <TootleCore/TSubscriber.h>
#include <TootleCore/TClassFactory.h>


namespace TLGui
{
	class TGui;
};



//----------------------------------------------
//	gr: currently this is a click->screen->action relay kinda system
//	consider turning the GUI (this) into another input device that 
//	piggy backs on the mouse input?
//----------------------------------------------
class TLGui::TGui : public TLMessaging::TPublisher, public TLMessaging::TSubscriber
{
protected:
	class TClick
	{
	public:
		TClick(const int2& CursorPos,float ActionValue) :
			m_CursorPos		( CursorPos ),
			m_ActionValue	( ActionValue )
		{
		}	
		TClick() :
			m_CursorPos		( 0,0 ),
			m_ActionValue	( 0.f )
		{
		}

	public:
		int2	m_CursorPos;	
		float	m_ActionValue;
	};

public:
	TGui(TRefRef RenderTargetRef,TRefRef RenderNodeRef,TRefRef UserRef,TRefRef ActionOutDown,TRefRef ActionOutUp=TRef());
	~TGui();
	
	SyncBool					Initialise();						//	continue initialising
	void						Shutdown();							//	shutdown code - just unsubscribes from publishers - this is to release all the TPtr's so we can be destructed

protected:
	void						QueueClick(const int2& CursorPos,float ActionValue)		{	m_QueuedClicks.Add( TClick( CursorPos, ActionValue ) );	}
	void						ProcessQueuedClicks();	//	go through queued-up (unhandled) clicks and respond to them
	virtual void				ProcessMessage(TPtr<TLMessaging::TMessage>& pMessage);	//	
	void						SendActionMessage(Bool ActionDown);	//	when click has been validated action message is sent to subscribers

protected:
	Bool						m_Subscribed;				//	have created actions and subscribed to user input
	Bool						m_InitialisedRenderNodes;	//	have setup render nodes to be clickable
	TRef						m_RenderTargetRef;
	TRef						m_RenderNodeRef;
	TRef						m_UserRef;
	TRef						m_ActionIn;
	Bool						m_ActionIsDown;			//	we store when we did get an action(which clicked), so if we keep holding, and go out of the zone, we can send an up message
	TRef						m_ActionOutDown;		//	action to send out when mouse goes down over render node
	TRef						m_ActionOutUp;			//	action to send out when mouse is relesed/not over render node
	TArray<TClick>				m_QueuedClicks;			//	action's we had to wait for
};


