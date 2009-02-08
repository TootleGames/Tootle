/*
 *  TApplication.cpp
 *  TootleCore
 *
 *  Created by Duane Bradbury on 06/02/2009.
 *  Copyright 2009 Tootle. All rights reserved.
 *
 */

#include "TApplication.h"

using namespace TLCore;


//-----------------------------------------------------------
//	Application initialisation
//-----------------------------------------------------------
SyncBool TApplication::Initialise()
{
	
	return TManager::Initialise();
}


//-----------------------------------------------------------
//	Application update
//-----------------------------------------------------------
SyncBool TApplication::Update(float fTimeStep)
{
	// Udpate the state machine
	m_ApplicationStateMachine.Update();
	
	return TManager::Update(fTimeStep);
}

//-----------------------------------------------------------
//	Application shutdown
//-----------------------------------------------------------
SyncBool TApplication::Shutdown()
{
	return TManager::Shutdown();
}


//-----------------------------------------------------------
//	process messages
//-----------------------------------------------------------
void TApplication::ProcessMessage(TPtr<TLMessaging::TMessage>& pMessage)
{
	
	TManager::ProcessMessage(pMessage);
}
