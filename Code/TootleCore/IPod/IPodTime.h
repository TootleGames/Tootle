/*------------------------------------------------------
 
 Ipod core header
 
 -------------------------------------------------------*/
#pragma once

#include "../TLTime.h"
//#include "IPodCore.h"


namespace TLTime
{
	namespace Platform
	{
		SyncBool	Init();												//	record bootup timestamp
		
		void		GetTimeNow(TTimestamp& Timestamp);					//	get timestamp for right now!
		void		GetMicroTimeNow(TTimestampMicro& Timestamp);		//	get timestamp for right now!
		void		Debug_PrintTimestamp(const TTimestamp& Timestamp,s32 Micro);
	}
}

