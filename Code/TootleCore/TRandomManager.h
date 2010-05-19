/*
 *  TRandomManager.h
 *  TootleCore
 *
 *	The random number manager currently simply sets a global random seed
 *
 *	The random number manager will eventually be a container for multiple random number
 *	generators that can use a variety of different random number generation techniques.
 *	
 *	The current random number generator is based on the time and therefore non-determinsic.  
 *	This is fine for any system where the randomness is irrelevent to the gameplay 
 *	i.e graphical random effects such as random rotations of particles 
 *
 *	This doesn't work well for multiplayer games because the lack of determinism usually means having to send around to 
 *	synchronise the random numbers which consumes bandwidth.
 *
 *	Additionally single player games that use the random number as part of the gameplay and need to be deterministic
 *	require the use of a repeting pattern that isn't based on time, i.e. elite planet system
 * 
 *	For non-trivial non-deterministic random numbers the use of the standard Rand() will suffice
 *	but for everything else should create a random number generator on the manager and only use that generator
 *	to obtain the next random number.  The generator can use any technique it wishes for updating the random number
 *
 *  Created by Duane Bradbury on 15/05/2010.
 *  Copyright 2010 Tootle. All rights reserved.
 *
 */

#pragma once

#include "TManager.h"


namespace TLRandom
{
	class TRandomNumberManager;
}


class TLRandom::TRandomNumberManager : public TLCore::TManager
{
public:
	TRandomNumberManager(TRefRef ManagerRef);
	
protected:
	
	virtual SyncBool Initialise();	
};