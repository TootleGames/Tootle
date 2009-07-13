

#pragma once

#include "TManager.h"
#include "TLMaths.h"
#include "TLTime.h"

namespace TLRandom
{
	class TRandomNumberManager;
}


class TLRandom::TRandomNumberManager : public TManager
{
public:
	TRandomNumberManager(TRefRef ManagerRef) :
	  TManager(ManagerRef)
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