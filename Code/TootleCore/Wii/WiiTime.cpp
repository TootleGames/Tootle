#include "WiiTime.h"
#include "WiiCore.h"
#include <ctime>
#include "../TString.h"

#include "../TCoreManager.h"

namespace TLTime
{
	namespace Platform
	{
		u32					g_BootupDate = 0;			//	base date so we don't need to fetch time() every time we want to get the current timestamp
		u32					g_BootupMilliSeconds = 0;	//	timeGetTime() at bootup
		float				g_MicroSecondsPerClock = 0;	//	when using performance counter this is how many Micro seconds each counter is worth. if 0 high performance not supported
		
		Bool				IsMicroClockSupported()		{	return (g_MicroSecondsPerClock != 0);	}

		void				Debug_PrintTimestamp(const TTimestamp& Timestamp,s32 Micro);
	}
}


SyncBool TLTime::Platform::Init()
{
	TLTime::TTimestampMicro StartTime;

		//	unsupported
		g_MicroSecondsPerClock = 0;

	//	get initial times to base all our other timestamps again
	g_BootupDate = (u32)time(0);
	//	init bootup timestamp
	if ( IsMicroClockSupported() )
	{
		//GetTimestampFromTickCount( StartTime, g_BootupMilliSeconds );
	}
	else
	{
		//GetTimestampFromClockCount( StartTime, g_BootupClockCount );
	}

	//	the bootup timestamp deducts bootup millisecs, which is right, but we still want those remaining millisecs in
	//	our bootup timestamp
	if ( IsMicroClockSupported() )
	{
		u32 Secs;
		u16 MilliSecs,MicroSecs;
		//GetSecsFromClockCount( g_BootupClockCount, Secs, MilliSecs, MicroSecs );
		StartTime.SetMilliSeconds( MilliSecs );
		StartTime.SetMicroSeconds( MilliSecs );
	}
	else
	{
		StartTime.SetMilliSeconds( g_BootupMilliSeconds %1000 );
		StartTime.SetMicroSeconds( 0 );
	}

	TLDebug_Print("Startup timestamp:");
	Debug_PrintTimestamp( StartTime );

	TLCore::g_pCoreManager->StoreTimestamp("TSStartTime", StartTime);

	return SyncTrue;
}


//-----------------------------------------------------------------------
//	get the current time
//	get the difference in milliseconds of now against when we booted the app
//	add that onto the bootup date to get the current date (to the nearest second) 
//	+ additional milliseconds
//-----------------------------------------------------------------------
void TLTime::Platform::GetTimeNow(TTimestamp& Timestamp)
{
	u32 NowMilliSeconds = 0;	
	//GetTimestampFromTickCount( Timestamp, NowMilliSeconds );
}


//-----------------------------------------------------------------------
//	get the current time
//	get the difference in milliseconds of now against when we booted the app
//	add that onto the bootup date to get the current date (to the nearest second) 
//	+ additional milliseconds
//-----------------------------------------------------------------------
void TLTime::Platform::GetMicroTimeNow(TTimestampMicro& Timestamp)
{
	if ( !IsMicroClockSupported() )
	{
		GetTimeNow( Timestamp );
		Timestamp.SetMicroSeconds(0);
		return;
	}

	//	get current counter
	return;
}







		


void TLTime::Platform::Debug_PrintTimestamp(const TTimestamp& Timestamp,s32 Micro)
{
	TTempString DebugString("Invalid Timestamp");

	TLDebug_Print( DebugString );
}

