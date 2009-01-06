/*------------------------------------------------------
	Core CPP... required to compile the headers



------------------------------------------------------*/
#include "TLCore.h"
#include "TSyncQueue.h"
#include "TLTime.h"
#include "TCoreManager.h"
#include "TPtr.h"
#include "TEventChannel.h"
#include "TRefManager.h"
#include "TMemoryManager.h"


namespace TLCore
{
	TPtr<TCoreManager>			g_pCoreManager;
	TLTime::TTimestamp			g_LastUpdateTime;	//	time on last update

	void RegisterManagers_Engine(TPtr<TCoreManager>& pCoreManager);
	void RegisterManagers_Game(TPtr<TCoreManager>& pCoreManager);

	void UnregisterManagers_Engine();
	void UnregisterManagers_Game();
}


//---------------------------------------------------
//	convert a pointer to an integer
//---------------------------------------------------
u32 TLCore::PointerToInteger(void* pAddress)
{
	u32 Address = 0;

	//	ignore pointer truncation warning (void* to integer)
#pragma warning(push)
#pragma warning(disable : 4311) 
	Address = reinterpret_cast<u32>( pAddress );
#pragma warning(pop)

	return Address;
}

void TLCore::RegisterManagers_Engine(TPtr<TCoreManager>& pManager)
{
	TLDebug_Print("Registering Engine Managers");
	
	pManager->CreateAndRegisterManager<TLRef::TRefManager>("REFMANAGER");	// NOTE: No global poitner for this manager
	pManager->CreateAndRegisterManager<TLMessaging::TEventChannelManager>(TLMessaging::g_pEventChannelManager, "EVENTCHANNEL");
	pManager->CreateAndRegisterManager<TLTime::TTimeManager>("TIMEMANAGER");	// NOTE: No global poitner for this manager
}

// Unset all global pointers - engine side
void TLCore::UnregisterManagers_Engine()
{
	TLDebug_Print("Unregistering Engine Managers");
	
	TLMessaging::g_pEventChannelManager = NULL;
}



//---------------------------------------------------
//	win32 entry
//---------------------------------------------------
Bool TLCore::TootMain()
{
	g_pCoreManager = new TCoreManager("CORE");

	// Register the engine managers
	RegisterManagers_Engine(g_pCoreManager);
	RegisterManagers_Game(g_pCoreManager);

	//	do init loop
	SyncBool InitLoopResult = SyncWait;
	while ( InitLoopResult == SyncWait )
	{
		InitLoopResult = g_pCoreManager->InitialiseLoop();
	} 
	
	//	init was okay, do update loop
	SyncBool UpdateLoopResult = (InitLoopResult == SyncTrue) ? SyncWait : SyncFalse;
	while ( UpdateLoopResult == SyncWait )
	{
		// If enabled go through the update loop
		if(g_pCoreManager->IsEnabled())
			UpdateLoopResult = g_pCoreManager->UpdateLoop();
	}

	//	shutdown loop
	SyncBool ShutdownLoopResult = SyncWait;
	while ( ShutdownLoopResult == SyncWait )
	{
		ShutdownLoopResult = g_pCoreManager->UpdateShutdown();
	}

	UnregisterManagers_Engine();
	UnregisterManagers_Game();

	//	destroy core manager
	g_pCoreManager = NULL;

	return (ShutdownLoopResult == SyncTrue);
}




/*

SyncBool TLCore::Update()
{
	//	need to do some updates?
//	if ( g_UpdateTimeQueue.GetSize() == 0 )
//		return SyncWait;

	//	is first update - reset timer (update time should be 0.f here)
	if ( !g_LastUpdateTime.IsValid() )
		g_LastUpdateTime.SetTimestampNow();

	//	accumulate the amount of millisecs since our last update
	//TLTime::TTimestamp& LastTimerTime = g_UpdateTimeQueue.ElementLast();
	TLTime::TTimestamp LastTimerTime(TRUE);

	//	get last timer time difference
	s32 TimeSinceUpdate = g_LastUpdateTime.GetTotalMilliSecondsDiff( LastTimerTime );
	if ( TimeSinceUpdate < 0 )
	{
		//TimeSinceUpdate = -TimeSinceUpdate;
		TimeSinceUpdate = 0;
	}

	//	turn time since update into 1/60th of a second
	float Timestep = ((float)TimeSinceUpdate) / (1000.f / (float)TLTime::GetUpdatesPerSecond());

	//	skip unneccasasry updates
	if ( Timestep < 1.0f )
		return SyncWait;

	//	publish an update
	g_pCorePublisher->PublishUpdateMessage(Timestep);

	//	publish a render
	g_pCorePublisher->PublishRenderMessage();

	//	reset last-update time
	g_LastUpdateTime.SetTimestampNow();
	g_UpdateTimeQueue.Empty();

	// Check to see if the app has been flagged to quit.  If so return true to say we have finished - otherwise return wait
	return SyncWait;
}

//-------------------------------------------
//	core lib init/shutdown
//-------------------------------------------
SyncBool TLCore::Shutdown()
{
	g_pCorePublisher = NULL;

	return SyncTrue;
}

*/
