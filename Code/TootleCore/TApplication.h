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
#include <TootleGame/TTimeline.h>

namespace TLCore
{
	class TApplication;

};


#include "TLanguage.h"


//---------------------------------------------------------
//	base application type
//---------------------------------------------------------
class TLCore::TApplication : public TLCore::TManager, public TStateMachine
{
protected:
	// Internal applicaiton states
	//	gr: can we rename these to something more usable... 
	//	their typenames are TLCore::TApplication::TApplicationState_Bootup
	//		TLCore::TApplication::Mode_Bootup would be a bit nicer :)
	class Mode_Base;					//	base mode - just eases access back to app
	class TApplicationState_Bootup;
	class TApplicationState_FrontEnd;
	class TApplicationState_EnterGame;
	class TApplicationState_Game;
	class TApplicationState_Pause;
	class TApplicationState_ExitGame;

public:	
	TApplication(TRefRef ManagerRef) :
		TLCore::TManager	( ManagerRef ),
		m_Options			("Options")
	{
	}
	
	//	global options
	FORCEINLINE void	SetLanguage(TRefRef LanguageRef)	{	SetOption("Lang", LanguageRef );	}
	FORCEINLINE TRef	GetLanguage()						{	return GetOption("Lang", TLLanguage::g_LanguageRef_English );	}
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

	template <class TYPE>
	TYPE*						GetGameObject()				{ return static_cast<TYPE*>(m_pGame.GetObject()); }

	FORCEINLINE void			SetAppMode(TRefRef Mode)	{	m_NewAppMode = Mode;	}	//	change app mode on next update

protected:
	virtual SyncBool			Initialise();
	virtual SyncBool			Update(float fTimestep);
	virtual SyncBool			Shutdown();
	
	virtual void				AddModes();
	
	virtual void				ProcessMessage(TLMessaging::TMessage& Message);	//	process messages
	
	virtual void				OnEventChannelAdded(TRefRef refPublisherID, TRefRef refChannelID);

	virtual void				GetPreloadFiles(TArray<TTypedRef>& PreloadFiles)		{} // builds a list of files to load at boot up

	virtual TTempString			GetName()	const = 0;			// Name of application - must be specified
	virtual void				GetName(TString& String) const	{	String.Append( GetName() );	}
	virtual TRef				GetDefaultScreenType() const	{	return "Screen";	}
	
	// Game object creation and destruction
	virtual TPtr<TLGame::TGame>	CreateGameObject();				//	overload this to return the game specific object
	Bool						CreateGame();
	void						DestroyGame();
	
	// Import and export of the global preferences
	virtual Bool				ExportData(TBinaryTree& data)			{ return FALSE; }
	virtual Bool				ImportData(TBinaryTree& data)			{ return FALSE; }

	void						OnOptionChanged(TRefRef OptionRef);		//	notify subscribers when option changes - and do any specific option stuff
		
private:
	TPtr<TLGame::TGame>		m_pGame;				//	The application's game object
	TRef					m_LocalFileSysRef;		//	ref of local file sys
	TRef					m_UserFileSysRef;		//	ref of local file sys user's runtime assets/save files etc
	TBinaryTree				m_Options;		//	Global & app specific preferences
	TRef					m_NewAppMode;	//	if valid, we switch to this app mode on next update
};





//---------------------------------------------------------
//	base application mode - just gives easy access back to app
//---------------------------------------------------------
class TLCore::TApplication::Mode_Base : public TStateMode
{
public:
	template<class APPTYPE>
	FORCEINLINE APPTYPE*				GetApplication()		{	return TStateMode::GetStateMachine<APPTYPE>();	}
	//template<>
	//FORCEINLINE TLCore::TApplication*	GetApplication()		{	return TStateMachine::GetStateMachine<TLCore::TApplication>();	}
};


//---------------------------------------------------------
// Bootup state
//---------------------------------------------------------
class TLCore::TApplication::TApplicationState_Bootup : public TLCore::TApplication::Mode_Base, public TLMessaging::TSubscriber
{
public:
	TApplicationState_Bootup();
	virtual Bool			OnBegin(TRefRef PreviousMode);
	virtual TRef			Update(float Timestep);
	virtual void			OnEnd(TRefRef NextMode);

protected:	

	void		ProcessMessage(TLMessaging::TMessage& Message);

	Bool		CreateIntroScreen();

	FORCEINLINE Bool ArePreloadFilesLoaded()	{ return (m_PreloadFiles.GetSize() == 0); }
	
private:
	TRef									m_LogoRenderNode;	//	
	TPtr<TLAnimation::TTimelineInstance>	m_pTimelineInstance;
	TRef									m_RenderTarget;		//	our render target
	TArray<TTypedRef>						m_PreloadFiles;		//	waiting for these files to async load
	Bool									m_SkipBootup;		//	skip bootup if we failed to create rendernode/screen/logo etc
};


//---------------------------------------------------------
// Front End state
//---------------------------------------------------------
class TLCore::TApplication::TApplicationState_FrontEnd : public TLCore::TApplication::Mode_Base
{
public:
	virtual Bool			OnBegin(TRefRef PreviousMode);
	virtual TRef			Update(float Timestep);			
};


//---------------------------------------------------------
// Enter Game transitional state
//---------------------------------------------------------
class TLCore::TApplication::TApplicationState_EnterGame : public TLCore::TApplication::Mode_Base
{
public:
	virtual Bool			OnBegin(TRefRef PreviousMode);
	virtual TRef			Update(float Timestep);			
};


//---------------------------------------------------------
// Game active state
//---------------------------------------------------------
class TLCore::TApplication::TApplicationState_Game : public TLCore::TApplication::Mode_Base
{
public:
	virtual TRef			Update(float Timestep);			
};


//---------------------------------------------------------
// Game paused state
//---------------------------------------------------------
class TLCore::TApplication::TApplicationState_Pause : public TLCore::TApplication::Mode_Base
{
public:
	virtual TRef			Update(float Timestep);			
};


//---------------------------------------------------------
// Exit Game transitional state
//---------------------------------------------------------
class TLCore::TApplication::TApplicationState_ExitGame : public TLCore::TApplication::Mode_Base
{
public:
	virtual Bool			OnBegin(TRefRef PreviousMode);
	virtual TRef			Update(float Timestep);
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


