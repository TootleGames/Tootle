/*
 *  TApplication.h
 *  TootleCore
 *
 *	Base application class
 *
 *  Created by Duane Bradbury on 06/02/2009.
 *  Copyright 2009 Tootle. All rights reserved.
 *
 */

#pragma once

#include "TManager.h"
#include "TBinaryTree.h"
#include "TStateMachine.h"

#include <TootleGame/TGame.h>

namespace TLCore
{
	class TApplication;
}

class TLCore::TApplication : public TManager, public TStateMachine
{	
public:	
	
	TApplication(TRef refManagerID) :
		TManager(refManagerID)
	{
	}
	
	// Preferences access
	void	SetLanguage(u8 uLanguageID)					{ }
	u8		GetLanguage()						const	{ return 0; }
	
	void	SetAudioSoundEffectsVolume(u8 uVolume)		{ }
	u8		GetAudioSoundEffectsVolume()		const	{ return 1; }
	
	void	SetAudioMusicVolume(u8 uVolume)				{ }
	u8		GetAudioMusicVolume()				const	{ return 1; }
	
	// Game object access
	const TPtr<TLGame::TGame>&		GetGameObject()		const	{ return m_pGame; }


protected:
		
	virtual SyncBool			Initialise();
	virtual SyncBool			Update(float fTimestep);
	virtual SyncBool			Shutdown();
	
	virtual void				AddModes();
	
	virtual void				ProcessMessage(TPtr<TLMessaging::TMessage>& pMessage);	//	process messages
	
	virtual void				GetPreloadFiles(TArray<TRef>& PreloadFiles)		{} // builds a list of files to load at boot up

	virtual TTempString			GetName()	const = 0;		// Name of application - must be specified
	
	// Game object creation and destruction
	virtual SyncBool			CreateGameObject();
	SyncBool					DestroyGameObject();

	
	// Import and export of the global preferences
	virtual Bool				ExportData(TBinaryTree& data)			{ return FALSE; }
	virtual Bool				ImportData(TBinaryTree& data)			{ return FALSE; }

	// Internal applicaiton states
	class TApplicationState_Bootup;
	class TApplicationState_FrontEnd;
	class TApplicationState_EnterGame;
	class TApplicationState_Game;
	class TApplicationState_Pause;
	class TApplicationState_ExitGame;
	
protected:	
	TPtr<TLGame::TGame>		m_pGame;							// The application's game object
	
private:
	TPtr<TBinaryTree>		m_pPreferences;						// Global preferences	
};


// Bootup state
class TLCore::TApplication::TApplicationState_Bootup : public TStateMode
{
public:
	virtual Bool			OnBegin(TRefRef PreviousMode);
	virtual TRef			Update();
	virtual void			OnEnd(TRefRef NextMode);

protected:	

	Bool		CreateIntroScreen();

	void		PreloadFiles();
	Bool		ArePreloadFilesLoaded();
	
private:
	TArray<TRef>	m_PreloadFiles;
	float			m_fTimer;
};

// Front End state
class TLCore::TApplication::TApplicationState_FrontEnd : public TStateMode
{
public:
	virtual Bool			OnBegin(TRefRef PreviousMode);
	virtual TRef			Update();			
};

// Enter Game transitional state
class TLCore::TApplication::TApplicationState_EnterGame : public TStateMode
{
public:
	virtual Bool			OnBegin(TRefRef PreviousMode);
	virtual TRef			Update();			
};

// Game active state
class TLCore::TApplication::TApplicationState_Game : public TStateMode
{
public:
	virtual TRef			Update();			
};

// Game paused state
class TLCore::TApplication::TApplicationState_Pause : public TStateMode
{
public:
	virtual TRef			Update();			
};

// Exit Game transitional state
class TLCore::TApplication::TApplicationState_ExitGame : public TStateMode
{
public:
	virtual Bool			OnBegin(TRefRef PreviousMode);
	virtual TRef			Update();
	virtual void			OnEnd(TRefRef NextMode);	
};
