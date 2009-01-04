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


namespace TLGui
{
	TPtr<TGuiFactory> g_pFactory;
};



//-------------------------------------------------
//	
//-------------------------------------------------
void TLGui::Init()
{
	g_pFactory = new TGuiFactory();
}


//-------------------------------------------------
//
//-------------------------------------------------
void TLGui::Shutdown()
{
	g_pFactory = NULL;
}



TLGui::TGui::TGui(TRefRef GuiRef) : 
	m_InitialisedRenderNodes	( FALSE ),
	m_Subscribed				( FALSE ),
	m_GuiRef					( GuiRef ),  
	m_ActionIsDown				( FALSE )	
{
}


//-------------------------------------------------
//	keeps subscribing to the appropriate channels until everything is done
//-------------------------------------------------
SyncBool TLGui::TGui::Initialise(TRefRef RenderTargetRef,TRefRef RenderNodeRef,TRefRef UserRef,TRefRef ActionOutDown,TRefRef ActionOutUp)
{
	//	set vars
	m_RenderTargetRef	= RenderTargetRef;
	m_RenderNodeRef		= RenderNodeRef;
	m_UserRef			= UserRef;
	m_ActionOutDown		= ActionOutDown;
	m_ActionOutUp		= ActionOutUp;

	//	continue initialisation
	return Initialise();
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
			pUser->MapAction( m_ActionIn, pInputDevice->GetDeviceRef(), (u32)0 );	//	left mouse button
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
		pRenderNode->GetRenderFlags().Set( TLRender::TRenderNode::RenderFlags::CalcWorldBoundsBox );
		pRenderNode->GetRenderFlags().Set( TLRender::TRenderNode::RenderFlags::CalcWorldBoundsSphere );

		m_InitialisedRenderNodes = TRUE;
	}

	return SyncTrue;
}

//-------------------------------------------------
//	
//-------------------------------------------------
void TLGui::TGui::ProcessMessage(TPtr<TLMessaging::TMessage>& pMessage)
{
	if(pMessage->GetMessageRef() == "Input")
	{
		if( HasSubscribers() && pMessage->HasChannelID("Action"))
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
			TPtrArray<TLRender::TScreen>& ScreenInstances = TLRender::g_pScreenManager->GetInstanceArray();
			for ( u32 s=0;	(!pRenderTarget) && s<ScreenInstances.GetSize();	s++ )
			{
				pScreen = ScreenInstances.ElementAt(s);
				pRenderTarget = pScreen->GetRenderTarget( m_RenderTargetRef );
			}

			//	didnt find the render target
			if ( !pRenderTarget )
				break;

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

	TLDebug_Print( TString("Gui click message %s", ActionDown ? "down" : "up" ) );
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

