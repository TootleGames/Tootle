/*------------------------------------------------------
	
	Platform specific time stuff

-------------------------------------------------------*/
#pragma once

#ifndef _MSC_EXTENSIONS
	#error PC file should not be included in ansi builds
#endif

#include "../TLTime.h"
#include <TootleGui/PC/PCGui.h>	//	windows headers


namespace TLTime
{
	namespace Platform
	{
		SyncBool	Init();										//	record bootup timestamp
		
		void		GetTimeNow(TTimestamp& Timestamp);			//	get timestamp for right now!
		void		GetMicroTimeNow(TTimestampMicro& Timestamp);			//	get timestamp for right now!
		void		GetTimestamp(TTimestamp& Timestamp,const SYSTEMTIME& SystemTime);	//	convert system time to timestamp
		void		GetTimestamp(TTimestamp& Timestamp,const FILETIME& FileTime);		//	convert file time to timestamp
		void		GetTimestampFromTickCount(TTimestamp& Timestamp,u32 TickCount);		//	convert win32 tick count to timestamp
	}
}


