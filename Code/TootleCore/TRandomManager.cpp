/*
 *  TRandomManager.cpp
 *  TootleCore
 *
 *  Created by Duane Bradbury on 15/05/2010.
 *  Copyright 2010 Tootle. All rights reserved.
 *
 */

#include "TRandomManager.h"

#include "TLTime.h"
#include "TLRandom.h"

using namespace TLRandom;

TRandomNumberManager::TRandomNumberManager(TRefRef ManagerRef) :
TLCore::TManager(ManagerRef)
{
}

SyncBool TRandomNumberManager::Initialise()
{
	//	init random seed
	TLTime::TTimestamp TimeNow(TRUE);
	TLMaths::SRand( TimeNow.GetTotalMilliSeconds() );
	
	return SyncTrue;
}
