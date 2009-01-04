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
	class TGuiFactory;

	void		Init();
	void		Shutdown();

	extern TPtr<TGuiFactory>	g_pFactory;	//	gr: i'll probably move the TGui's into something else (like the TRenderGraph or the User) when I've determined the bottle neck (if any)
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
	TGui(TRefRef GuiRef);
	
	SyncBool					Initialise(TRefRef RenderTargetRef,TRefRef RenderNodeRef,TRefRef UserRef,TRefRef ActionOutDown,TRefRef ActionOutUp=TRef());	//	keeps subscribing to the appropriate channels until everything is done
	SyncBool					Initialise();						//	continue initialising
	FORCEINLINE TRefRef			GetGuiRef() const					{	return m_GuiRef;	}

	FORCEINLINE Bool			operator==(TRefRef GuiRef) const	{	return GetGuiRef() == GuiRef;	}

protected:
	void						QueueClick(const int2& CursorPos,float ActionValue)		{	m_QueuedClicks.Add( TClick( CursorPos, ActionValue ) );	}
	void						ProcessQueuedClicks();	//	go through queued-up (unhandled) clicks and respond to them
	virtual void				ProcessMessage(TPtr<TLMessaging::TMessage>& pMessage);	//	
	void						SendActionMessage(Bool ActionDown);	//	when click has been validated action message is sent to subscribers

protected:
	Bool						m_Subscribed;				//	have created actions and subscribed to user input
	Bool						m_InitialisedRenderNodes;	//	have setup render nodes to be clickable
	TRef						m_GuiRef;
	TRef						m_RenderTargetRef;
	TRef						m_RenderNodeRef;
	TRef						m_UserRef;
	TRef						m_ActionIn;
	Bool						m_ActionIsDown;			//	we store when we did get an action(which clicked), so if we keep holding, and go out of the zone, we can send an up message
	TRef						m_ActionOutDown;
	TRef						m_ActionOutUp;
	TArray<TClick>				m_QueuedClicks;			//	action's we had to wait for
};



class TLGui::TGuiFactory : public TClassFactory<TLGui::TGui,TRUE>
{
public:

protected:
	virtual TLGui::TGui*	CreateObject(TRefRef InstanceRef,TRefRef TypeRef)	{	return new TGui(InstanceRef);	}
};

