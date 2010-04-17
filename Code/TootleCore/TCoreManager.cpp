#include "TCoreManager.h"
#include "TEventChannel.h"
#include "TSyncQueue.h"

#include "TLMaths.h"
#include "TLTime.h"
#include "TLanguage.h"
#include "TRef.h"

#include "TLCore.h"

//#define ENABLE_TIMER_HITS_OUTPUT


#pragma message("todo: gr: replace this with a gui manager. BUT remember, the GUI lib is now the first to startup...")
namespace TLGui
{
	extern SyncBool Init();
	extern SyncBool Shutdown();
}

using namespace TLCore;

// DB - quick define for the maximum and minimum time step modifiers we can reach
#define TIMESTEP_MIN 0.0f
#define TIMESTEP_MAX 3.0f

#if defined(TL_TARGET_IPOD)
	#define LIMIT_UPDATE_RATE
#elif defined(TL_TARGET_PC)
	#define LIMIT_UPDATE_RATE
#endif

#define STALL_UPDATE_RATE	//	if enabled stalls updates with sleep until we're the right amount of time since last one. if not defined, skips frame and waits till next timer

//#define DEBUG_RECORD_SUBSCRIBER_UPDATE_TIMES


TCoreManager::TCoreManager(TRefRef ManagerRef) :
	TManager							( ManagerRef ),
	m_Debug_FramesPerSecond				( 0 ),
	m_Debug_CurrentFramesPerSecond		( 0 ),
	m_Debug_FrameTimePerSecond			( 0.f ),
	m_Debug_CurrentFrameTimePerSecond	( 0.f ),
	m_ChannelsInitialised				( FALSE ),
	m_bEnabled							( TRUE ),
	m_bQuit								( FALSE ),
	m_TimerUpdateCount					( 0 ),
	m_MachineData						( "Machine" )
{
	//	gr: I've turned this off... they use memory routines before we've created a memory manager...
/*
	// Query the hardware for various hardware and OS specific data as soon as we create the core manager.
	// Some of this data we may need in advance of creating any other managers.
	// Query the device specific information - UDID, OS and version etc
	TLCore::Platform::QueryHardwareInformation( m_MachineData );
	
	// Query user selected data on the device - languages and other settings
	TLCore::Platform::QueryLanguageInformation( m_MachineData );
	*/
}

TCoreManager::~TCoreManager()
{
	UnregisterAllManagers();
}


//---------------------------------------------------------
//	remove manager from list
//---------------------------------------------------------
Bool TLCore::TCoreManager::UnregisterManager(TRefRef ManagerRef)
{
	//	get index
	s32 Index = m_Managers.FindIndex( ManagerRef );
	if ( !Index )
		return FALSE;

	//	see if we have a [global] pointer to this manager we need to null
	//	gr: cant use FindIndex because we'd have to match pointers to the TPtr
	//		and because we may have cast it to a TPtr<Manager> from another TPtr<TYPE>
	//		in register manager, the address of the TPtr* may not be the same as the 
	//		TPtr stored in our array. Especially not if the array has been reallocated
	for ( u32 i=0;	i<m_ManagerPointers.GetSize();	i++ )
	{
		TPtr<TManager>& pManagerPointer = *m_ManagerPointers[i];
		if ( !pManagerPointer )
		{
			TLDebug_Print("manager pointer expected");
			continue;
		}

		//	match
		if ( pManagerPointer->GetManagerRef() != ManagerRef )
			continue;

		//	release global pointer
		pManagerPointer = NULL;
	}

	//	null pointer to (hopefully) release manager
	TPtr<TManager>& pManager = m_Managers[Index];
	pManager = NULL;

	//	remove (now null ptr) from list
	m_Managers.RemoveAt(Index);

	return TRUE;
}



void TCoreManager::UnregisterAllManagers()				
{
	s32 sIndex = 0;
	s32 sNumberOfManagers = (s32)m_ManagerPointers.GetSize();

	// Go through the pointers to TPtr's and set them to null
	for(sIndex = 0; sIndex < sNumberOfManagers; sIndex++)
	{
		TPtr<TManager>& pPtr = *m_ManagerPointers.ElementAt(sIndex);
		pPtr = NULL;
	}

	m_ManagerPointers.Empty(TRUE);


	sNumberOfManagers = m_Managers.GetLastIndex();
	// Go through the list of managers and null the TPtr's in reverse order
	for(sIndex = sNumberOfManagers; sIndex >= 0; sIndex--)
	{
		m_Managers.ElementAt(sIndex) = NULL;
	}

	m_Managers.Empty(TRUE);


}



SyncBool TCoreManager::Initialise()
{
	//	generate the TRef lookup table
	TLRef::GenerateCharLookupTable();
	
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
			//	reset last-update timestamp
			m_LastUpdateTime.SetTimestampNow();
			
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
		TLMessaging::g_pEventChannelManager->RegisterEventChannel(this, GetManagerRef(), InitialiseRef);
		TLMessaging::g_pEventChannelManager->RegisterEventChannel(this, GetManagerRef(), UpdateRef);
		TLMessaging::g_pEventChannelManager->RegisterEventChannel(this, GetManagerRef(), RenderRef);
		TLMessaging::g_pEventChannelManager->RegisterEventChannel(this, GetManagerRef(), ShutdownRef);

		
		// re-order the shutdown channel so that managers are sent messages in reverse order
		TLMessaging::g_pEventChannelManager->SetPublishOrder(TLArray::Descending, GetManagerRef(), ShutdownRef);
		
		TLGui::Init();

		//	other non complex one-off init's
		TLMaths::Init();

		m_ChannelsInitialised = TRUE;
	}

	// Initialisation of all other managers
	// NOTE: May be able to move this into the TootMain routine which also does the
	//		 Initialisation of the core manager.   Maybe even add the core manager to itself?

	//gr: core manager wasn't being initialised, or checked for it's error state. Fix this
	this->Initialise();

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


// Returns a TRef version of the language that was stored when queried by the hardware
// The TRef will be appropriate for our engine and can be any language TRef - no language filtering is doen via this routine, 
// tha callee (gr: caller. this funcion IS the callee) is responsible for determining whether the language selected by the hardware is appropriately supported.
TRef TCoreManager::GetHardwareLanguage()
{
	TRef LanguageRef = TLLanguage_RefEnglish;

	m_MachineData.ImportData("Language", LanguageRef);
	
	return LanguageRef;
}


//-----------------------------------------------------
// one process of Main update loop
//-----------------------------------------------------
SyncBool TCoreManager::UpdateLoop()
{
	// Send the update and render message
	// Not required when using multithreading?
	//TLTime::TScopeTimer Timer("loop");

	// Main update loop phase
	u32 TimerUpdateCount = m_TimerUpdateCount;
	
	//	no timer hits
	if ( TimerUpdateCount == 0 )
	{
		//	update the per-second counters
		Debug_UpdateDebugCounters();
		return SyncWait;
	}
	
	//	always do an update and see if we skipped it
	Bool DidUpdate = PublishUpdateMessage();

	//	only render if we did an update
	if ( DidUpdate )
		PublishRenderMessage();

	// Process the message queue independant of update - this is to get network messages maybe? other "interrupt" style stuff 
	ProcessMessageQueue();

	if ( DidUpdate )
	{
		//	increment frame counter
		m_Debug_CurrentFramesPerSecond++;

		//	covered these timer hits
		m_TimerUpdateCount -= TimerUpdateCount;	

#ifdef ENABLE_TIMER_HITS_OUTPUT
		if ( m_TimerUpdateCount > 0 )
		{
		//	TLDebug_Print( TString("%d timer hits during update/render", m_TimerUpdateCount ) );
		}
#endif


		//	error with manager[s]
		//	gr: assume no managers will fail if we did no update
		if ( CheckManagersInErrorState() )
			return SyncFalse;
	}

	//	if quit, return positive result
	if ( m_bQuit )
		return SyncTrue;

	//	update the per-second counters
	Debug_UpdateDebugCounters();

	//	still updating
	return SyncWait;
}


//-----------------------------------------------------
//	
//-----------------------------------------------------
void TCoreManager::Debug_UpdateDebugCounters()
{
	//	update fps
	TLTime::TTimestamp TimeNow(TRUE);
	if ( !m_Debug_LastCountTime.IsValid() || m_Debug_LastCountTime.GetMilliSecondsDiff(TimeNow) > 1000 )
	{
		m_Debug_LastCountTime = TimeNow;

		//	do the global timer elapses
		TLTime::OnSecondElapsed( m_Debug_CurrentFramesPerSecond );
		TLCounter::OnSecondElapsed( m_Debug_CurrentFramesPerSecond );

		m_Debug_FramesPerSecond			= m_Debug_CurrentFramesPerSecond;
		m_Debug_CurrentFramesPerSecond	= 0;
		
		m_Debug_FrameTimePerSecond			= m_Debug_CurrentFrameTimePerSecond;
		m_Debug_CurrentFrameTimePerSecond	= 0.f;

	}
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

	//	gr: will probbaly move this out of the core manager's responsiblity now
	TLGui::Shutdown();

	return SyncTrue;
}


//-------------------------------------------
//	publish an init
//-------------------------------------------
void TCoreManager::PublishInitMessage()
{
	TLMessaging::TMessage Message(InitialiseRef);
	
	PublishMessage( Message );
}


//-------------------------------------------
//	publish a shutdown
//-------------------------------------------
void TCoreManager::PublishShutdownMessage()
{
	TLMessaging::TMessage Message(ShutdownRef);
	
	PublishMessage( Message );
}


//-------------------------------------------
//	publish an update
//-------------------------------------------
Bool TCoreManager::PublishUpdateMessage(Bool bForced)
{
	TLTime::TTimestampMicro TimeNow(TRUE);
	float fTimeStep = bForced ? (1.f/TLTime::GetUpdatesPerSecondf()) : GetUpdateTimeStepDifference(TimeNow);

	//	no need to process framesteps less than 1 frame
	#ifdef LIMIT_UPDATE_RATE
	{
		if(!bForced)
		{
			float fFrameStep = fTimeStep * TLTime::GetUpdatesPerSecondf();
			u32 Debug_SleepCounter = 0;
			while ( fFrameStep < 1.0f )
			{
#ifdef STALL_UPDATE_RATE
				//	stall by the time it'll take until we're ready for a frame
				float TimeTillUpdate = (1.f / TLTime::GetUpdatesPerSecondf()) - fTimeStep;
				u32 TimeTillUpdateMs = (u32)( TimeTillUpdate * 1000.f );
				TLCore::Platform::Sleep( TimeTillUpdateMs );
				Debug_SleepCounter++;

				//	re-evaluate time
				TimeNow.SetTimestampNow();
				fTimeStep = GetUpdateTimeStepDifference(TimeNow);
				fFrameStep = fTimeStep * TLTime::GetUpdatesPerSecondf();
#else // STALL_UPDATE_RATE
				//	dont stall, skip this frame as it's less than a frame
				return FALSE;
#endif // STALL_UPDATE_RATE
			}

			if ( Debug_SleepCounter > 1 )
			{
		//		TLDebug_Break("Expected sleep counter to only occur once");
			}
		}
	}
	#endif

	//	reset last update timestamp
	m_LastUpdateTime = TimeNow;

	//	create an update message
	TLMessaging::TMessage Message( TLCore::UpdateRef );

	//	add timestep data
//	Message.ExportData( TLCore::TimeStepRef, fTimeStep );
	Message.Write( fTimeStep );

	//	add timestep modifier data
	float fModifier = GetTimeStepModifier();
//	Message.ExportData( TLCore::TimeStepModRef, fModifier );
	Message.Write( fModifier );

	//	track CPU time of update
	TLTime::TScopeTimer Timer( TRef_Static(U,p,d,a,t) );

	//	send message
	PublishMessage( Message );

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
void TCoreManager::PublishRenderMessage()
{
	//	make up render message to send
	TLMessaging::TMessage Message(RenderRef);

	//	send message
	PublishMessage( Message );
}



void TCoreManager::ProcessMessage(TLMessaging::TMessage& Message)
{
	if ( Message.GetMessageRef() == TLCore::QuitRef )
	{
		// Quit
		m_bQuit = TRUE;
	}

	TManager::ProcessMessage(Message);
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
		{
			TLDebug_Print("Manager is in error state");
			TString ManagerName;
			pManager->GetManagerRef().GetString(ManagerName);
			TLDebug_Print(ManagerName);
			return TRUE;
		}
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


Bool TCoreManager::SendMessageTo(TRefRef RecipientRef, TRefRef ManagerRef, TLMessaging::TMessage& Message)
{
	if(!ManagerRef.IsValid())
	{
		TLDebug_Break("Trying to send a message with an invalid manager ref");
		return FALSE;
	}

	// Go through the list of managers and find the right one to send the message via.
	// If the Sender is invalid or the same as the managerref then we are sending the message
	// to the manager itself.
	TPtr<TManager> pManager = m_Managers.FindPtr(ManagerRef);

	if(pManager)
	{
		if(!pManager->SendMessageTo(RecipientRef, Message))
		{
			// Failed
			TLDebug_Break("Failed to send message to recipient");
			return FALSE;
		}

		// Success
		return TRUE;
	}
	else
	{
		TTempString str("Coremanager unable to find manager: ");
		ManagerRef.GetString(str); 
		TLDebug_Print(str);
		TLDebug_Print("Unable to send message");
	}

	return FALSE;
}

