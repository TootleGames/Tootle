/*
 *  TGame.cpp
 *  TootleGame
 *
 *  Created by Duane Bradbury on 06/02/2009.
 *  Copyright 2009 Tootle. All rights reserved.
 *
 */

#include "TGame.h"


TLGame::TGame::TGame(TRefRef ManagerRef,TLCore::TApplication& Application) :
	TLCore::TManager	( ManagerRef ),
	m_pApplication		( &Application )
{
	if ( !m_pApplication )
	{
		TLDebug_Break("application expected");
	}
}

	
TLGame::TGame::~TGame()
{
	if ( m_pApplication )
	{
		TLDebug_Break("TGame was not Shutdown() properly");
	}
}

SyncBool TLGame::TGame::Initialise()
{
	AddModes();
	
	//	init the manager
	return TManager::Initialise();
}


SyncBool TLGame::TGame::Update(float fTimeStep)
{
	//	request to change app mode
	if ( m_NewGameMode.IsValid() )
	{
		if ( GetCurrentModeRef() != m_NewGameMode )
			TStateMachine::SetMode( m_NewGameMode );
		m_NewGameMode.SetInvalid();
	}

	// Update the game state machine
	TStateMachine::Update(fTimeStep);
	
	//	update the manager
	return TManager::Update( fTimeStep );
}


SyncBool TLGame::TGame::Shutdown()
{
	//	shutdown the state machine
	TStateMachine::Shutdown();
	
	//	release app ptr
	m_pApplication = NULL;

	//	shutdown the manager
	return TManager::Shutdown();
}
	
