/*------------------------------------------------------
	
	Graham doesn't know why this is a manager and not
	just some maths static Init function called from
	the core/bootup Init...

-------------------------------------------------------*/
#pragma once

#include "TManager.h"
#include "TLMaths.h"
#include "TLTime.h"

namespace TLRandom
{
	class TRandomNumberManager;
}


class TLRandom::TRandomNumberManager : public TLCore::TManager
{
public:
	TRandomNumberManager(TRefRef ManagerRef) :
		TLCore::TManager(ManagerRef)
	{
	}

protected:

	virtual SyncBool Initialise()
	{
		//	init random seed
		TLTime::TTimestamp TimeNow(TRUE);
		srand( TimeNow.GetTotalMilliSeconds() );

		return SyncTrue;
	}

};