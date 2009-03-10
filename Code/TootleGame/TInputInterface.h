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


namespace TLInput
{
	class TInputInterface;
};

namespace TLRender
{
	class TScreen;
	class TRenderTarget;
	class TRenderNode;
};


//----------------------------------------------
//	gr: currently this is a click->screen->action relay kinda system
//	consider turning the GUI (this) into another input device that 
//	piggy backs on the mouse input?
//----------------------------------------------
class TLInput::TInputInterface : public TLMessaging::TPublisher, public TLMessaging::TSubscriber
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
	TInputInterface(TRefRef RenderTargetRef,TRefRef RenderNodeRef,TRefRef UserRef,TRefRef ActionOutDown,TRefRef ActionOutUp=TRef());
	~TInputInterface();
	
	SyncBool					Initialise();						//	continue initialising
	void						Shutdown();							//	shutdown code - just unsubscribes from publishers - this is to release all the TPtr's so we can be destructed

protected:
	virtual Bool				Update();											//	update routine - return FALSE if we don't need updates any more
	virtual void				ProcessMessage(TLMessaging::TMessage& Message);	//	
	virtual SyncBool			ProcessClick(const TClick& Click,TLRender::TScreen& Screen,TLRender::TRenderTarget& RenderTarget,TPtr<TLRender::TRenderNode>& pRenderNode);	//	process a click and detect clicks on/off our render node. return SyncWait if we didnt process it and want to process again
	void						SendActionMessage(Bool ActionDown,float RawData);	//	when click has been validated action message is sent to subscribers
	virtual void				GetRenderNodes(TArray<TRef>& RenderNodeArray);		//	get array of all the render nodes we're using
	
	virtual void				OnInitialised()										{	};

	virtual void				OnClickBegin();
	virtual void				OnClickEnd();

	virtual void				OnCursorMove();		
	virtual void				OnCursorHoverOn()	{}
	virtual void				OnCursorHoverOff()	{}

private:
	void						QueueClick(const int2& CursorPos,float ActionValue);	//	put a click in the queue
	void						ProcessQueuedClicks();	//	go through queued-up (unhandled) clicks and respond to them

protected:
	TRef						m_RenderTargetRef;
	TRef						m_RenderNodeRef;

	TRef						m_ActionOutDown;		//	action to send out when mouse goes down over render node
	TRef						m_ActionOutUp;			//	action to send out when mouse is relesed/not over render node

private:
	Bool						m_Subscribed;				//	have created actions and subscribed to user input
	Bool						m_InitialisedRenderNodes;	//	have setup render nodes to be clickable
	TRef						m_UserRef;
	TRef						m_ActionIn;
	TArray<TClick>				m_QueuedClicks;			//	action's we had to wait for
};

