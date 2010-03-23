#include "PCTime.h"
#include <mmsystem.h>
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
		
		LARGE_INTEGER		g_BootupClockCount;			//	bootup result from performance counter

		Bool				IsMicroClockSupported()		{	return (g_MicroSecondsPerClock != 0);	}

		void				Debug_PrintTimestamp(const TTimestamp& Timestamp,s32 Micro);
		void				GetTimestampFromClockCount(TTimestampMicro& Timestamp,const LARGE_INTEGER& ClockCount);
		Bool				GetSecsFromClockCount(const LARGE_INTEGER& ClockCount,u32& Secs,u16& MilliSecs,u16& MicroSecs);
	}
}


SyncBool TLTime::Platform::Init()
{
	TLTime::TTimestampMicro StartTime;

	//	see if performance counter is supported
	LARGE_INTEGER Frequency;

	if ( QueryPerformanceFrequency( &Frequency ) )
	{
		if ( Frequency.HighPart > 0 )
		{
			TLDebug_Break("unhandled values from high performance counter");
		}
		else
		{
			g_MicroSecondsPerClock = (float)SecsToMicroSecs(1) / (float)Frequency.QuadPart;
		}

		//	get initial counter
		if ( !QueryPerformanceCounter( &g_BootupClockCount ) )
			g_MicroSecondsPerClock = 0;
	}
	else
	{
		//	unsupported
		g_MicroSecondsPerClock = 0;
	}

	//	get initial times to base all our other timestamps again
	g_BootupDate = (u32)time(0);
	g_BootupMilliSeconds = timeGetTime();
	QueryPerformanceCounter( &g_BootupClockCount );

	//	init bootup timestamp
	if ( IsMicroClockSupported() )
	{
		GetTimestampFromTickCount( StartTime, g_BootupMilliSeconds );
	}
	else
	{
		GetTimestampFromClockCount( StartTime, g_BootupClockCount );
	}

	//	the bootup timestamp deducts bootup millisecs, which is right, but we still want those remaining millisecs in
	//	our bootup timestamp
	if ( IsMicroClockSupported() )
	{
		u32 Secs;
		u16 MilliSecs,MicroSecs;
		GetSecsFromClockCount( g_BootupClockCount, Secs, MilliSecs, MicroSecs );
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
	u32 NowMilliSeconds = timeGetTime();
	
	GetTimestampFromTickCount( Timestamp, NowMilliSeconds );
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
	LARGE_INTEGER CounterNow;
	if ( !QueryPerformanceCounter( &CounterNow ) )
	{
		TLDebug_Break("Query performance counter no longer working?");
		GetTimeNow( Timestamp );
		Timestamp.SetMicroSeconds(0);
		return;
	}

	GetTimestampFromClockCount( Timestamp, CounterNow );

	return;
}



//-----------------------------------------------------------------------
//	convert system time to timestamp
//-----------------------------------------------------------------------
void TLTime::Platform::GetTimestamp(TTimestamp& Timestamp,const SYSTEMTIME& SystemTime)
{
	//	convert to filetime, then convert to timestamp
	FILETIME FileTime;
    if ( !SystemTimeToFileTime( &SystemTime, &FileTime ) )
	{
		TLDebug_Break("Failed to convert win32 timestamp");
		Timestamp.SetInvalid();
		return;
	}

	//	now convert to timestamp
	GetTimestamp( Timestamp, FileTime );
}


//-----------------------------------------------------------------------
//	convert file time to timestamp
//-----------------------------------------------------------------------
void TLTime::Platform::GetTimestamp(TTimestamp& Timestamp,const FILETIME& FileTime)
{
	//	calculate the epoch time from a system time
	SYSTEMTIME EpochSysTime = { 1970, 1, 0, 1, 0, 0, 0, 0 };
	FILETIME EpochFileTime;
	if ( !SystemTimeToFileTime( &EpochSysTime, &EpochFileTime ) )
	{
		TLDebug_Break("Failed to convert win32 timestamp");
		Timestamp.SetInvalid();
		return;
	}

	ULARGE_INTEGER LargeInt;

	//	deduct epoch time
	LargeInt.QuadPart  = ((ULARGE_INTEGER *)&FileTime)->QuadPart;
	LargeInt.QuadPart -= ((ULARGE_INTEGER *)&EpochFileTime)->QuadPart;

	//	turn from 100millisecs to secs(i think, something like that)
	u64 clunks_per_second = 10000000L;
	LargeInt.QuadPart /= clunks_per_second;

	//	set timestamp
	Timestamp.SetEpochSeconds( LargeInt.LowPart );
	Timestamp.SetMilliSeconds( 0 );
}


//-----------------------------------------------------------------------
//	convert win32 tick count to timestamp
//-----------------------------------------------------------------------
void TLTime::Platform::GetTimestampFromTickCount(TTimestamp& Timestamp,u32 TickCount)
{
	//	bootup date cannot possibly be zero (unless its 1970) means time lib hasn't been initialised
	if ( g_BootupDate == 0 )
	{
		if ( !TLDebug_Break("Time lib hasn't been initialised") )
			return;
	}

	//	get difference since bootup
	TickCount -= g_BootupMilliSeconds;

	//	adjust date
	u32 NowDate = g_BootupDate + (TickCount/1000);

	//	set timestamp
	Timestamp.SetEpochSeconds( NowDate );
	Timestamp.SetMilliSeconds( TickCount % 1000 );
}


//-----------------------------------------------------------------------
//	convert clock count to timestamp
//-----------------------------------------------------------------------
void TLTime::Platform::GetTimestampFromClockCount(TTimestampMicro& Timestamp,const LARGE_INTEGER& ClockCount)
{
	//	bootup date cannot possibly be zero (unless its 1970) means time lib hasn't been initialised
	if ( g_BootupDate == 0 )
	{
		if ( !TLDebug_Break("Time lib hasn't been initialised") )
			return;
	}

	//	get difference since bootup
	LARGE_INTEGER ClocksSinceBootup;
	ClocksSinceBootup.QuadPart = ClockCount.QuadPart - g_BootupClockCount.QuadPart;

	//	
	u32 Secs;
	u16 MilliSecs,MicroSecs;
	GetSecsFromClockCount( ClocksSinceBootup, Secs, MilliSecs, MicroSecs );

	//	set timestamp
	Timestamp.SetEpochSeconds( g_BootupDate + Secs );
	Timestamp.SetMilliSeconds( MilliSecs );
	Timestamp.SetMicroSeconds( MicroSecs );
}

		
Bool TLTime::Platform::GetSecsFromClockCount(const LARGE_INTEGER& ClockCount,u32& Secs,u16& MilliSecs,u16& MicroSecs)
{
	//	high performance not supported
	if ( g_MicroSecondsPerClock == 0 )
		return FALSE;

	//	convert to micro secs
	u64 TotalMicroSecs = (u64)( (float)( ClockCount.QuadPart * g_MicroSecondsPerClock ) );
	MicroSecs = (u16)( TotalMicroSecs % 1000 );

	u64 TotalMilliSecs = (TotalMicroSecs - MicroSecs)/1000;
	MilliSecs = (u16)( TotalMilliSecs % 1000 );

	Secs = (u32)((TotalMilliSecs - MilliSecs ) / 1000);

	return TRUE;
}


void TLTime::Platform::Debug_PrintTimestamp(const TTimestamp& Timestamp,s32 Micro)
{
	time_t t = Timestamp.GetEpochSeconds();
	FILETIME ft;

	// Note that LONGLONG is a 64-bit value
	LONGLONG ll;

	ll = Int32x32To64(t, 10000000) + 116444736000000000;
	ft.dwLowDateTime = (DWORD)ll;
	ft.dwHighDateTime = ll >> 32;

	SYSTEMTIME st;

	FileTimeToSystemTime(&ft, &st);

	TTempString TimeString;
	if ( Micro > -1 )
		TimeString.Appendf("%d::%d::%d %d ms %d us", st.wHour, st.wMinute, st.wSecond, Timestamp.GetMilliSeconds(), Micro );
	else
		TimeString.Appendf("%d::%d::%d (%d ms)", st.wHour, st.wMinute, st.wSecond, Timestamp.GetMilliSeconds() );

	TTempString DateString;
	DateString.Appendf("%dth %d, %d", st.wDay, st.wMonth, st.wYear );

	TTempString DebugString("Timestamp: ");
	DebugString.Append( DateString );
	DebugString.Append(" ");
	DebugString.Append( TimeString );

	TLDebug_Print( DebugString );
}
