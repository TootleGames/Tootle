#include "TInputInterface.h"
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

TLInput::TInputInterface::TInputInterface(TRefRef RenderTargetRef,TRefRef RenderNodeRef,TRefRef UserRef,TRefRef ActionOutDown,TRefRef ActionOutUp)  : 
	m_Initialised				( SyncWait ),
	m_RenderTargetRef			( RenderTargetRef ),
	m_RenderNodeRef				( RenderNodeRef ),
	m_UserRef					( UserRef),
	m_ActionOutDown				( ActionOutDown ),
	m_ActionOutUp				( ActionOutUp )
	//m_ClickCount				( 0 )
{
	//	no actions going out means this TInputInterface wont do anything
	if ( !m_ActionOutDown.IsValid() && !m_ActionOutUp.IsValid() )
	{
		TLDebug_Break("TInputInterface created that won't send out actions");
	}

	//	start initialise
	SyncBool InitResult = Initialise();
	if ( InitResult == SyncWait )
	{
		//	init not finished yet, subscribe to updates until initialised
		this->SubscribeTo( TLCore::g_pCoreManager );
	}
}


TLInput::TInputInterface::~TInputInterface()
{
	TTempString Debug_String("TInputInterface destructed ");
	m_RenderNodeRef.GetString( Debug_String );
	TLDebug_Print( Debug_String );
	
	if(m_ActionInClick.IsValid() || m_ActionInMove.IsValid())
	{
		TLDebug_Print("Action hasn't been cleared.  has shutown been called?");
		TLDebug_Break("Doing last chance action removal");
		RemoveAllActions();
	}

}


//-------------------------------------------------
//	keeps subscribing to the appropriate channels until everything is done
//-------------------------------------------------
SyncBool TLInput::TInputInterface::Initialise()
{
	//	init done alreayd
	if ( m_Initialised != SyncWait )
		return m_Initialised;

	//	get user
	TPtr<TLUser::TUser>	pUser = TLUser::g_pUserManager->GetUser( m_UserRef );
	if ( !pUser )
		return SyncWait;

	//	subscribe to user's actions
	if(!SubscribeTo( pUser ))
		return SyncWait;


	//	all done
	m_Initialised = SyncTrue;

	//	notify init has finished
	OnInitialised();

	return m_Initialised;
}


//-------------------------------------------------
//	get array of all the render nodes we're using
//-------------------------------------------------
void TLInput::TInputInterface::GetRenderNodes(TArray<TRef>& RenderNodeArray)
{
	RenderNodeArray.Add( m_RenderNodeRef );
}


//-------------------------------------------------
//	shutdown code - just unsubscribes from publishers - this is to release all the TPtr's so we can be destructed
//-------------------------------------------------
void TLInput::TInputInterface::Shutdown()
{
	RemoveAllActions();
	
	
	TLMessaging::TPublisher::Shutdown();
	TLMessaging::TSubscriber::Shutdown();
}


void TLInput::TInputInterface::RemoveAllActions()
{
	// Remove the actions from the user
	TPtr<TLUser::TUser>	pUser = TLUser::g_pUserManager->GetUser( m_UserRef );
	if ( pUser )
	{
		if(m_ActionInClick.IsValid())
		{
			if(!pUser->RemoveAction(m_ActionInClick))
			{
				TLDebug_Break("Failed to remove action");
			}
			
			m_ActionInClick.SetInvalid();
		}
		
		if(m_ActionInMove.IsValid())
		{
			if(!pUser->RemoveAction(m_ActionInMove))
			{
				TLDebug_Break("Failed to remove action");
			}
			m_ActionInMove.SetInvalid();
		}
	}
	else
	{
		TLDebug_Break("Failed to get user");
	}

}


//-------------------------------------------------
//	
//-------------------------------------------------
void TLInput::TInputInterface::ProcessMessage(TLMessaging::TMessage& Message)
{
	if(Message.GetMessageRef() == TRef_Static(A,c,t,i,o))
	{
		if ( !HasSubscribers() )
		{
#ifdef _DEBUG
			TTempString Debug_String("TInputInterface ");
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
					float RawValue = 0.f;
					if ( Message.ImportData("RawData", RawValue) )
					{
							//	queue up this click
							QueueClick( CursorPosition, RawValue, ActionRef );
					}
				}
				else if(TLGui::g_pWidgetManager->IsMoveActionRef(ActionRef))
				{
					OnCursorMove(CursorPosition, ActionRef);
				}
				
				//	now process ALL the queued clicks so if we have some unprocessed they're not lost and kept in order
				ProcessQueuedClicks();
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
void TLInput::TInputInterface::QueueClick(const int2& CursorPos,float ActionValue, TRefRef ActionRef)		
{
	//	if this "click" is a mouse up, and the previous was too, then dont add it
	if ( m_QueuedClicks.GetSize() > 0 )
	{
		//	both this and prev action values were "off" so skip adding to the queue
		if ( ActionValue < TLMaths_NearZero && m_QueuedClicks.ElementLast().GetActionValue() < TLMaths_NearZero )
			return;
	}

	//	add to queue
	m_QueuedClicks.Add( TClick( CursorPos, ActionValue, ActionRef ) );
}



//-------------------------------------------------
//	update routine - return FALSE if we don't need updates any more
//-------------------------------------------------
Bool TLInput::TInputInterface::Update()
{
	Bool ContinueUpdate = FALSE;

	//	keep doing initialise
	if ( Initialise() == SyncWait )
		ContinueUpdate |= TRUE;

	//	process any queued up clicks
	ProcessQueuedClicks();

	//	some clicks still need processing
	if ( m_QueuedClicks.GetSize() > 0 )
		ContinueUpdate |= TRUE;

	//	initialise finished, no need to update any more
	return ContinueUpdate;
}


//-------------------------------------------------
//	go through queued-up (unhandled) clicks and respond to them
//-------------------------------------------------
void TLInput::TInputInterface::ProcessQueuedClicks()
{
	//	no queue
	if ( m_QueuedClicks.GetSize() == 0 )
		return;

	//	if we have no subscribers we can just ditch the clicks...
	if ( !HasSubscribers() )
	{
		m_QueuedClicks.Empty();
		return;
	}

	//	find the render target in a screen...
	TPtr<TLRender::TScreen> pScreen;
	TPtr<TLRender::TRenderTarget>& pRenderTarget = TLRender::g_pScreenManager->GetRenderTarget( m_RenderTargetRef, pScreen );

	//	didnt find the render target
	if ( !pRenderTarget )
		return;

	//	render target isnt enabled, ignore
	if ( !pRenderTarget->IsEnabled() )
		return;

	//	got a render target, fetch a render node
	TPtr<TLRender::TRenderNode>& pRenderNode = m_RenderNodeRef.IsValid() ? TLRender::g_pRendergraph->FindNode( m_RenderNodeRef ) : TLPtr::GetNullPtr<TLRender::TRenderNode>();

	//	gr: for support of those that don't use this base render node variable, only check if the ref is valid
	if ( m_RenderNodeRef.IsValid() && !pRenderNode )
		return;


	TLRender::TScreen& Screen = *pScreen;
	TLRender::TRenderTarget& RenderTarget = *pRenderTarget;
	TLRender::TRenderNode& RenderNode = *pRenderNode;

	while ( m_QueuedClicks.GetSize() )
	{
		TClick& Click = m_QueuedClicks[0];

		if ( ProcessClick( Click, Screen, RenderTarget, RenderNode ) == SyncWait )
			break;

		m_QueuedClicks.RemoveAt( 0 );
	}

	//	need to process this click again later so subscribe to updates to process them next time we can
	if ( m_QueuedClicks.GetSize() > 0 )
		this->SubscribeTo( TLCore::g_pCoreManager );
}


//-------------------------------------------------
//	process a click and detect clicks on/off our render node. return SyncWait if we didnt process it and want to process again
//-------------------------------------------------
SyncBool TLInput::TInputInterface::ProcessClick(TClick& Click,TLRender::TScreen& Screen,TLRender::TRenderTarget& RenderTarget,TLRender::TRenderNode& RenderNode)
{
	Bool bClickAction = TLGui::g_pWidgetManager->IsClickActionRef(Click.GetActionRef());
	Bool bMoveAction = TLGui::g_pWidgetManager->IsMoveActionRef(Click.GetActionRef());
	Bool bCurrentAction = FALSE;
	Bool bActionPair = FALSE;
	
	if(m_ActionBeingProcessedRef.IsValid())
	{
		bCurrentAction = (Click.GetActionRef() == m_ActionBeingProcessedRef);
		bActionPair = TLGui::g_pWidgetManager->IsActionPair(m_ActionBeingProcessedRef, Click.GetActionRef());
	}

	
	Bool bTestIntersection = FALSE;
	
	if(m_ActionBeingProcessedRef.IsValid() )
	{
		if(bClickAction && bCurrentAction)
		{
			// Click occured and it's the same as the currently processing action
			bTestIntersection = TRUE;
		}
		else if(bMoveAction)
		{
			if(bActionPair)
			{
				// move action accured and is paired with our action being processed
				bTestIntersection = TRUE;
			}
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
	SyncBool Intersection = IsIntersecting(Screen, RenderTarget, RenderNode, Click );

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
			if(Click.GetActionValue() == 0.0f)
			{
				OnClickEnd( Click );

				// If the click was an actual click, not a move, then set the action being processed ref
				if(bClickAction)
					m_ActionBeingProcessedRef.SetInvalid();
			}
			else
			{
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


SyncBool TLInput::TInputInterface::IsIntersecting(TLRender::TScreen& Screen, TLRender::TRenderTarget& RenderTarget, TLRender::TRenderNode& RenderNode,TClick& Click)
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
			const TLMaths::TShapeSphere2D& WorldBoundsSphere = RenderNode.GetWorldBoundsSphere2D();
			if ( WorldBoundsSphere.IsValid() )
				Intersection = WorldBoundsSphere.GetSphere().GetIntersection( WorldRay ) ? SyncTrue : SyncFalse;
		}

		//	if we haven't already failed a check, test again bounds box for a tighter bounds check
		if ( Intersection != SyncFalse )
		{
			//	gr: test against 2D box
			const TLMaths::TShapeBox2D& WorldBoundsBox2D = RenderNode.GetWorldBoundsBox2D();
			if ( WorldBoundsBox2D.IsValid() )
				Intersection = WorldBoundsBox2D.GetBox().GetIntersection( WorldRay ) ? SyncTrue : SyncFalse;
		}
	}

	return Intersection;
}



void TLInput::TInputInterface::OnClickBegin(const TClick& Click)
{
	//if(m_ClickCount == 0)
		SendActionMessage( TRUE, 1.f );

	//m_ClickCount++;
}

void TLInput::TInputInterface::OnClickEnd(const TClick& Click)
{
	//m_ClickCount--;

	//if(m_ClickCount == 0)
		SendActionMessage( FALSE, 0.f );
}



//-------------------------------------------------
//	when click has been validated action message is sent to subscribers
//-------------------------------------------------
void TLInput::TInputInterface::SendActionMessage(Bool ActionDown,float RawData)
{
	if ( !HasSubscribers() )
		return;
	
	TRef ActionOutRef = ActionDown ? m_ActionOutDown : m_ActionOutUp;

#ifdef _DEBUG
	TTempString Debug_String("TInputInterface (");
	m_RenderNodeRef.GetString( Debug_String );
	Debug_String.Append(") outgoing click message ");
	ActionOutRef.GetString( Debug_String );
	Debug_String.Appendf(": %s", ActionDown ? "down" : "up" );
	TLDebug_Print( Debug_String );
#endif

	if ( ActionOutRef.IsValid() )
	{
		//	make up fake input message
		TLMessaging::TMessage Message(TRef_Static(A,c,t,i,o));
		Message.Write( ActionOutRef );
		Message.ExportData("RawData", RawData );

		//	send message
		PublishMessage( Message );
	}
}

