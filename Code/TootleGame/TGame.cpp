/*
 *  TGame.cpp
 *  TootleGame
 *
 *  Created by Duane Bradbury on 06/02/2009.
 *  Copyright 2009 Tootle. All rights reserved.
 *
 */

#include "TGame.h"

using namespace TLGame;


SyncBool TGame::Initialise()
{
	AddModes();
	
	return SyncTrue;
}


SyncBool TGame::Update(float fTimeStep)
{
	// Update the game state machine
	TStateMachine::Update(fTimeStep);
	
	return SyncTrue;
}

