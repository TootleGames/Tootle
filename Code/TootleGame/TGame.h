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

	template<class STATEMACHINETYPE>
	class TGameMode;
}


//-------------------------------------------------------
//	templated state machine mode for games, just has easy access to the game type atm
//	use: TYourGameMode : public TLGame::TGameMode<TYourGame>;
//-------------------------------------------------------
template<class STATEMACHINETYPE>
class TLGame::TGameMode : public TStateMode
{
protected:
	FORCEINLINE STATEMACHINETYPE&	GetGame()		
	{
		TStateMachine* pStateMachine = TStateMode::GetStateMachine();
		STATEMACHINETYPE* pGame = static_cast<STATEMACHINETYPE*>( pStateMachine );
		return *pGame;
	}
};



// NOTE: Not sure whether we can make this a manager that is created and removed during the applications lifetime?  
// It would mean we can subscribe to it and provide some basic init, udpate and shutdown routines...
//	gr: dont make it a manager then! add "manager" functionality later if we need it
class TLGame::TGame : public TLCore::TManager, public TStateMachine
{
public:
	TGame(TRefRef ManagerRef) :
		TLCore::TManager	(ManagerRef)
	{
	}
	

protected:
	virtual SyncBool	Initialise();
	virtual SyncBool	Update(float fTimeStep);
	virtual SyncBool	Shutdown();

	virtual void		AddModes()		{	}
};

