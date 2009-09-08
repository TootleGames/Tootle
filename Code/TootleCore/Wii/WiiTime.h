/*------------------------------------------------------
	
	Platform specific time stuff

-------------------------------------------------------*/
#pragma once


#include "../TLTime.h"
#include "WiiCore.h"


namespace TLTime
{
	namespace Platform
	{
		SyncBool	Init();										//	record bootup timestamp
		
		void		GetTimeNow(TTimestamp& Timestamp);			//	get timestamp for right now!
		void		GetMicroTimeNow(TTimestampMicro& Timestamp);			//	get timestamp for right now!
		}
}

