/*
 *  IPadTime.mm
 *  TootleCore
 *
 *  Created by Duane Bradbury on 15/12/2009.
 *  Copyright 2009 Tootle. All rights reserved.
 *
 */

#include "IPadTime.h"
#include <sys/time.h>

#include "../TCoreManager.h"

namespace TLTime
{
	namespace Platform
	{
	}
}


SyncBool TLTime::Platform::Init()
{
	TLTime::TTimestampMicro StartTime(TRUE);
	
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
	struct timeval TimeNow = { 0, 0 };
	struct timezone TimeZone = { 0, 1 };	//	no GMT offset
	
	//	zero == success
	if ( gettimeofday(&TimeNow, &TimeZone) != 0 )
	{
		TLDebug_Break("Failed to get time");
	}
	
	Timestamp.SetEpochSeconds( TimeNow.tv_sec );
	
	u32 MilliSeconds = (TimeNow.tv_usec) / 1000;
	Timestamp.SetMilliSeconds( MilliSeconds );
}


//-----------------------------------------------------------------------
//	get the current time
//	get the difference in milliseconds of now against when we booted the app
//	add that onto the bootup date to get the current date (to the nearest second) 
//	+ additional milliseconds
//-----------------------------------------------------------------------
void TLTime::Platform::GetMicroTimeNow(TTimestampMicro& Timestamp)
{
	struct timeval TimeNow = { 0, 0 };
	struct timezone TimeZone = { 0, 1 };	//	no GMT offset

	//	zero == success
	if ( gettimeofday(&TimeNow, &TimeZone) != 0 )
	{
		TLDebug_Break("Failed to get time");
	}
	
	Timestamp.SetEpochSeconds( TimeNow.tv_sec );
	
	u32 MicroSeconds = TimeNow.tv_usec % 1000;
	u32 MilliSeconds = (TimeNow.tv_usec - MicroSeconds) / 1000;
	Timestamp.SetMilliSeconds( MilliSeconds );
	Timestamp.SetMicroSeconds( MicroSeconds );
}



//-----------------------------------------------------------------------
//	
//-----------------------------------------------------------------------
void TLTime::Platform::Debug_PrintTimestamp(const TTimestamp& Timestamp,s32 Micro)
{
}


