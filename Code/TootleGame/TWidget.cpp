#include "TWidget.h"
#include <TootleInput/TUser.h>
#include <TootleInput/TLInput.h>
#include <TootleInput/TDevice.h>
#include <TootleRender/TRenderNode.h>
#include <TootleRender/TRenderTarget.h>
#include <TootleRender/TScreen.h>
#include <TootleRender/TLRender.h>
#include <TootleRender/TScreenManager.h>
#include <TootleRender/TRendergraph.h>
#include <TootleCore/TCoreManager.h>
#include "TWidgetManager.h"



TLGui::TWidget::TWidget(TRefRef RenderTargetRef,TRefRef RenderNodeRef, TRefRef UserRef,TRefRef ActionOutDown,TRefRef ActionOutUp,TBinaryTree* pWidgetData, TRefRef DatumRef)  : 
	m_RenderTargetRef			( RenderTargetRef ),
	m_RenderNodeRef				( RenderNodeRef ),
	m_RenderNodeDatum			( DatumRef ),
	m_RenderNodeDatumKeepShape	( TRUE ),
	m_UserRef					( UserRef),
	m_ActionOutDown				( ActionOutDown ),
	m_ActionOutUp				( ActionOutUp ),
	m_WidgetData				("WidgetData"),
	m_Enabled					( TRUE )
{
	//	no actions going out means this TWidget wont do anything
	if ( !m_ActionOutDown.IsValid() && !m_ActionOutUp.IsValid() )
	{
		TLDebug_Break("TWidget created that won't send out actions");
	}

	//	copy user-supplied data
	if ( pWidgetData )
		m_WidgetData.ReferenceDataTree( *pWidgetData );

	//	do OnEnabled functionality like an initialise
	OnEnabled();
}


	
TLGui::TWidget::TWidget(TRefRef RenderTargetRef,TBinaryTree& WidgetData)  : 
	m_RenderTargetRef			( RenderTargetRef ),
	m_RenderNodeDatumKeepShape	( TRUE ),
	m_UserRef					( "global" ),
	m_WidgetData				("WidgetData"),
	m_Enabled					( TRUE )
{
	//	read actions out of the TBinary
	WidgetData.ImportData("ActDown", m_ActionOutDown );
	WidgetData.ImportData("ActUp", m_ActionOutUp );

	//	read user ref
	WidgetData.ImportData("User", m_UserRef );

	//	read render node
	WidgetData.ImportData("Node", m_RenderNodeRef );

	//	read out a datum - if none supplied we use the bounds box
	if ( !WidgetData.ImportData("Datum", m_RenderNodeDatum ) )
		m_RenderNodeDatum = TLRender_TRenderNode_DatumBoundsBox2D;

	WidgetData.ImportData("DtKeepShape", m_RenderNodeDatumKeepShape );	

	//	copy user-supplied data
	//	m_WidgetData.ReferenceDataTree( *pData, FALSE );
	m_WidgetData.AddUnreadChildren( WidgetData, FALSE );

	//	do OnEnabled functionality like an initialise
	OnEnabled();
}


TLGui::TWidget::~TWidget()
{
	Shutdown();
}



//-------------------------------------------------
//	get array of all the render nodes we're using
//-------------------------------------------------
void TLGui::TWidget::GetRenderNodes(TArray<TRef>& RenderNodeArray)
{
	RenderNodeArray.Add( m_RenderNodeRef );
}


//-------------------------------------------------
//	shutdown code - just unsubscribes from publishers - this is to release all the TPtr's so we can be destructed
//-------------------------------------------------
void TLGui::TWidget::Shutdown()
{
	TLMessaging::TPublisher::Shutdown();
	TLMessaging::TSubscriber::Shutdown();
}


//-------------------------------------------------
//	
//-------------------------------------------------
void TLGui::TWidget::ProcessMessage(TLMessaging::TMessage& Message)
{
	if(Message.GetMessageRef() == TRef_Static(A,c,t,i,o))
	{
		if ( !HasSubscribers() )
		{
#ifdef _DEBUG
			TTempString Debug_String("TWidget ");
			m_RenderNodeRef.GetString( Debug_String );
			Debug_String.Append(" has no subscribers");
			TLDebug_Warning( Debug_String );
#endif
		}
		else 
		{
			TRef ActionRef;
			int2 CursorPosition;

			if ( Message.Read(ActionRef) && Message.ImportData("CURSOR", CursorPosition ) )
			{

				if(TLGui::g_pWidgetManager->IsClickActionRef(ActionRef))
				{
					//	read the raw value to see if it's a mouse down or mouse up
					float RawValue = 0.f;
					if ( Message.ImportData("RawData", RawValue) )
					{
						//	queue up this click
						QueueClick( CursorPosition, RawValue, ActionRef, RawValue < TLMaths_NearZero ? TLGui_WidgetActionType_Up : TLGui_WidgetActionType_Down );
					}
				}
				else if(TLGui::g_pWidgetManager->IsMoveActionRef(ActionRef))
				{
					OnCursorMove(CursorPosition, ActionRef);
				}
				
				//	now process ALL the queued clicks so if we have some unprocessed they're not lost and kept in order
				if ( ProcessQueuedClicks() == SyncFalse )
					m_QueuedClicks.Empty();
			}
		}
	}
	else if ( Message.GetMessageRef() == TLCore::UpdateRef )
	{
		if ( !Update() )
		{
			TLCore::g_pCoreManager->Unsubscribe(this);
		}
	}
}



//-------------------------------------------------
//	put a click in the queue
//-------------------------------------------------
void TLGui::TWidget::QueueClick(const int2& CursorPos,float ActionValue, TRefRef ActionRef,TRefRef ActionType)		
{
	//	if we have no subscribers we can just ditch the clicks as we're not going to do anything
	if ( !HasSubscribers() || !IsEnabled() )
		return;

	//	if this "click" is a mouse up, and the previous was too, then dont add it
	if ( m_QueuedClicks.GetSize() > 0 )
	{
		//	both this and prev action values were "off" so skip adding to the queue
		if ( ActionType == TLGui_WidgetActionType_Up && m_QueuedClicks.ElementLast().GetActionType() == TLGui_WidgetActionType_Up )
			return;
	}

	TLRender::TRenderTarget* pRenderTarget = TLRender::g_pScreenManager->GetRenderTarget( m_RenderTargetRef );
	//	if no render target at all then assume error - ignore click
	if ( !pRenderTarget )
	{
		TLDebug_Break("Render target expected");
		return;
	}

	//	gr: when the render target (maybe expand to include rendernode) is disabled
	//		we need to ignore clicks and mouse moves so they dont continue to queue up 
	//		and then all get processed if the widget comes back
	//		BUT we still want to process mouse releases to "finish" a current click
	if ( !pRenderTarget->IsEnabled() )
	{
		switch ( ActionType.GetData() )
		{
		case TLGui_WidgetActionType_Move:
		case TLGui_WidgetActionType_Down:
			return;

		//	if it's an "up" then let it go through if it's the current action
		case TLGui_WidgetActionType_Up:
			if ( ActionRef != m_ActionBeingProcessedRef )
				return;
			break;

		default:
			TLDebug_Break("Unknown action type");
			return;
		}
	}

	//	add to queue
	m_QueuedClicks.Add( TClick( CursorPos, ActionValue, ActionRef, ActionType ) );
}



//-------------------------------------------------
//	update routine - return FALSE if we don't need updates any more
//-------------------------------------------------
Bool TLGui::TWidget::Update()
{
	Bool ContinueUpdate = FALSE;

	//	process any queued up clicks
	if ( ProcessQueuedClicks() == SyncFalse )
		m_QueuedClicks.Empty();

	//	some clicks still need processing
	if ( m_QueuedClicks.GetSize() > 0 )
		ContinueUpdate |= TRUE;

	//	initialise finished, no need to update any more
	return ContinueUpdate;
}


//-------------------------------------------------
//	
//-------------------------------------------------
SyncBool TLGui::TWidget::PrefetchProcessData(TLRender::TScreen*& pScreen,TLRender::TRenderTarget*& pRenderTarget,TLRender::TRenderNode*& pRenderNode,const TLMaths::TShapeSphere2D*& pWorldBoundsSphere,TPtr<TLMaths::TShape>& pClickDatum)
{
	//	find the render target in a screen...
	TPtr<TLRender::TScreen> pScreenPtr;
	pRenderTarget = TLRender::g_pScreenManager->GetRenderTarget( m_RenderTargetRef, pScreenPtr );
	pScreen = pScreenPtr;

	//	didnt find the render target
	if ( !pRenderTarget )
		return SyncFalse;

	//	render target isnt enabled, ignore clicks
	if ( !pRenderTarget->IsEnabled() )
		return SyncWait;

	//	gr: not sure we can have an invalid rendernode and not crash below... check if anyhting is using the system like this
	if ( !m_RenderNodeRef.IsValid() )
	{
		TLDebug_Break("Is there any code using this functionality - widget with no render node ref");
		return SyncFalse;
	}

	//	got a render target, fetch a render node
	pRenderNode = m_RenderNodeRef.IsValid() ? TLRender::g_pRendergraph->FindNode( m_RenderNodeRef ) : TLPtr::GetNullPtr<TLRender::TRenderNode>();

	//	gr: for support of those that don't use this base render node variable, only check if the ref is valid
	if ( m_RenderNodeRef.IsValid() && !pRenderNode )
		return SyncFalse;

	//	ditch clicks if disabled
	if ( !pRenderNode->IsEnabled() )
		return SyncFalse;

	//	pre-fetch bounds

	//	get world bounds sphere
	pWorldBoundsSphere = &pRenderNode->GetWorldBoundsSphere2D();

	//	if this fails, we can assume the transform on the render node is out of date
	if ( !pWorldBoundsSphere->IsValid() )
		return SyncWait;

	//	fetch the clickable datum if one is specified - if this fails then we abort (probably missing datum)
	//	if none was specified we use the bounds sphere we've already fetched and assume the widget has some custom
	//	code that doesn't use the datum anyway
	if ( m_RenderNodeDatum.IsValid() )
	{
		pClickDatum = pRenderNode->GetWorldDatum( m_RenderNodeDatum, m_RenderNodeDatumKeepShape );
		if ( !pClickDatum )
		{
			TTempString Debug_String("Missing click-datum ");
			m_RenderNodeDatum.GetString( Debug_String );
			Debug_String.Append(" for widget on render node ");
			pRenderNode->GetNodeRef().GetString( Debug_String );
			Debug_String.Append(" (mesh is ");
			pRenderNode->GetMeshRef().GetString( Debug_String );
			Debug_String.Append(")");
			TLDebug_Print( Debug_String );
			return SyncWait;
		}
	}

	//	got all the values we want, continue onto process/intersection tests
	return SyncTrue;
}

//-------------------------------------------------
//	do a one-off intersection test with this click information. This shouldnt be used in critical code or for normal processing, ProcessQueuedClicks pre-caches much more info
//-------------------------------------------------
SyncBool TLGui::TWidget::IsIntersecting(TClick& Click)
{
	//	gr: don't allow intersection if disabled?
	if ( !IsEnabled() )
		return SyncFalse;

	//	get all the variables we need
	TLRender::TScreen* pScreen = NULL;
	TLRender::TRenderTarget* pRenderTarget = NULL;
	TLRender::TRenderNode* pRenderNode = NULL;
	const TLMaths::TShapeSphere2D* pWorldBoundsSphere = NULL;
	TPtr<TLMaths::TShape> pClickDatum;

	SyncBool PrefetchResult = PrefetchProcessData( pScreen, pRenderTarget, pRenderNode, pWorldBoundsSphere, pClickDatum );

	//	if it failed then return whatever the fetch said
	if ( PrefetchResult != SyncTrue )
		return PrefetchResult;

	//	do intersection test
	SyncBool IntersectionResult = IsIntersecting( *pScreen, *pRenderTarget, *pRenderNode, *pWorldBoundsSphere, pClickDatum, Click );

	return IntersectionResult;
}

//-------------------------------------------------
//	go through queued-up (unhandled) clicks and respond to them. 
//	Return FALSE if we cannot process and want to ditch all collected clicks. SyncWait if we don't process the clicks but want to keep them
//-------------------------------------------------
SyncBool TLGui::TWidget::ProcessQueuedClicks()
{
	//	no queue
	if ( m_QueuedClicks.GetSize() == 0 )
		return SyncTrue;

	//	not enabled - ditch clicks
	//	gr: allow process if they're here, just ignore them in QueueClick
//	if ( !IsEnabled() )
//		return SyncFalse;

	//	if we have no subscribers we can just ditch the clicks...
	if ( !HasSubscribers() )
		return SyncFalse;

	//	fetch all our neccessary variables
	TLRender::TScreen* pScreen = NULL;
	TLRender::TRenderTarget* pRenderTarget = NULL;
	TLRender::TRenderNode* pRenderNode = NULL;
	const TLMaths::TShapeSphere2D* pWorldBoundsSphere = NULL;
	TPtr<TLMaths::TShape> pClickDatum;

	SyncBool PrefetchResult = PrefetchProcessData( pScreen, pRenderTarget, pRenderNode, pWorldBoundsSphere, pClickDatum );

	//	if it failed then return whatever the fetch said
	if ( PrefetchResult != SyncTrue )
		return PrefetchResult;

	//	procecess the clicks in the order they came in - removing as we go
	while ( m_QueuedClicks.GetSize() )
	{
		TClick& Click = m_QueuedClicks[0];

		if ( ProcessClick( Click, *pScreen, *pRenderTarget, *pRenderNode, *pWorldBoundsSphere, pClickDatum ) == SyncWait )
			break;

		m_QueuedClicks.RemoveAt( 0 );
	}

	//	need to process this click again later so subscribe to updates to process them next time we can
	if ( m_QueuedClicks.GetSize() > 0 )
		this->SubscribeTo( TLCore::g_pCoreManager );

	return SyncTrue;
}


//-------------------------------------------------
//	process a click and detect clicks on/off our render node. return SyncWait if we didnt process it and want to process again
//-------------------------------------------------
SyncBool TLGui::TWidget::ProcessClick(TClick& Click,TLRender::TScreen& Screen,TLRender::TRenderTarget& RenderTarget,TLRender::TRenderNode& RenderNode,const TLMaths::TShapeSphere2D& BoundsDatum,const TLMaths::TShape* pClickDatum)
{
	Bool bMoveAction = Click.GetActionRef() == TLGui_WidgetActionType_Move;
	Bool bClickAction = !bMoveAction;
	Bool bIsCurrentAction = FALSE;
	Bool bIsActionPair = FALSE;
	
	if(m_ActionBeingProcessedRef.IsValid())
	{
		bIsCurrentAction = (Click.GetActionRef() == m_ActionBeingProcessedRef);
		bIsActionPair = TLGui::g_pWidgetManager->IsActionPair(m_ActionBeingProcessedRef, Click.GetActionRef());
	}

	
	Bool bTestIntersection = FALSE;
	
	if(m_ActionBeingProcessedRef.IsValid() )
	{
		if(bClickAction && bIsCurrentAction)
		{
			// Click occured and it's the same as the currently processing action
			bTestIntersection = TRUE;
		}
		else if(bMoveAction && bIsActionPair )
		{
			// move action accured and is paired with our action being processed
			bTestIntersection = TRUE;
		}
		
	}
	else if(bClickAction)
	{
		// No 'click' action in progress yet.  Test intersection
		bTestIntersection = TRUE;
	}
	
	// If we aren't going to test the intersection then we don't need to continue processing this click
	// effectively we have some click actions which we aren't interested in.
	if(!bTestIntersection)
		return SyncFalse;

	//	see if ray intersects our object - check all our collision objects to get closest hit.
	SyncBool Intersection = IsIntersecting(Screen, RenderTarget, RenderNode, BoundsDatum, pClickDatum, Click );

	switch ( Intersection )
	{
		//	failed to check - no valid bounds? might have to wait till next frame
		case SyncWait:
		{
			return SyncWait;
		}
		break;
	
		//	click position intersected with shape - test to see if the click should end
		case SyncTrue:
		{
			if(Click.GetActionType() == TLGui_WidgetActionType_Up)
			{
				OnClickEnd( Click );

				// If the click was an actual click, not a move, then set the action being processed ref
				if(bClickAction)
					m_ActionBeingProcessedRef.SetInvalid();
			}
			else
			{
				if ( Click.GetActionType() == TLGui_WidgetActionType_Move )
				{
					TLDebug_Break("gr: should we be calling OnClickBegin during a move?");
				}

				// This is where we *should* check for being able to hover 'onto' a button
				// but the movement is classed as clicks along with the actual clicks.... that's gonna have to change! :/
				//if(AllowClickOnHoverOver())
				OnClickBegin( Click );

				if(bClickAction)
					m_ActionBeingProcessedRef = Click.GetActionRef();
			}
		}
		break;

		// Click pos does not intersect shape
		case SyncFalse:
		{
			//if((bClickAction  && bCurrentAction) || (bMoveAction && bActionPair))
			if(m_ActionBeingProcessedRef.IsValid())
			{
				OnClickEnd( Click );
				
				if(bClickAction)
					m_ActionBeingProcessedRef.SetInvalid();
			}
		}
		break;
	}

	return SyncTrue;
}


SyncBool TLGui::TWidget::IsIntersecting(TLRender::TScreen& Screen, TLRender::TRenderTarget& RenderTarget, TLRender::TRenderNode& RenderNode,const TLMaths::TShapeSphere2D& BoundsDatum,const TLMaths::TShape* pClickDatum,TClick& Click)
{
	SyncBool Intersection = SyncWait;

	//	test for click on the collision shapes on the render node
	TLMaths::TLine WorldRay;

	if ( !Screen.GetWorldRayFromScreenPos( RenderTarget, WorldRay, Click.GetCursorPos() ) )
	{
		//	click was out of the render target so we couldnt get a ray
		Intersection = SyncFalse;
	}
	else
	{
		//	store the ray in the click
		Click.SetWorldRay( WorldRay );

		//	check for a click in the fastest order...

		//	if we haven't already failed a check, test again bounds sphere
		if ( Intersection != SyncFalse )
		{
			Intersection = BoundsDatum.GetSphere().GetIntersection( WorldRay ) ? SyncTrue : SyncFalse;
		}

		//	if we haven't already failed a check, test again bounds box for a tighter bounds check
		if ( pClickDatum && Intersection != SyncFalse )
		{
			Intersection = pClickDatum->HasIntersection( WorldRay ) ? SyncTrue : SyncFalse;
		}
	}

	return Intersection;
}



void TLGui::TWidget::OnClickBegin(const TClick& Click)
{
	SendActionMessage( Click, m_ActionOutDown );
}

void TLGui::TWidget::OnClickEnd(const TClick& Click)
{
	SendActionMessage( Click, m_ActionOutUp );
}



//-------------------------------------------------
//	when click has been validated action message is sent to subscribers
//-------------------------------------------------
void TLGui::TWidget::SendActionMessage(const TClick& Click,TRefRef ActionRef,TBinaryTree* pExtraData)
{
	if ( !HasSubscribers( TRef_Static(A,c,t,i,o) ) )
	{
		TLDebug_Break("Subscribers should have been checked before we did tons of click processing");
		return;
	}	

#ifdef _DEBUG
	TTempString Debug_String("TWidget (");
	m_RenderNodeRef.GetString( Debug_String );
	Debug_String.Append(") outgoing click message ");
	ActionRef.GetString( Debug_String );
	Debug_String.Append(": ");
	Click.GetActionType().GetString( Debug_String );
	TLDebug_Print( Debug_String );
#endif

	if ( !ActionRef.IsValid() )
		return;

	//	make up fake input message
	TLMessaging::TMessage Message( TRef_Static(A,c,t,i,o) );
	AppendWidgetData( Message );

	Message.Write( ActionRef );
	Message.ExportData("RawData", Click.GetActionValue() );

	//	write world positions
	Message.ExportData("Pos2", Click.GetCursorPos() );
	if ( Click.IsWorldRayValid() )
		Message.ExportData("Pos3", Click.GetWorldPos(0.f) );

	//	write original action and type
	Message.ExportData("InpAction", Click.GetActionRef() );
	Message.ExportData("InpType", Click.GetActionType() );

	if ( pExtraData)
		Message.ReferenceDataTree( *pExtraData );

	//	send message
	PublishMessage( Message );
}


//-------------------------------------------------
//	widget was enabled
//-------------------------------------------------
void TLGui::TWidget::OnEnabled()
{
	//	subscribe to user's actions
	TPtr<TLUser::TUser>& pUser = TLUser::g_pUserManager->GetUser( m_UserRef );
	if ( pUser )
	{
		this->SubscribeTo( pUser );
	}
	else
	{
		TLDebug_Break("Invalid user ref for widget");
	}

	if ( m_QueuedClicks.GetSize() > 0 )
	{
		TLDebug_Break("Clicks queued up in widget on enabling, need to dump these clicks?");
	}
}

//-------------------------------------------------
//	widget disabled
//-------------------------------------------------
void TLGui::TWidget::OnDisabled()
{
	//	ditch queued up clicks
	m_QueuedClicks.Empty();

	//	unsubscribe from user's actions for a bit of a speed up
	TPtr<TLUser::TUser>& pUser = TLUser::g_pUserManager->GetUser( m_UserRef );
	if ( pUser )
	{
		this->UnsubscribeFrom( pUser );
	}
	else
	{
		TLDebug_Break("Invalid user ref for widget");
	}
}
