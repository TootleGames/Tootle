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

};


#include "TLanguage.h"

class TLCore::TApplication : public TManager, public TStateMachine
{
protected:
	// Internal applicaiton states
	class TApplicationState_Bootup;
	class TApplicationState_FrontEnd;
	class TApplicationState_EnterGame;
	class TApplicationState_Game;
	class TApplicationState_Pause;
	class TApplicationState_ExitGame;

public:	
	TApplication(TRef refManagerID) :
		TManager	( refManagerID ),
		m_Options	("Options")
	{
	}
	
	//	global options
	FORCEINLINE void	SetLanguage(TRefRef LanguageRef)	{	SetOption("Lang", LanguageRef );	}
	FORCEINLINE TRef	GetLanguage()						{	return GetOption("Lang", TLanguage::g_LanguageRef_Eng );	}
	FORCEINLINE void	SetSoundVolume(u8 Volume)			{	SetOption("sfxvol", Volume );	}
	FORCEINLINE TRef	GetSoundVolume()					{	return GetOption<u8>("sfxvol", 100 );	}
	FORCEINLINE void	SetMusicVolume(u8 Volume)			{	SetOption("musvol", Volume );	}
	FORCEINLINE TRef	GetMusicVolume()					{	return GetOption<u8>("musvol", 100 );	}

	//	options access
	template<typename TYPE>
	Bool				GetOption(TRefRef OptionRef,TYPE& Value,const TYPE& DefaultValue,Bool SetDefault=FALSE);	//	get option value - if it doesnt exist FALSE is return and Value is set to DefaultValue
	template<typename TYPE>
	FORCEINLINE TYPE	GetOption(TRefRef OptionRef,const TYPE& DefaultValue)				{	TYPE Value;	GetOption( OptionRef, Value, DefaultValue );	return Value;	}
	template<typename TYPE>
	void				SetOption(TRefRef OptionRef,const TYPE& Value);						//	overwrite/add option value

	
	// Game object access
	//TPtr<TLGame::TGame>		GetGameObject()		const	{ return m_pGame; }

	template <class TYPE>
	TYPE*						GetGameObject()				{ return static_cast<TYPE*>(m_pGame.GetObject()); }

protected:
	virtual SyncBool			Initialise();
	virtual SyncBool			Update(float fTimestep);
	virtual SyncBool			Shutdown();
	
	virtual void				AddModes();
	
	virtual void				ProcessMessage(TLMessaging::TMessage& Message);	//	process messages
	
	virtual void				OnEventChannelAdded(TRefRef refPublisherID, TRefRef refChannelID);

	virtual void				GetPreloadFiles(TArray<TRef>& PreloadFiles)		{} // builds a list of files to load at boot up

	virtual TTempString			GetName()	const = 0;			// Name of application - must be specified
	virtual void				GetName(TString& String) const	{	String.Append( GetName() );	}
	virtual TRef				GetDefaultScreenType() const	{	return "Screen";	}
	
	// Game object creation and destruction
	virtual SyncBool			CreateGameObject();
	SyncBool					DestroyGameObject();

	
	// Import and export of the global preferences
	virtual Bool				ExportData(TBinaryTree& data)			{ return FALSE; }
	virtual Bool				ImportData(TBinaryTree& data)			{ return FALSE; }

	void						OnOptionChanged(TRefRef OptionRef);		//	notify subscribers when option changes - and do any specific option stuff
	
protected:	
	TPtr<TLGame::TGame>		m_pGame;				//	The application's game object
	TRef					m_LocalFileSysRef;		//	ref of local file sys
	TRef					m_UserFileSysRef;		//	ref of local file sys user's runtime assets/save files etc
	
private:
	TBinaryTree				m_Options;		//	Global & app specific preferences
};


// Bootup state
class TLCore::TApplication::TApplicationState_Bootup : public TStateMode
{
public:
	TApplicationState_Bootup();
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
	Bool			m_SkipBootup;		//	skip bootup if we failed to create rendernode/screen/logo etc
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



//--------------------------------------------------
//	get option value - if it doesnt exist FALSE is return and Value is set to DefaultValue
//--------------------------------------------------
template<typename TYPE>
Bool TLCore::TApplication::GetOption(TRefRef OptionRef,TYPE& Value,const TYPE& DefaultValue,Bool SetDefault)
{
	if ( m_Options.ImportData( OptionRef, Value ) )
		return TRUE;

	//	no existing settings, return default
	Value = DefaultValue;

	//	set the default
	if ( SetDefault )
	{
		SetOption( OptionRef, Value );
	}

	return FALSE;
}


//--------------------------------------------------
//	overwrite/add option value
//--------------------------------------------------
template<typename TYPE>
void TLCore::TApplication::SetOption(TRefRef OptionRef,const TYPE& Value)
{
	//	get existing option data
	TPtr<TBinaryTree>& pOptionData = m_Options.GetChild( OptionRef );

	//	no existing setting, create new
	if ( !pOptionData )
	{
		m_Options.ExportData( OptionRef, Value );
		OnOptionChanged( OptionRef );
		return;
	}

	//	get existing option value to see if the data changed
	pOptionData->ResetReadPos();
	TYPE* pOldValue = pOptionData->ReadNoCopy<TYPE>();
	if ( pOldValue )
	{
		//	no change - abort
		if ( *pOldValue == Value )
			return;
	}

	//	just overwrite data
	if ( pOldValue )
	{
		*pOldValue = Value;
	}
	else
	{
		//	clear existing data and write new data
		pOptionData->Empty();
		pOptionData->Write( Value );
	}

	//	data changed, notify as required
	OnOptionChanged( OptionRef );
}


