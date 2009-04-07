/*
 *  TApplication.cpp
 *  TootleCore
 *
 *  Created by Duane Bradbury on 06/02/2009.
 *  Copyright 2009 Tootle. All rights reserved.
 *
 */

#include "TApplication.h"

#include <TootleCore/TCoreManager.h>
#include <TootleRender/TLRender.h>
#include <TootleRender/TScreenManager.h>
#include <TootleFileSys/TLFileSys.h>
#include <TootleFileSys/TFileSys.h>


#include <TootleInput/TLInput.h>


// Time for the bootup sequence to occur over
#ifdef TL_TARGET_PC
	#define BOOTUP_TIME_MIN 2.0f
#else
	#define BOOTUP_TIME_MIN 0.2f	//	gr: reduced so just gets out of splash screen asap
#endif


using namespace TLCore;


//-----------------------------------------------------------
//	Application initialisation
//-----------------------------------------------------------
SyncBool TApplication::Initialise()
{
	Bool Waiting = FALSE;

	//	create file systems
	if ( !m_LocalFileSysRef.IsValid() )
	{
		#if defined(TL_TARGET_PC)
			TTempString AssetDir = "Assets\\";
		#elif defined(TL_TARGET_MAC)
			TTempString AssetDir = "Assets\\";
		#elif defined(TL_TARGET_IPOD)
			TTempString AppName = GetName();
			TTempString AssetDir = AppName;	
			AssetDir.Append(".app/");
		#endif

		TLFileSys::CreateLocalFileSys( m_LocalFileSysRef, AssetDir, FALSE );
	}

	//	continue init of file system
	if ( m_LocalFileSysRef.IsValid() )
	{
		SyncBool Result = SyncFalse;

		TPtr<TLFileSys::TFileSys>& pFileSys = TLFileSys::GetFileSys( m_LocalFileSysRef );
		if ( pFileSys )
			Result = pFileSys->Init();

		if ( Result == SyncFalse )
		{
			//	gr: fail if asset dir cannot be opened
			TLDebug_Break("failed to create local file sys");
			m_LocalFileSysRef.SetInvalid();
			return SyncFalse;
		}
			
		Waiting |= (Result == SyncWait);
	}



	//	create docs file sys
	if ( !m_UserFileSysRef.IsValid() )
	{
		#if defined(TL_TARGET_PC)
			TTempString AssetDir = "Assets\\User\\";
		#elif defined(TL_TARGET_MAC)
			TTempString AssetDir = "Assets\\User\\";
		#elif defined(TL_TARGET_IPOD)
			TTempString AssetDir = "Documents/";	//	gr: I'm pretty sure the documents dir is above the app dir
		#endif

		TLFileSys::CreateLocalFileSys( m_UserFileSysRef, AssetDir, TRUE );
	}	
	
	//	continue init of file system
	if ( m_UserFileSysRef.IsValid() )
	{
		SyncBool Result = SyncFalse;

		TPtr<TLFileSys::TFileSys>& pFileSys = TLFileSys::GetFileSys( m_UserFileSysRef );
		if ( pFileSys )
			Result = pFileSys->Init();

		if ( Result == SyncFalse )
		{
			//	gr: let system continue if this dir cannot be used
			TLDebug_Print("failed to create local user file sys");
			m_UserFileSysRef.SetInvalid();
		}
			
		Waiting |= (Result == SyncWait);
	}


	//	subscribe to screen manager to get screen-deleted messages
	SubscribeTo( TLRender::g_pScreenManager );
	
	//	subscribe core to app so we can send quit message
	TLCore::g_pCoreManager->SubscribeTo( this );
	
	//	create screen
	//	gr: todo: turn this into some factory? or an alias for platform screen?
	TPtr<TLRender::TScreen>& pScreen = TLRender::g_pScreenManager->GetInstance(TRef("Screen"), TRUE, GetDefaultScreenType() );
	if(!pScreen)
	{
		TLDebug_Print("Failed to create main screen object");
		return SyncFalse;
	}
	
	// Initialise the screen object
	SyncBool Result = pScreen->Init();
	if ( Result != SyncTrue )
	{
		TLDebug_Print("Error: Failed to initialise screen");
		return Result;
	}
	
	// Add the application modes
	AddModes();

	return TManager::Initialise();
}


void TApplication::AddModes()
{
	AddMode<TApplicationState_Bootup>("Bootup");
	AddMode<TApplicationState_FrontEnd>("FrontEnd");
	AddMode<TApplicationState_EnterGame>("EnterGame");
	AddMode<TApplicationState_Game>("Game");
	AddMode<TApplicationState_Pause>("Pause");
	AddMode<TApplicationState_ExitGame>("ExitGame");	
}



//-----------------------------------------------------------
//	Application update
//-----------------------------------------------------------
SyncBool TApplication::Update(float fTimeStep)
{
	// Udpate the state machine
	TStateMachine::Update(fTimeStep);
		
	return TManager::Update(fTimeStep);
}

//-----------------------------------------------------------
//	Application shutdown
//-----------------------------------------------------------
SyncBool TApplication::Shutdown()
{
	//	gr: put this in the clean up of the game mode?
	SyncBool Result = DestroyGameObject();
	if(Result != SyncTrue)
		return Result;

	//	clean up current mode bgy ending it
	TStateMachine::SetMode( TRef() );

	return TManager::Shutdown();
}


//-----------------------------------------------------------
//	process messages
//-----------------------------------------------------------
void TApplication::ProcessMessage(TLMessaging::TMessage& Message)
{
	if ( Message.GetMessageRef() == "ScreenChanged" ) 
	{
		TRef State;

		if(Message.ImportData("State", State))
		{
			//	screen was deleted
			if ( State == "Deleted" )
			{
				//	if there are no screens left, close app
				if ( TLRender::g_pScreenManager->GetSize() == 0 )
				{
					TLMessaging::TMessage Message(TLCore::QuitRef);
					PublishMessage( Message );
				}
			}
		}
	}

	TManager::ProcessMessage(Message);
}


void TApplication::OnEventChannelAdded(TRefRef refPublisherID, TRefRef refChannelID)
{

	TManager::OnEventChannelAdded(refPublisherID, refChannelID);
}



SyncBool TApplication::CreateGameObject()
{
	TLDebug_Break("Overload this! You cannot use the base func!");

	// Already have a game object? Create one if not
	if(!m_pGame)	
	{
		m_pGame = new TLGame::TGame("Game");

		// Subscribe the game object to the application
		m_pGame->SubscribeTo(this);
		
		// Subscribe to the core manager for update messages
		m_pGame->SubscribeTo(TLCore::g_pCoreManager);
		
		// Send a message to the new game object to initialise
		// NOTE: Currently asume this happens in a frame so may need to cater for it occuring over a number of frames
		TLMessaging::TMessage Message(TLCore::InitialiseRef);
		
		PublishMessage(Message);
		
		/*
		 // NOTE: I'm not sure this is a good plan.  The managers list in the core manager
		 // is for gloabal managers and rely on the mesaging to be done in a certain order
		 // for initialisation and shutdown.  Creation of  manager at this point would mean
		 // having missed the initialise message and also could cause issues if the shutdown occurs
		 // at the same time as the application manager shutdown - which removes it's pointer to the 
		 // game object and would need to request removing the manager from the core manager
		 // so it could get deleted prematurely
		 TLCore::g_pCoreManager->CreateAndRegisterManager<TLGame::TGame>(m_pGame, "GAMEMANAGER");
		
		 // Subscribe the game manager to the event channel manager
		 m_pGame->SubscribeTo(TLMessaging::g_pEventChannelManager);

		 // Subscribe the game manager to the core manager messages
		 TLMessaging::g_pEventChannelManager->SubscribeTo(this, "COREMANAGER", TLCore::UpdateRef); 
		 TLMessaging::g_pEventChannelManager->SubscribeTo(this, "COREMANAGER", TLCore::ShutdownRef); 
		 */
	}
	
	return SyncTrue;
}


SyncBool TApplication::DestroyGameObject()
{
	
	// Send a shutdown message to the game object
	// NOTE: Currently asume this happens in a frame so may need to cater for it occuring over a number of frames
	TLMessaging::TMessage Message(TLCore::ShutdownRef);
	
	PublishMessage(Message);
			
	m_pGame = NULL;
	
	// NOTE: This would likely cause issues if the game object was a manager and subscribed to the 
	// core manager removing it could occur during shutdown and may leave the game object 
	// without having been shutdown properly.
	//TLCore::g_pCoreManager->UnRegisterManager("GAMEMANAGER");
	
	return SyncTrue;
}


//--------------------------------------------------
//	notify subscribers when option changes - and do any specific option stuff
//--------------------------------------------------
void TApplication::OnOptionChanged(TRefRef OptionRef)
{
	TLMessaging::TMessage Message("OptChanged");
	Message.Write( OptionRef );
	
	PublishMessage( Message );
}









// Application States


// Bootup state

Bool TApplication::TApplicationState_Bootup::OnBegin(TRefRef PreviousMode)
{
	TApplication* pApp = GetStateMachine<TApplication>();
	// TODO:

	//	gr: if this fails (e.g. no logo asset) skip onto next mode
	Bool bSuccess = CreateIntroScreen();
	
	if(!bSuccess)
		return FALSE;
	
	// TODO:
	// Setup language
	// Setup Audio - language specific
	// Load Text - language specific
	// Setup global settings

	// Obtain list of files to load at startup
	pApp->GetPreloadFiles(m_PreloadFiles);

	if(m_PreloadFiles.GetSize() > 0)
		PreloadFiles();
	
	return TStateMode::OnBegin(PreviousMode);
}


TApplication::TApplicationState_Bootup::TApplicationState_Bootup() :
	m_SkipBootup	( FALSE )
{
}

Bool TApplication::TApplicationState_Bootup::CreateIntroScreen()
{
	m_SkipBootup = FALSE;

	TPtr<TLRender::TScreen>& pScreen = TLRender::g_pScreenManager->GetDefaultScreen();	
	if(!pScreen)
	{
		TLDebug_Break("Error: Failed to get screen");
		return FALSE;
	}
		
	//	create background graphic
	TPtr<TLAsset::TAsset>& pBgAsset = TLAsset::LoadAsset("logo", TRUE);
	if ( pBgAsset )
	{
		TLMessaging::TMessage InitMessage(TLCore::InitialiseRef);
		InitMessage.ExportData("MeshRef", pBgAsset->GetAssetRef() );
		InitMessage.ExportData(TRef_Static(T,r,a,n,s), float3( 0.f, 0.f, -50.f ) );
		InitMessage.ExportData("LineWidth", 3.f );

		m_LogoRenderNode = TLRender::g_pRendergraph->CreateNode("logo", TRef(), TRef(), &InitMessage );
	}
	else
	{
		//TLDebug_Break("Error: Failed to load logo asset");
		m_SkipBootup = TRUE;

		//	gr: this will just go into a "no mode" mode.
		//return FALSE;
	}

	//	create a render target if we created a render node
	if ( m_LogoRenderNode.IsValid() )
	{
		TPtr<TLRender::TRenderTarget> pRenderTarget = pScreen->CreateRenderTarget( TRef("Intro") );
		if(!pRenderTarget)
		{
			TLDebug_Break("Error: Failed to create logo render target");
			return FALSE;
		}
	
		m_RenderTarget = pRenderTarget->GetRef();
		pRenderTarget->SetClearColour( TColour( 1.f, 1.f, 1.f, 1.f ) );
	
		TPtr<TLRender::TCamera> pCamera = new TLRender::TOrthoCamera;
		pRenderTarget->SetCamera( pCamera );
		pCamera->SetPosition( float3( 0, 0, -10.f ) );
		pRenderTarget->SetRootRenderNode( m_LogoRenderNode );
	}
	
	// All done
	return TRUE;
}


TRef TApplication::TApplicationState_Bootup::Update(float Timestep)
{
	if ( m_SkipBootup || (GetModeTime() > BOOTUP_TIME_MIN) && ArePreloadFilesLoaded() )
	{
		TApplication* pApp = GetStateMachine<TApplication>();
		
		// If we have a front end mode then go to the front end mode, otherwise drop into the enter game mode
		if(pApp->HasMode("FrontEnd"))
			return "FrontEnd";
		else
			return "EnterGame";
	}

	// Wait for preload files to be loaded and the min time
	return TRef();
};


void TApplication::TApplicationState_Bootup::PreloadFiles()
{
	for(u32 uIndex = 0; uIndex < m_PreloadFiles.GetSize(); uIndex++)
	{
		TLAsset::LoadAsset(m_PreloadFiles.ElementAt(uIndex));
	}
}

Bool TApplication::TApplicationState_Bootup::ArePreloadFilesLoaded()
{
	for(u32 uIndex = 0; uIndex < m_PreloadFiles.GetSize(); uIndex++)
	{
		TRef& refFileID = m_PreloadFiles.ElementAt(uIndex);
		
		TPtr<TLAsset::TAsset>& pAsset = TLAsset::GetAsset(refFileID);
		if ( !pAsset )
		{
			TTempString DebugString("Preload: Missing Asset: ");
			refFileID.GetString( DebugString );
			TLDebug_Print( DebugString );
			continue;
		}
		
		TLAsset::TLoadingState AssetLoadingState = pAsset->GetLoadingState();

		//	file failed to load
		if ( AssetLoadingState == TLAsset::LoadingState_Failed )
		{
			TTempString DebugString("Preload: Asset failed to load: ");
			refFileID.GetString( DebugString );
			DebugString.Appendf(" %x", pAsset.GetObject() );
			TLDebug_Print( DebugString );
			continue;
		}
		else if ( AssetLoadingState == TLAsset::LoadingState_Loaded )
		{
			//	is loaded
			continue;
		}
		else
		{
			//	still waiting for this asset
			TTempString DebugString("Preload: Waiting for asset to load... ");
			refFileID.GetString( DebugString );
			DebugString.Append(" (");
			pAsset->GetAssetType().GetString( DebugString );
			DebugString.Append(")");
			TLDebug_Print( DebugString );
			return FALSE;
		}
	}

	
	return TRUE;
}


void TApplication::TApplicationState_Bootup::OnEnd(TRefRef NextMode)
{
	//	delete node
	TLRender::g_pRendergraph->RemoveNode( m_LogoRenderNode );

	//	delete render target
	if ( m_RenderTarget.IsValid() )
	{
		TLRender::g_pScreenManager->DeleteRenderTarget( m_RenderTarget );
		m_RenderTarget.SetInvalid();
	}

	//	delete asset
	TLAsset::DeleteAsset("logo");
}



// Front End state
Bool TApplication::TApplicationState_FrontEnd::OnBegin(TRefRef PreviousMode)
{
	// TODO:
	// Load front end scheme
	
	return TStateMode::OnBegin(PreviousMode);
}

TRef TApplication::TApplicationState_FrontEnd::Update(float Timestep)
{
	// Essentially wait until the app is signalled for what mode to change into
	return TRef();
};



// Enter Game transitional state
Bool TApplication::TApplicationState_EnterGame::OnBegin(TRefRef PreviousMode)
{
	// TODO:
	// Begin transition

	// Create (gamespecific) TGame object
	TApplication* pApp = GetStateMachine<TApplication>();
	
	pApp->CreateGameObject();
	
	return TStateMode::OnBegin(PreviousMode);
}

TRef TApplication::TApplicationState_EnterGame::Update(float Timestep)
{	
	// Wait for game files to laod
	return "Game";
};



// Game active state
TRef TApplication::TApplicationState_Game::Update(float Timestep)
{
	return TRef();
};



// Game paused state - may be moved to a state on the TGame object instead
TRef TApplication::TApplicationState_Pause::Update(float Timestep)
{
	// Pause game
	return TRef();
};



// Exit Game transitional state
Bool TApplication::TApplicationState_ExitGame::OnBegin(TRefRef PreviousMode)
{
	// TODO:
	// Begin transition
	// Save states of stuff
	
	return TStateMode::OnBegin(PreviousMode);
}

TRef TApplication::TApplicationState_ExitGame::Update(float Timestep)
{
	// Wait for transition to be ready
	// return TRef();
	
	// Request unload game schemes
		
	return "FrontEnd";
};

void TApplication::TApplicationState_ExitGame::OnEnd(TRefRef NextMode)
{
	// Destroy (game specific) TGame object
	TApplication* pApp = GetStateMachine<TApplication>();
	
	pApp->DestroyGameObject();	
	
	return TStateMode::OnEnd(NextMode);
}




