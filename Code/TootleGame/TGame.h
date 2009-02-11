/*
 *  TGame.h
 *  TootleGame
 *
 *	Game class delegate of an appliction
 *
 *  Created by Duane Bradbury on 06/02/2009.
 *  Copyright 2009 Tootle. All rights reserved.
 *
 */

#pragma once

#include <TootleCore/TManager.h>
#include <TootleCore/TStateMachine.h>

namespace TLGame
{
	class TGame;
}


// NOTE: Not sure whether we can make this a manager that is created and removed during the applications lifetime?  
// It would mean we can subscribe to it and provide some basic init, udpate and shutdown routines...
class TLGame::TGame : public TManager, public TStateMachine
{
public:
	TGame(TRef refManagerID) :
		TManager(refManagerID)
	{
	}
	

protected:
	virtual SyncBool Update(float fTimeStep);
	
};

