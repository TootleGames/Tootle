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

// Old system

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

	Bool SubscribedToAnyAction = FALSE;

	//	map action to at least one mouse device
	for ( u32 d=0;	d<TLInput::g_pInputSystem->GetSize();	d++ )
	{
		TLInput::TInputDevice& InputDevice = *(TLInput::g_pInputSystem->ElementAt( d ));
		if ( InputDevice.GetDeviceType() != "Mouse" )
			continue;

		//	subscribe to all of the button sensors
		u32 NumberOfButtons = InputDevice.GetSensorCount(TLInput::Button);

		for ( u32 s=0; s < NumberOfButtons ;s++ )
		{
			TRef ButtonRef = TLInput::GetDefaultButtonRef(s);

			//	map this action to this button sensor
			SubscribedToAnyAction |= pUser->MapAction( m_ActionIn, InputDevice.GetDeviceRef(), ButtonRef );
		}
	}

// NEW SYTEM
/*
		//	create action
		if ( !m_ActionIn.IsValid() )
		{
			// NOTE: There is a BClick generic event too which will need to be processed for when you
			// click 'press' on an object.
			// The EClick is for release and is more correct because you could click and move the cusor
			// and hence not actually want to click on the object.
			m_ActionIn = "EClick";
		
			//	subscribe to user's actions
			SubscribeTo( pUser );

			m_Subscribed = TRUE;
		}
		else
		{

			//	map action to at least one mouse device
			for ( u32 d=0;	d<TLInput::g_pInputSystem->GetSize();	d++ )
			{
				TLInput::TInputDevice& InputDevice = *(TLInput::g_pInputSystem->ElementAt( d ));
				if ( InputDevice.GetDeviceType() != "Mouse" )
					continue;

				//	subscribe to all of the button sensors
				u32 NumberOfButtons = InputDevice.GetSensorCount(TLInput::Button);

				for ( u32 s=0; s < NumberOfButtons ;s++ )
				{
					TRef ButtonRef = TLInput::GetDefaultButtonRef(s);

					//	map this action to this button sensor
					m_Subscribed |= pUser->MapAction( m_ActionIn, InputDevice.GetDeviceRef(), ButtonRef );
				}
			}
		}
*/

	//	failed to subscribe to any actions
	if ( !SubscribedToAnyAction )
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
	TLMessaging::TPublisher::Shutdown();
	TLMessaging::TSubscriber::Shutdown();
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
			TTempString Debug_String("TInputInterface ");
			m_RenderNodeRef.GetString( Debug_String );
			Debug_String.Append(" has no subscribers");
			TLDebug_Warning( Debug_String );
		}
		else 
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
void TLInput::TInputInterface::QueueClick(const int2& CursorPos,float ActionValue)		
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
SyncBool TLInput::TInputInterface::ProcessClick(const TClick& Click,TLRender::TScreen& Screen,TLRender::TRenderTarget& RenderTarget,TLRender::TRenderNode& RenderNode)
{
	//	see if ray intersects our object - check all our collision objects to get closest hit.
	SyncBool Intersection = IsIntersecting(Screen, RenderTarget, RenderNode, Click.m_CursorPos);

	//	failed to check - no valid bounds? might have to wait till next frame
	if ( Intersection == SyncWait )
		return SyncWait;

	//	send out click/no click message
	if ( Intersection == SyncTrue )
	{
		if(Click.m_ActionValue == 0.0f)
			OnClickEnd();
		else
			OnClickBegin();
	}
	else // == SyncFalse
	{
		//OnClickEnd();
	}

	return SyncTrue;
}


SyncBool TLInput::TInputInterface::IsIntersecting(TLRender::TScreen& Screen, TLRender::TRenderTarget& RenderTarget, TLRender::TRenderNode& RenderNode, const int2& Pos)
{
	SyncBool Intersection = SyncWait;

	//	test for click on the collision shapes on the render node
	TLMaths::TLine WorldRay;

	if ( !Screen.GetWorldRayFromScreenPos( RenderTarget, WorldRay, Pos ) )
	{
		//	click was out of the render target so we couldnt get a ray
		Intersection = SyncFalse;
	}
	else
	{
		//	check for a click in the fastest order...

		//	if we haven't already failed a check, test again bounds sphere
		if ( Intersection != SyncFalse )
		{
			const TLMaths::TSphere2D& WorldBoundsSphere = RenderNode.GetWorldBoundsSphere2D();
			if ( WorldBoundsSphere.IsValid() )
				Intersection = WorldBoundsSphere.GetIntersection( WorldRay ) ? SyncTrue : SyncFalse;
		}

		//	if we haven't already failed a check, test again bounds box for a tighter bounds check
		if ( Intersection != SyncFalse )
		{
			//	gr: test against 2D box
			const TLMaths::TBox2D& WorldBoundsBox2D = RenderNode.GetWorldBoundsBox2D();
			if ( WorldBoundsBox2D.IsValid() )
				Intersection = WorldBoundsBox2D.GetIntersection( WorldRay ) ? SyncTrue : SyncFalse;
		}
	}

	return Intersection;
}



void TLInput::TInputInterface::OnClickBegin()
{
	//if(m_ClickCount == 0)
		SendActionMessage( TRUE, 1.f );

	//m_ClickCount++;
}

void TLInput::TInputInterface::OnClickEnd()
{
	//m_ClickCount--;

	//if(m_ClickCount == 0)
		SendActionMessage( FALSE, 0.f );
}

void TLInput::TInputInterface::OnCursorMove()
{

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

