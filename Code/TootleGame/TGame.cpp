/*
 *  TGame.cpp
 *  TootleGame
 *
 *  Created by Duane Bradbury on 06/02/2009.
 *  Copyright 2009 Tootle. All rights reserved.
 *
 */

#include "TGame.h"



SyncBool TLGame::TGame::Initialise()
{
	AddModes();
	
	//	init the manager
	return TManager::Initialise();
}


SyncBool TLGame::TGame::Update(float fTimeStep)
{
	// Update the game state machine
	TStateMachine::Update(fTimeStep);
	
	//	update the manager
	return TManager::Update( fTimeStep );
}


SyncBool TLGame::TGame::Shutdown()
{
	//	shutdown the state machine
	TStateMachine::Shutdown();
	
	//	shutdown the manager
	return TManager::Shutdown();
}
	
