/*------------------------------------------------------

	Base Widget class. Creates a simple link between screen render
	objects, input and outputs action messages.
	Designed to integrate nicely with menu's or as an input
	device replacement. (eg. on-screen thumbstick)

-------------------------------------------------------*/
#pragma once

#include <TootleCore/TLTypes.h>
#include <TootleCore/TPublisher.h>
#include <TootleCore/TSubscriber.h>
#include <TootleCore/TClassFactory.h>
#include <TootleMaths/TLine.h>
#include <TootleMaths/TShape.h>
#include <TootleMaths/TSphere.h>

#include <TootleRender/TRenderNode.h>


namespace TLGui
{
	class TWidget;

#define TLGui_WidgetActionType_Down		TRef_Static4(D,o,w,n)
#define TLGui_WidgetActionType_Move		TRef_Static4(M,o,v,e)
#define TLGui_WidgetActionType_Up		TRef_Static2(U,p)
	
	class TWidgetFactory;
	class TWidgetManager;
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
class TLGui::TWidget : public TLMessaging::TPublisher, public TLMessaging::TSubscriber
{
	friend class TLGui::TWidgetFactory;
	friend class TLGui::TWidgetManager;
	
public:
	class TClick
	{
	public:
		TClick(const int2& CursorPos,float ActionValue, TRefRef ActionRef, TRefRef ActionType) :
			m_CursorPos		( CursorPos ),
			m_ActionValue	( ActionValue ),
			m_ActionRef		( ActionRef ),
			m_ActionType	( ActionType ),
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
		FORCEINLINE Bool					IsWorldRayValid() const							{	return m_WorldRayValid;	}
		FORCEINLINE const TLMaths::TLine&	GetWorldRay() const								{	return m_WorldRay;	}
		FORCEINLINE float3					GetWorldPos(float z) const;
		FORCEINLINE TRefRef					GetActionRef() const							{	return m_ActionRef; }
		FORCEINLINE TRefRef					GetActionType() const							{	return m_ActionType; }

	protected:
		int2			m_CursorPos;		//	in screen space
		float			m_ActionValue;
		TRef			m_ActionRef;
		TRef			m_ActionType;		//	TLGui_WidgetActionType_*
		TLMaths::TLine	m_WorldRay;
		Bool			m_WorldRayValid;
	};

public:
	
	TWidget(TRefRef RenderTargetRef,TRefRef RenderNodeRef,TRefRef UserRef,TRefRef ActionOutDown,TRefRef ActionOutUp=TRef(),TBinaryTree* pWidgetData=NULL, TRefRef DatumRef=TLRender_TRenderNode_DatumBoundsBox2D);
	TWidget(TRefRef RenderTargetRef,TBinaryTree& WidgetData);
	virtual ~TWidget();

	void						Shutdown();							//	shutdown code - just unsubscribes from publishers - this is to release all the TPtr's so we can be destructed
	
	virtual TRefRef				GetSubscriberRef() const		{	static TRef Ref("inpint");	return Ref;	}

	TBinaryTree&				GetWidgetData()					{	return m_WidgetData;	}
	const TBinaryTree&			GetWidgetData() const			{	return m_WidgetData;	}

	FORCEINLINE void			SetEnabled(Bool Enabled);		//	enable/disable widget
	FORCEINLINE Bool			IsEnabled() const				{	return m_Enabled;	}

	SyncBool					IsIntersecting(TClick& Click);	//	do a one-off intersection test with this click information. This shouldnt be used in critical code or for normal processing, ProcessQueuedClicks pre-caches much more info

	FORCEINLINE TRefRef			GetRenderNodeRef() const		{	return m_RenderNodeRef;	}	//	can be invalid, or used in more than one widget, so dont rely on this to be unique!

	TRef						GetWidgetRef()	const			{ return m_WidgetRef; }
	TRef						GetTypeRef()	const			{ return m_TypeRef; }
	
	FORCEINLINE Bool			operator==(TRefRef WidgetRef) const			{	return m_WidgetRef == WidgetRef;	}
	
protected:
	TWidget(TRefRef InstanceRef, TRefRef TypeRef);

	
	virtual void				Initialise(TLMessaging::TMessage& Message);
	virtual void				SetProperty(TLMessaging::TMessage& Message);
	
	virtual Bool				Update();											//	update routine - return FALSE if we don't need updates any more
	virtual void				ProcessMessage(TLMessaging::TMessage& Message);	//	
	virtual SyncBool			ProcessClick(TClick& Click,TLRender::TScreen& Screen,TLRender::TRenderTarget& RenderTarget,TLRender::TRenderNode& RenderNode,const TLMaths::TShapeSphere2D& BoundsDatum,const TLMaths::TShape* pClickDatum);	//	process a click and detect clicks on/off our render node. return SyncWait if we didnt process it and want to process again
	void						SendActionMessage(const TClick& Click,TRefRef ActionRef,TBinaryTree* pExtraData=NULL);	//	when click has been validated action message is sent to subscribers
	virtual void				GetRenderNodes(TArray<TRef>& RenderNodeArray);		//	get array of all the render nodes we're using
	
	SyncBool					IsIntersecting(TLRender::TScreen& Screen, TLRender::TRenderTarget& RenderTarget, TLRender::TRenderNode& RenderNode,const TLMaths::TShapeSphere2D& BoundsDatum,const TLMaths::TShape* pClickDatum,TClick& Click);

	virtual void				OnClickBegin(const TClick& Click);
	virtual void				OnClickEnd(const TClick& Click);

	virtual void				OnCursorMove(const int2& NewCursorPosition, TRefRef ActionRef)			{}
	
	virtual void				OnCursorHoverOn()	{}
	virtual void				OnCursorHoverOff()	{}
	
	void						QueueClick(const int2& CursorPos,float ActionValue, TRefRef ActionRef, TRefRef ActionType);	//	put a click in the queue

	FORCEINLINE void			AppendWidgetData(TLMessaging::TMessage& Message)			{	Message.ReferenceDataTree( m_WidgetData );	}

	virtual void				OnEnabled();			//	widget was enabled
	virtual void				OnDisabled();			//	widget disabled

private:
	SyncBool					ProcessQueuedClicks();	//	go through queued-up (unhandled) clicks and respond to them. Return FALSE if we cannot process and want to ditch all collected clicks. SyncWait if we don't process the clicks but want to keep them

	SyncBool					PrefetchProcessData(TLRender::TScreen*& pScreen,TLRender::TRenderTarget*& pRenderTarget,TLRender::TRenderNode*& pRenderNode,const TLMaths::TShapeSphere2D*& pWorldBoundsSphere,TPtr<TLMaths::TShape>& pClickDatum);

protected:
	TRef						m_WidgetRef;			// Widget ref
	TRef						m_TypeRef;				// Type ref
			
	
	TRef						m_RenderTargetRef;		//	render target where we can see the render node to click on
	TRef						m_RenderNodeRef;		//	render node we're clicking on
	TRef						m_RenderNodeDatum;		//	after a fast sphere check we check for a click inside this datum shape (if none supplied at construction we use the bounds box)
	Bool						m_RenderNodeDatumKeepShape;	//	sometimes for efficiency we can keep the datum shape as the same type (ie. keep a rotated box as a box shape)

	TRef						m_ActionOutDown;		//	action to send out when mouse goes down over render node
	TRef						m_ActionOutUp;			//	action to send out when mouse is relesed/not over render node

	TRef						m_ActionBeingProcessedRef;	// Action in process

	TBinaryTree					m_WidgetData;			//	generic widget data - this gets attached to all messages sent out so you cna attach node refs etc

private:
	TRef						m_UserRef;
	TArray<TClick>				m_QueuedClicks;			//	action's we had to wait for
	Bool						m_Enabled;				//	widget is/isn't enabled
};





FORCEINLINE float3 TLGui::TWidget::TClick::GetWorldPos(float z) const						
{	
	float Factor = m_WorldRay.GetFactorAlongZ(z);	
	float3 Pos;	
	m_WorldRay.GetPointAlongLine( Pos, Factor );	
	return Pos;	
}
	

//-------------------------------------------------
//	enable/disable widget
//-------------------------------------------------
FORCEINLINE void TLGui::TWidget::SetEnabled(Bool Enabled)		
{
	if ( Enabled == IsEnabled() )	
		return;	
	
	m_Enabled = Enabled;	
	
	if ( IsEnabled() )	
		OnEnabled();	
	else	
		OnDisabled();
}


