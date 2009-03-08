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
		this->SubscribeTo( TLCore::g_pCoreManager );
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
	//	already initialised
	if ( m_Subscribed && m_InitialisedRenderNodes )
		return SyncTrue;

	//	setup render nodes
	if ( !m_InitialisedRenderNodes )
	{
		u32 ValidRenderNodeCount = 0;
		TFixedArray<TRef,100> RenderNodeArray(0);
		GetRenderNodes( RenderNodeArray );

		//	get render node
		for ( u32 i=0;	i<RenderNodeArray.GetSize();	i++ )
		{
			TRefRef RenderNodeRef = RenderNodeArray[i];
			if ( !RenderNodeRef.IsValid() )
				continue;

			TPtr<TLRender::TRenderNode>& pRenderNode = TLRender::g_pRendergraph->FindNode( RenderNodeRef );
			if ( !pRenderNode )
				return SyncWait;

			//	enable bounds-calc flags
		//	pRenderNode->GetRenderFlags().Set( TLRender::TRenderNode::RenderFlags::Debug_WorldBoundsBox );
			pRenderNode->GetRenderFlags().Set( TLRender::TRenderNode::RenderFlags::CalcWorldBoundsBox );
			pRenderNode->GetRenderFlags().Set( TLRender::TRenderNode::RenderFlags::CalcWorldBoundsSphere );

			ValidRenderNodeCount++;
		}

		//	to get around the problem of lack of vtables... as this function is called from a constructor
		//	if none of the render nodes were valid, we'll try again on the next init
		if ( ValidRenderNodeCount > 0 )
			m_InitialisedRenderNodes = TRUE;
	}

	//	setup input actions
	if ( !m_Subscribed )
	{
		//	get user
		TPtr<TLUser::TUser>	pUser = TLUser::g_pUserManager->GetUser( m_UserRef );
		if ( !pUser )
			return SyncWait;

		//	create action
		if ( !m_ActionIn.IsValid() )
		{
			//	just some ref we know of. Could be randomly generated or some unused value on the user...
			//	has to be unique on the user though
			TRef NewActionRef = pUser->GetUnusedActionRef("click");
			
			//	make up action for this device
			if ( !pUser->AddAction("Simple", NewActionRef ) )
			{
				TLDebug_Break("Failed to create new action on user");
				return SyncWait;
			}

			m_ActionIn = NewActionRef;
		
			//	subscribe to user's actions
			SubscribeTo( pUser );
		}

		//	map action to at least one mouse device
		for ( u32 d=0;	d<TLInput::g_pInputSystem->GetSize();	d++ )
		{
			TLInput::TInputDevice& InputDevice = *(TLInput::g_pInputSystem->ElementAt( d ));
			if ( InputDevice.GetDeviceType() != "Mouse" )
				continue;

			//	subscribe to all of the button sensors
			TPtrArray<TLInput::TInputSensor>& InputDeviceSensors = InputDevice.GetSensors();

			for ( u32 s=0;	s<InputDeviceSensors.GetSize();	s++ )
			{
				TPtr<TLInput::TInputSensor>& pSensor = InputDeviceSensors[s];
				if ( pSensor->GetSensorType() != TLInput::Button )
					continue;

				//	map this action to this button sensor
				m_Subscribed |= pUser->MapAction( m_ActionIn, pSensor );
			}
		}

		//	mark as subscribed
		if ( !m_Subscribed )
			return SyncWait;
	}

	//	still waiting
	if ( !m_Subscribed || !m_InitialisedRenderNodes )
		return SyncWait;

	//	notify init has finished
	OnInitialised();
	return SyncTrue;
}


//-------------------------------------------------
//	get array of all the render nodes we're using
//-------------------------------------------------
void TLGui::TGui::GetRenderNodes(TArray<TRef>& RenderNodeArray)
{
	RenderNodeArray.Add( m_RenderNodeRef );
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
void TLGui::TGui::ProcessMessage(TLMessaging::TMessage& Message)
{
	if(Message.GetMessageRef() == "Input")
	{
		if ( !HasSubscribers() )
		{
			TTempString Debug_String("TGui ");
			m_RenderNodeRef.GetString( Debug_String );
			Debug_String.Append(" has no subscribers");
			TLDebug_Warning( Debug_String );
		}
		else if( Message.HasChannelID("Action"))
		{
			TRef ActionRef;
			float RawValue = 0.f;
			if ( Message.Read(ActionRef) && Message.ImportData("RawData", RawValue) )
			{
				if ( ActionRef == m_ActionIn )
				{
					int2 CursorPosition;
					Message.ImportData("CURSOR", CursorPosition );
					
					//	queue up this click
					QueueClick( CursorPosition, RawValue );

					//	now process ALL the queued clicks so if we have some unprocessed they're not lost and kept in order
					ProcessQueuedClicks();
				}
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
void TLGui::TGui::QueueClick(const int2& CursorPos,float ActionValue)		
{
	//	if this "click" is a mouse up, and the previous was too, then dont add it
	if ( m_QueuedClicks.GetSize() > 0 )
	{
		//	both this and prev action values were "off" so skip adding to the queue
		if ( ActionValue < TLMaths::g_NearZero && m_QueuedClicks.ElementLast().m_ActionValue < TLMaths::g_NearZero )
			return;
	}

	//	add to queue
	m_QueuedClicks.Add( TClick( CursorPos, ActionValue ) );
}



//-------------------------------------------------
//	update routine - return FALSE if we don't need updates any more
//-------------------------------------------------
Bool TLGui::TGui::Update()
{
	//	keep doing initialise
	if ( Initialise() == SyncWait )
		return TRUE;

	//	initialise finished, no need to update any more
	return FALSE;
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


	while ( m_QueuedClicks.GetSize() )
	{
		TClick& Click = m_QueuedClicks[0];

		if ( ProcessClick( Click, *pScreen.GetObject(), *pRenderTarget.GetObject(), pRenderNode ) == SyncWait )
			break;

		m_QueuedClicks.RemoveAt( 0 );
	}

	//	gr: todo: if we don't process a click (and have a queue) subscribe to a core update
	//			so we can keep trying until we empty the queue
}


//-------------------------------------------------
//	process a click and detect clicks on/off our render node. return SyncWait if we didnt process it and want to process again
//-------------------------------------------------
SyncBool TLGui::TGui::ProcessClick(const TClick& Click,TLRender::TScreen& Screen,TLRender::TRenderTarget& RenderTarget,TPtr<TLRender::TRenderNode>& pRenderNode)
{
	if ( Click.m_ActionValue == 0.f )
	{
		SendActionMessage( FALSE, 0.f );
		return SyncTrue;
	}

	//	test for click on the collision shapes on the render node
	TLMaths::TLine WorldRay;

	//	see if ray intersects our object - check all our collision objects to get closest hit.
	SyncBool Intersection = SyncWait;

	if ( !Screen.GetWorldRayFromScreenPos( RenderTarget, WorldRay, Click.m_CursorPos ) )
	{
		//	click was out of the render target so we couldnt get a ray
		Intersection = SyncFalse;
	}
	else
	{
		//	fastest order!

	/*		//	if valid and we havent already decided click has missed
		if ( pRenderNode->GetWorldBoundsSphere().IsValid() )
		{
			if ( pRenderNode->GetWorldBoundsSphere().GetIntersection( WorldRay ) )
				Intersection = SyncTrue;
			else
				Intersection = SyncFalse;
		}

		//	if valid and we havent already decided click has missed
		if ( pRenderNode->GetWorldBoundsCapsule().IsValid() )
		{
			if ( pRenderNode->GetWorldBoundsCapsule().GetIntersection( WorldRay ) )
				Intersection = SyncTrue;
			else
				Intersection = SyncFalse;
		}
	*/
		//	if valid and we havent already decided click has missed
		if ( pRenderNode->GetWorldBoundsBox().IsValid() )
		{
			if ( pRenderNode->GetWorldBoundsBox().GetIntersection( WorldRay ) )
				Intersection = SyncTrue;
			else
				Intersection = SyncFalse;
		}
	}

	//	failed to check - no valid bounds? might have to wait till next frame
	if ( Intersection == SyncWait )
		return SyncWait;

	//	send out click/no click message
	if ( Intersection == SyncTrue )
	{
		SendActionMessage( TRUE, 1.f );
	}
	else
	{
		SendActionMessage( FALSE, 0.f );
	}

	return SyncTrue;
}


//-------------------------------------------------
//	when click has been validated action message is sent to subscribers
//-------------------------------------------------
void TLGui::TGui::SendActionMessage(Bool ActionDown,float RawData)
{
	if ( !HasSubscribers() )
		return;
	
	//	if action is "up" and we're already "up" dont need to send the message
//	if ( !ActionDown && !m_ActionIsDown )
//		return;

	TRef ActionOutRef = ActionDown ? m_ActionOutDown : m_ActionOutUp;

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
		TLMessaging::TMessage Message("Input");
		Message.AddChannelID("Action");
		Message.Write( ActionOutRef );
		Message.ExportData("RawData", RawData );

		//	send message
		PublishMessage( Message );
	}

	//	update current action state
	m_ActionIsDown = ActionDown;
}

