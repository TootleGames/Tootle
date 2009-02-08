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

namespace TLCore
{
	class TApplication;
}

class TLCore::TApplication : public TManager
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

protected:
		
	virtual SyncBool	Initialise();
	virtual SyncBool	Update(float fTimestep);
	virtual SyncBool	Shutdown();
	
	virtual void		ProcessMessage(TPtr<TLMessaging::TMessage>& pMessage);	//	process messages
	
	
	// Import and export of the global preferences
	virtual Bool	ExportData(TBinaryTree& data)			{ return FALSE; }
	virtual Bool	ImportData(TBinaryTree& data)			{ return FALSE; }
	
private:
	
	TPtr<TBinaryTree>		m_pPreferences;						// Global preferences
	
	TStateMachine			m_ApplicationStateMachine;			// The application state machine
};