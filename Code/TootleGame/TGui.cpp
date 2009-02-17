#include "TGui.h"
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



TLGui::TGui::TGui(TRefRef RenderTargetRef,TRefRef RenderNodeRef,TRefRef UserRef,TRefRef ActionOutDown,TRefRef ActionOutUp)  : 
	m_InitialisedRenderNodes	( FALSE ),
	m_Subscribed				( FALSE ),
	m_ActionIsDown				( FALSE ),
	m_RenderTargetRef			( RenderTargetRef ),
	m_RenderNodeRef				( RenderNodeRef ),
	m_UserRef					( UserRef),
	m_ActionOutDown				( ActionOutDown ),
	m_ActionOutUp				( ActionOutUp )
{
	//	no actions going out means this TGui wont do anything
	if ( !m_ActionOutDown.IsValid() && !m_ActionOutUp.IsValid() )
	{
		TLDebug_Break("TGui created that won't send out actions");
	}

	//	start initialise
	SyncBool InitResult = Initialise();
	if ( InitResult == SyncWait )
	{
		//	init not finished yet, subscribe to updates until initialised
		TLCore::g_pCoreManager->Subscribe(this);
	}
}


TLGui::TGui::~TGui()
{
	TTempString Debug_String("TGui destructed ");
	m_RenderNodeRef.GetString( Debug_String );
	TLDebug_Print( Debug_String );
}


//-------------------------------------------------
//	keeps subscribing to the appropriate channels until everything is done
//-------------------------------------------------
SyncBool TLGui::TGui::Initialise()
{
	//	setup input actions
	if ( !m_Subscribed )
	{
		//	get user
		TPtr<TLUser::TUser>	pUser = TLUser::g_pUserManager->GetUser( m_UserRef );
		if ( !pUser )
			return SyncWait;

		//	just some ref we know of. Could be randomly generated or some unused value on the user...
		//	has to be unique on the user though
		m_ActionIn = pUser->GetUnusedActionRef("click");
		Bool CreatedAction = FALSE;

		//	find all the mouse devices
		for ( u32 d=0;	d<TLInput::g_pInputSystem->GetSize();	d++ )
		{
			TPtr<TLInput::TInputDevice>& pInputDevice = TLInput::g_pInputSystem->ElementAt( d );
			if ( pInputDevice->GetDeviceType() != "Mouse" )
				continue;

			//	make up action for this device
			pUser->AddAction("Simple", m_ActionIn );
			//	left mouse button
			if ( !pUser->MapAction( m_ActionIn, pInputDevice->GetDeviceRef(), (u32)0 ) )
			{
				TLDebug_Break("Failed to map action to gui");
				continue;
			}
			CreatedAction = TRUE;
		}

		//	wait until we've created some actions
		if ( !CreatedAction )
			return SyncWait;

		//	subscribe to user's actions
		SubscribeTo( pUser );

		//	mark as subscribed
		m_Subscribed = TRUE;
	}

	//	setup render nodes
	if ( !m_InitialisedRenderNodes )
	{
		//	get render node
		TPtr<TLRender::TRenderNode>& pRenderNode = TLRender::g_pRendergraph->FindNode( m_RenderNodeRef );
		if ( !pRenderNode )
			return SyncWait;

		//	enable bounds-calc flags
		pRenderNode->GetRenderFlags().Set( TLRender::TRenderNode::RenderFlags::Debug_WorldBoundsBox );
		pRenderNode->GetRenderFlags().Set( TLRender::TRenderNode::RenderFlags::CalcWorldBoundsBox );
		pRenderNode->GetRenderFlags().Set( TLRender::TRenderNode::RenderFlags::CalcWorldBoundsSphere );

		m_InitialisedRenderNodes = TRUE;
	}

	return SyncTrue;
}


//-------------------------------------------------
//	shutdown code - just unsubscribes from publishers - this is to release all the TPtr's so we can be destructed
//-------------------------------------------------
void TLGui::TGui::Shutdown()
{
	TLMessaging::TPublisher::Shutdown();
	TLMessaging::TSubscriber::Shutdown();
}


//-------------------------------------------------
//	
//-------------------------------------------------
void TLGui::TGui::ProcessMessage(TPtr<TLMessaging::TMessage>& pMessage)
{
	if(pMessage->GetMessageRef() == "Input")
	{
		if ( !HasSubscribers() )
		{
			TTempString Debug_String("TGui ");
			m_RenderNodeRef.GetString( Debug_String );
			Debug_String.Append(" has no subscribers");
			TLDebug_Warning( Debug_String );
		}
		else if( pMessage->HasChannelID("Action"))
		{
			TRef ActionRef;
			float RawValue = 0.f;
			if ( pMessage->Read(ActionRef) && pMessage->ImportData("RawData", RawValue) )
			{
				if ( ActionRef == m_ActionIn )
				{
					int2 CursorPosition;
					pMessage->ImportData("CURSOR", CursorPosition );
					
					//	queue up this click
					QueueClick( CursorPosition, RawValue );

					//	now process ALL the queued clicks so if we have some unprocessed they're not lost and kept in order
					ProcessQueuedClicks();
				}
			}
		}
	}
	else if ( pMessage->GetMessageRef() == TLCore::UpdateRef )
	{
		//	keep doing initialise
		if ( Initialise() != SyncWait )
		{
			//	initialise finished, no need to update any more
			TLCore::g_pCoreManager->Unsubscribe(this);
		}
	}
}


//-------------------------------------------------
//	go through queued-up (unhandled) clicks and respond to them
//-------------------------------------------------
void TLGui::TGui::ProcessQueuedClicks()
{
	//	if we have no subscribers we can just ditch the clicks...
	if ( !HasSubscribers() )
	{
		m_QueuedClicks.Empty();
		return;
	}

	//	get render target as we need it (but hold onto the pointer to save getting it more than once)
	TPtr<TLRender::TRenderNode> pRenderNode;
	TPtr<TLRender::TScreen> pScreen;
	TPtr<TLRender::TRenderTarget> pRenderTarget;

	while ( m_QueuedClicks.GetSize() )
	{
		TClick& Click = m_QueuedClicks[0];

		//	don't need a render target if it's a mouse-release
		if ( Click.m_ActionValue == 0.f )
		{
			SendActionMessage( FALSE );
			m_QueuedClicks.RemoveAt( 0 );
			continue;
		}

		//	is a mouse-down... fetch render target if we need it
		if ( !pRenderNode )
		{
			//	find the render target in a screen...
			TPtr<TLRender::TRenderTarget>& pRenderTargetPtr = TLRender::g_pScreenManager->GetRenderTarget( m_RenderTargetRef, pScreen );
			//	didnt find the render target
			if ( !pRenderTargetPtr )
				break;

			//	render target isnt enabled, ignore
			if ( !pRenderTargetPtr->IsEnabled() )
				break;

			//	
			pRenderTarget = pRenderTargetPtr;

			//	got a render target, fetch a render node
			pRenderNode = TLRender::g_pRendergraph->FindNode( m_RenderNodeRef );
			if ( !pRenderNode )
				break;
		}

		//	test for click on the collision shapes on the render node
		TLMaths::TLine WorldRay;

		//	see if ray intersects our object - check all our collision objects to get closest hit.
		SyncBool Intersection = SyncWait;

		if ( !pScreen->GetWorldRayFromScreenPos( pRenderTarget, WorldRay, Click.m_CursorPos ) )
		{
			//	click was out of the render target so we couldnt get a ray
			Intersection = SyncFalse;
		}

		//	fastest order!

/*		//	if valid and we havent already decided click has missed
		if ( pRenderNode->GetWorldBoundsSphere().IsValid() && Intersection != SyncFalse )
		{
			if ( pRenderNode->GetWorldBoundsSphere().GetIntersection( WorldRay ) )
				Intersection = SyncTrue;
			else
				Intersection = SyncFalse;
		}

		//	if valid and we havent already decided click has missed
		if ( pRenderNode->GetWorldBoundsCapsule().IsValid() && Intersection != SyncFalse )
		{
			if ( pRenderNode->GetWorldBoundsCapsule().GetIntersection( WorldRay ) )
				Intersection = SyncTrue;
			else
				Intersection = SyncFalse;
		}
*/
		//	if valid and we havent already decided click has missed
		if ( pRenderNode->GetWorldBoundsBox().IsValid() && Intersection != SyncFalse )
		{
			if ( pRenderNode->GetWorldBoundsBox().GetIntersection( WorldRay ) )
				Intersection = SyncTrue;
			else
				Intersection = SyncFalse;
		}

		//	failed to check - no valid bounds? might have to wait till next frame
		if ( Intersection == SyncWait )
			break;

		//	
		SendActionMessage( Intersection == SyncTrue );

		//	done this click
		m_QueuedClicks.RemoveAt( 0 );
	}

	//	gr: todo: if we don't process a click (and have a queue) subscribe to a core update
	//			so we can keep trying until we empty the queue
}


//-------------------------------------------------
//	when click has been validated action message is sent to subscribers
//-------------------------------------------------
void TLGui::TGui::SendActionMessage(Bool ActionDown)
{
	if ( !HasSubscribers() )
		return;
	
	//	if action is "up" and we're already "up" dont need to send the message
//	if ( !ActionDown && !m_ActionIsDown )
//		return;

	TRef ActionOutRef = ActionDown ? m_ActionOutDown : m_ActionOutUp;
	float RawDataValue = ActionDown ? 1.f : 0.f;

#ifdef _DEBUG
	TTempString Debug_String("TGui (");
	m_RenderNodeRef.GetString( Debug_String );
	Debug_String.Append(") outgoing click message ");
	ActionOutRef.GetString( Debug_String );
	Debug_String.Appendf(": %s", ActionDown ? "down" : "up" );
	TLDebug_Print( Debug_String );
#endif

	if ( ActionOutRef.IsValid() )
	{
		//	make up fake input message
		TPtr<TLMessaging::TMessage> pMessage = new TLMessaging::TMessage("Input");
		pMessage->AddChannelID("Action");
		pMessage->Write( ActionOutRef );
		pMessage->ExportData("RawData", RawDataValue );

		//	send message
		PublishMessage( pMessage );
	}

	//	update current action state
	m_ActionIsDown = ActionDown;
}

