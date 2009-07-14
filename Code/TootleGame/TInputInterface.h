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
#include <TootleMaths/TLine.h>


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
		TClick(const int2& CursorPos,float ActionValue, TRefRef ActionRef) :
			m_CursorPos		( CursorPos ),
			m_ActionValue	( ActionValue ),
			m_ActionRef		( ActionRef ),
			m_WorldRayValid	( FALSE )
		{
		}	
		TClick() :
			m_CursorPos		( -1,-1 ),
			m_ActionValue	( 0.f ),
			m_WorldRayValid	( FALSE )
		{
		}

		FORCEINLINE const int2&				GetCursorPos() const							{	return m_CursorPos;	}
		FORCEINLINE const float&			GetActionValue() const							{	return m_ActionValue;	}
		FORCEINLINE void					SetWorldRay(const TLMaths::TLine& WorldRay)		{	m_WorldRayValid = TRUE;	m_WorldRay = WorldRay;	}
		FORCEINLINE const TLMaths::TLine&	GetWorldRay() const								{	return m_WorldRay;	}
		FORCEINLINE float3					GetWorldPos(float z) const;
		FORCEINLINE TRefRef					GetActionRef() const							{	return m_ActionRef; }

	protected:
		int2			m_CursorPos;	
		float			m_ActionValue;
		TRef			m_ActionRef;
		TLMaths::TLine	m_WorldRay;
		Bool			m_WorldRayValid;
	};

public:
	TInputInterface(TRefRef RenderTargetRef,TRefRef RenderNodeRef,TRefRef UserRef,TRefRef ActionOutDown,TRefRef ActionOutUp=TRef(),TBinaryTree* pWidgetData=NULL);
	~TInputInterface();
	
	SyncBool					Initialise();						//	continue initialising
	void						Shutdown();							//	shutdown code - just unsubscribes from publishers - this is to release all the TPtr's so we can be destructed
	virtual TRefRef				GetSubscriberRef() const		{	static TRef Ref("inpint");	return Ref;	}

	TBinaryTree&				GetWidgetData()					{	return m_WidgetData;	}
	const TBinaryTree&			GetWidgetData() const			{	return m_WidgetData;	}

protected:
	virtual Bool				Update();											//	update routine - return FALSE if we don't need updates any more
	virtual void				ProcessMessage(TLMessaging::TMessage& Message);	//	
	virtual SyncBool			ProcessClick(TClick& Click,TLRender::TScreen& Screen,TLRender::TRenderTarget& RenderTarget,TLRender::TRenderNode& RenderNode);	//	process a click and detect clicks on/off our render node. return SyncWait if we didnt process it and want to process again
	void						SendActionMessage(Bool ActionDown,float RawData);	//	when click has been validated action message is sent to subscribers
	virtual void				GetRenderNodes(TArray<TRef>& RenderNodeArray);		//	get array of all the render nodes we're using
	
	SyncBool					IsIntersecting(TLRender::TScreen& Screen, TLRender::TRenderTarget& RenderTarget, TLRender::TRenderNode& RenderNode,TClick& Click);

	virtual void				OnInitialised()										{	}

	virtual void				OnClickBegin(const TClick& Click);
	virtual void				OnClickEnd(const TClick& Click);

	virtual void				OnCursorMove(const int2& NewCursorPosition, TRefRef ActionRef)			{}
	
	virtual void				OnCursorHoverOn()	{}
	virtual void				OnCursorHoverOff()	{}
	
	void						QueueClick(const int2& CursorPos,float ActionValue, TRefRef ActionRef);	//	put a click in the queue

	FORCEINLINE void			AppendWidgetData(TLMessaging::TMessage& Message)			{	Message.ReferenceDataTree( m_WidgetData, FALSE );	}

private:
	void						ProcessQueuedClicks();	//	go through queued-up (unhandled) clicks and respond to them

	void						RemoveAllActions();
protected:
	TRef						m_RenderTargetRef;
	TRef						m_RenderNodeRef;

	TRef						m_ActionOutDown;		//	action to send out when mouse goes down over render node
	TRef						m_ActionOutUp;			//	action to send out when mouse is relesed/not over render node

	TRef						m_ActionBeingProcessedRef;	// Action in process

	TBinaryTree					m_WidgetData;			//	generic widget data - this gets attached to all messages sent out so you cna attach node refs etc

private:
	SyncBool					m_Initialised;			//	have created actions and subscribed to user input
	TRef						m_UserRef;
	TRef						m_ActionInClick;		//	our click action
	TRef						m_ActionInMove;			//	our drag action (only occurs when mouse is down)
	TArray<TClick>				m_QueuedClicks;			//	action's we had to wait for
};





FORCEINLINE float3 TLInput::TInputInterface::TClick::GetWorldPos(float z) const						
{	
	float Factor = m_WorldRay.GetFactorAlongZ(z);	
	float3 Pos;	
	m_WorldRay.GetPointAlongLine( Pos, Factor );	
	return Pos;	
}

