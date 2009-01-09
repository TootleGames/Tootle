#include "TCoreManager.h"
#include "TEventChannel.h"
#include "TSyncQueue.h"

#include "TLMaths.h"
#include "TRefManager.h"
#include "TLTime.h"

using namespace TLCore;

// DB - quick define for the maximum and minimum time step modifiers we can reach
#define TIMESTEP_MIN 0.0f
#define TIMESTEP_MAX 10.0f

#if defined(TL_TARGET_IPOD)
	#define LIMIT_UPDATE_RATE
	#define LIMIT_RENDER_RATE
#elif defined(TL_TARGET_PC)
	#define LIMIT_UPDATE_RATE
	//#define LIMIT_RENDER_RATE
#endif

TCoreManager::TCoreManager(TRefRef refManagerID) :
	TManager							( refManagerID ),
	m_Debug_FramesPerSecond				( 0 ),
	m_Debug_CurrentFramesPerSecond		( 0 ),
	m_Debug_UpdatesPerSecond			( 0 ),
	m_Debug_CurrentUpdatesPerSecond		( 0 ),
	m_Debug_FrameTimePerSecond			( 0.f ),
	m_Debug_CurrentFrameTimePerSecond	( 0.f ),
	m_ChannelsInitialised				( FALSE ),
	m_bEnabled							( TRUE ),
	m_bQuit								( FALSE )
{
}

TCoreManager::~TCoreManager()
{
	UnregisterAllManagers();
}


SyncBool TCoreManager::Initialise()
{
	return SyncTrue;
}

SyncBool TCoreManager::Update(float fTimeStep)
{
	/*
	//	need to do some updates?
	if (HasTimeSteps())
		return SyncWait;

	//	publish an update
	PublishUpdateMessage();

	//	publish a render
	PublishRenderMessage();
	*/

	return SyncWait;
}

SyncBool TCoreManager::Shutdown()
{
	return SyncTrue;
}

void TCoreManager::Enable(Bool bEnable)
{
	if(bEnable)
	{
		// Enable the core manager updates
		if(!m_bEnabled)
		{
			TLTime::TTimestampMicro TimeNow(TRUE);

			m_LastUpdateTime = TimeNow;
			m_LastRenderTime = TimeNow;
			
			m_bEnabled = TRUE;
		}
	}
	else
	{
		// Disable the core manager updates
		if(m_bEnabled)
		{
			m_bEnabled = FALSE;
		}
	}
}


//-----------------------------------------------------
// one process of Main init
//-----------------------------------------------------
SyncBool TCoreManager::InitialiseLoop()	
{
	//	initialise channels once
	if ( !m_ChannelsInitialised )
	{
		//	If we have the event channel manager then register some event channels
		if(!TLMessaging::g_pEventChannelManager)
			return SyncWait;
		
		for(u32 uIndex = 0; uIndex < m_Managers.GetSize(); uIndex++)
		{
			TPtr<TManager> pManager = m_Managers.ElementAt(uIndex);
			
			// Subscribe all managers to the event channel manager - will notify changes to event channels
			// i.e. additions and removals.  This will allow managers to susbscribe to event channels 
			// as they become available rather than assuming they exist and being dependant on the order of
			// manager initialisation
			pManager->SubscribeTo(TLMessaging::g_pEventChannelManager);
		}
		
		// Create the base event channels for the core manager
		// All managers will subscribe to the initialise and shutdown channels by default in the 
		// base TManager::OnEventChannelAdded() rotuine
		TLMessaging::g_pEventChannelManager->RegisterEventChannel(this, GetManagerID(), InitialiseRef);
		TLMessaging::g_pEventChannelManager->RegisterEventChannel(this, GetManagerID(), UpdateRef);
		TLMessaging::g_pEventChannelManager->RegisterEventChannel(this, GetManagerID(), RenderRef);
		TLMessaging::g_pEventChannelManager->RegisterEventChannel(this, GetManagerID(), ShutdownRef);


		//TODO: Platform specific manager??
		TLCore::Platform::Init();

		//	other non complex one-off init's
		TLTime::Platform::Init();
		TLMaths::Init();

		m_ChannelsInitialised = TRUE;
	}

	// Initialisation of all other managers
	// NOTE: May be able to move this into the TootMain routine which also does the
	//		 Initialisation of the core manager.   Maybe even add the core manager to itself?

	// Send out initialisation messages until the managers are either in the 'ready' state
	PublishInitMessage();

	//	manager[s] in error state, init failed
	if ( CheckManagersInErrorState() )
		return SyncFalse;
	
	//	everyone ready, init finished
	if ( CheckManagersInState(TLManager::S_Ready) )
		return SyncTrue;

	//	manager[s] not ready yet
	return SyncWait;
}


//-----------------------------------------------------
// one process of Main update loop
//-----------------------------------------------------
SyncBool TCoreManager::UpdateLoop()
{
	// Main update loop phase

	// Send the update and render message
	// Not required when using multithreading?
	//TLTime::TScopeTimer Timer("loop");

	{
		// TODO: Add Platform manager to handle platform related stuff?
	//	TLTime::TScopeTimer Timer("Platform update");
		TLCore::Platform::Update();
	}

	//	only attempt a render if we do an update
	Bool DoRender = FALSE;

	{
	//	TLTime::TScopeTimer Timer("update");
		DoRender = PublishUpdateMessage();
	}

#if !defined(LIMIT_RENDER_RATE)
	DoRender = TRUE;
#endif

	if ( DoRender )
	{
	//	TLTime::TScopeTimer Timer("render");
		PublishRenderMessage();
	}

	{
	//	TLTime::TScopeTimer Timer("queue");		// Process the message queue
		ProcessMessageQueue();
	}

	//	error with manager[s]
	if ( CheckManagersInErrorState() )
		return SyncFalse;

	//	if quit, return positive result
	if ( m_bQuit )
		return SyncTrue;

	//	update fps
	TLTime::TTimestamp TimeNow(TRUE);
	if ( !m_Debug_LastCountTime.IsValid() || m_Debug_LastCountTime.GetMilliSecondsDiff(TimeNow) > 1000 )
	{
		m_Debug_LastCountTime = TimeNow;

		m_Debug_FramesPerSecond			= m_Debug_CurrentFramesPerSecond;
		m_Debug_CurrentFramesPerSecond	= 0;
		m_Debug_UpdatesPerSecond		= m_Debug_CurrentUpdatesPerSecond;
		m_Debug_CurrentUpdatesPerSecond	= 0;
		m_Debug_FrameTimePerSecond		= m_Debug_CurrentFrameTimePerSecond;
		m_Debug_CurrentFrameTimePerSecond = 0.f;
	}

	//	empty timesteps
	m_UpdateTimeQueue.Empty();

	//	still updating
	return SyncWait;
}


//-----------------------------------------------------
// one process of Main shutdown
//-----------------------------------------------------
SyncBool TCoreManager::UpdateShutdown()
{
	// Main shutdown phase
	// NOTE: May be able to move this into the core managers shutdown ro the TootMain

	//	shutdown app and app's libs
	PublishShutdownMessage();

	//	wait for all managers to come out of shutdown
	//	not all in shutdown yet...
	if ( !CheckManagersInState(TLManager::S_Shutdown) )
		return SyncWait;

	// TODO: Platform specific manager?
	TLCore::Platform::Shutdown();

	return SyncTrue;
}


//-------------------------------------------
//	publish an init
//-------------------------------------------
void TCoreManager::PublishInitMessage()
{
	TPtr<TLMessaging::TMessage> pMessage = new TLMessaging::TMessage(InitialiseRef);
	pMessage->AddChannelID(InitialiseRef);
	
	PublishMessage( pMessage );
}


//-------------------------------------------
//	publish a shutdown
//-------------------------------------------
void TCoreManager::PublishShutdownMessage()
{
	TPtr<TLMessaging::TMessage> pMessage = new TLMessaging::TMessage(ShutdownRef);
	pMessage->AddChannelID(ShutdownRef);
	
	PublishMessage( pMessage );
}


//-------------------------------------------
//	publish an update
//-------------------------------------------
Bool TCoreManager::PublishUpdateMessage(Bool bForced)
{
	TLTime::TTimestampMicro TimeNow(TRUE);
	float fTimeStep = GetUpdateTimeStepDifference(TimeNow);

	//	no need to process framesteps less than 1 frame
	#ifdef LIMIT_UPDATE_RATE
	{
		if(!bForced)
		{
			float fFrameStep = fTimeStep * (float)TLTime::GetUpdatesPerSecond();
			if ( fFrameStep < 1.0f )
				return FALSE;
		}
	}
	#endif
	
	if ( bForced )
	{
		fTimeStep = 1.f / TLTime::GetUpdatesPerSecond();
	}
	
	//	reset last update timestamp
	m_LastUpdateTime = TimeNow;

	//	create an update message
	TPtr<TLMessaging::TMessage> pMessage = new TLMessaging::TMessage(UpdateRef);
	pMessage->AddChannelID(UpdateRef);

	//	add timestep data
	pMessage->AddChildAndData( TLCore::TimeStepRef, fTimeStep );

	//	add timestep modifier data
	float fModifier = GetTimeStepModifier();
	pMessage->AddChildAndData( TLCore::TimeStepModRef, fModifier );

	//	send message
	PublishMessage( pMessage );

	m_Debug_CurrentFrameTimePerSecond += fTimeStep;
	m_Debug_CurrentUpdatesPerSecond++;

	return TRUE;
}

float TCoreManager::GetTimeStepModifier()
{
	// No modifiers?  Then return a modifier of 1.0f to allow to run at full time step speed
	if(m_aTimeStepModifiers.GetSize() == 0)
		return 1.0f;

	float fModifier = 1.0f;

	for(u32 uIndex = 0; uIndex < m_aTimeStepModifiers.GetSize(); uIndex++)
	{
		float fValue = m_aTimeStepModifiers.ElementAt(uIndex);
		fModifier *= fValue;
	}

	TLMaths::Limit(fModifier, TIMESTEP_MIN, TIMESTEP_MAX);

	// fModifier is a final percentage reduction in the time step
	return fModifier;
}

Bool TCoreManager::SetTimeStepModifier(TRefRef ModiferRef, const float& fValue)
{
	float* pfValue = m_aTimeStepModifiers.Find(ModiferRef);

	if(pfValue)
	{
		*pfValue = fValue;
		return TRUE;
	}

	// Not found
	return FALSE;
}


Bool TCoreManager::IncrementTimeStepModifier(TRefRef ModiferRef, const float& fValue)
{
	float fCurrentValue = 0.0f;
		
	if(GetTimeStepModifier(ModiferRef, fCurrentValue))
	{
		fCurrentValue += fValue;
		TLMaths::Limit(fCurrentValue, TIMESTEP_MIN, TIMESTEP_MAX);
		return SetTimeStepModifier(ModiferRef, fCurrentValue);
	}

	// Failed
	return FALSE;
}

Bool TCoreManager::DecrementTimeStepModifier(TRefRef ModiferRef, const float& fValue)
{
	float fCurrentValue = 0.0f;
	
	if(GetTimeStepModifier(ModiferRef, fCurrentValue))
	{
		fCurrentValue -= fValue;
		TLMaths::Limit(fCurrentValue, TIMESTEP_MIN, TIMESTEP_MAX);
		return SetTimeStepModifier(ModiferRef, fCurrentValue);
	}

	// Failed
	return FALSE;
}




//-------------------------------------------
//	publish a render
//-------------------------------------------
Bool TCoreManager::PublishRenderMessage(Bool bForced)
{
	TLTime::TTimestampMicro TimeNow(TRUE);
	float fTimeStep = GetRenderTimeStepDifference(TimeNow);

	//	no need to process framesteps less than 1 frame
	#ifdef LIMIT_RENDER_RATE
	{
		if(!bForced)
		{
			float fFrameStep = fTimeStep * (float)TLTime::GetRendersPerSecond();
			if ( fFrameStep < 1.0f )
				return FALSE;
		}
	}
	#endif

	if ( bForced )
	{	
		fTimeStep = 1.f / TLTime::GetUpdatesPerSecond();
	}

	//	reset last render timestamp
	m_LastRenderTime = TimeNow;

	//	make up render message to send
	TPtr<TLMessaging::TMessage> pMessage = new TLMessaging::TMessage(RenderRef);
	pMessage->AddChannelID(RenderRef);
	
	//	send out render message
	PublishMessage( pMessage );

	m_Debug_CurrentFramesPerSecond++;

	return TRUE;
}



void TCoreManager::ProcessMessage(TPtr<TLMessaging::TMessage>& pMessage)
{
	if ( pMessage->GetMessageRef() == "Quit" )
	{
		// Quit
		m_bQuit = TRUE;
	}

	TManager::ProcessMessage(pMessage);
}

//-------------------------------------------
// Checks the managers to see if they are in the specified state
//-------------------------------------------
Bool TCoreManager::CheckManagersInState(TLManager::ManagerState State)
{
	u32 uNumberOfManagers = m_Managers.GetSize();
	for(u32 uIndex = 0; uIndex < uNumberOfManagers; uIndex++)
	{
		TPtr<TManager> pManager = m_Managers.ElementAtConst(uIndex);
		if(pManager->GetState() != State)
			return FALSE;
	}

	// All in the required state
	return TRUE;
}


//-------------------------------------------
// Checks the managers to see if any are in the error state
//-------------------------------------------
Bool TCoreManager::CheckManagersInErrorState()
{
	u32 uNumberOfManagers = m_Managers.GetSize();
	for(u32 uIndex = 0; uIndex < uNumberOfManagers; uIndex++)
	{
		const TPtr<TManager>& pManager = m_Managers.ElementAtConst(uIndex);
		if(pManager->GetState() == TLManager::S_Error)
			return TRUE;
	}

	return FALSE;
}

//-------------------------------------------
//
//-------------------------------------------
void TCoreManager::SubscribeAllManagers()
{
	// Subscribe all managers to the Init and Shutdown?
	// Or do this when the managers are registered?
}


//-------------------------------------------
//	work out timestep for update
//-------------------------------------------
float TCoreManager::GetTimeStepDifference(TLTime::TTimestampMicro& LastTimestamp,const TLTime::TTimestampMicro& TimeNow)
{
	//	is first update - reset timer
	if ( !LastTimestamp.IsValid() )
		LastTimestamp.SetTimestampNow();

	//	get millisecs since last update
	//TLTime::TTimestamp LastTimerTime(TRUE);
	const TLTime::TTimestampMicro& LastTimerTime = TimeNow;
	s32 TimeSinceUpdate = LastTimestamp.GetMilliSecondsDiff( LastTimerTime );

	if ( TimeSinceUpdate < 0 )
		TimeSinceUpdate = 0;

	//	get time diff in seconds
	float Timestep = ((float)TimeSinceUpdate) / 1000.0f;
	return Timestep;
}


void TCoreManager::AddTimeStep(const TLTime::TTimestamp& UpdateTimerTime)	
{
	if ( !IsEnabled() )
		return;
	
	//	if this gets excessive, ditch old timestamps.
	if ( m_UpdateTimeQueue.GetSize() > 10 )
		m_UpdateTimeQueue.RemoveAt( 0 );

	m_UpdateTimeQueue.Add( UpdateTimerTime ); 
}


