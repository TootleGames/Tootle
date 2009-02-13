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
	//	create a local file sys in our asset dir
#if defined(TL_TARGET_PC)
	TTempString AssetDir = "Assets\\";
#elif defined(TL_TARGET_IPOD)
	TTempString AppName = GetName();
	
	TTempString AssetDir = AppName;	
	AssetDir.Append(".app/");
#endif
	
	TRef FileSysRef;
	SyncBool Result = TLFileSys::CreateLocalFileSys( FileSysRef, AssetDir );
	if ( Result != SyncTrue )
	{
		TLDebug_Print("Error: Failed to create local file system");
		return Result;
	}
	
	//	subscribe to screen manager to get screen-deleted messages
	SubscribeTo( TLRender::g_pScreenManager );
	
	//	subscribe core to app so we can send quit message
	TLCore::g_pCoreManager->SubscribeTo( this );
	
	//	create screen
	//	gr: todo: turn this into some factory? or an alias for platform screen?
	TPtr<TLRender::TScreen> pScreen = TLRender::g_pScreenManager->GetInstance(TRef("Screen"), TRUE, TRef("Screen") );
	
	if(!pScreen)
	{
		TLDebug_Print("Failed to create main screen object");
		return SyncFalse;
	}
	
	// Initialise the screen object
	Result = pScreen->Init();
	
	if ( Result != SyncTrue )
	{
		TLDebug_Print("Error: Failed to initialise screen");
		return Result;
	}

	/////////////////////////////////////////////////////////////
	// DB - Needs to be moved/removed from here
	// NOTE: Should be able to intercept the screen creation by subscribing the input 
	// to the screen manager instead and subscribing when that occurs
	/////////////////////////////////////////////////////////////
	// Subscribe the input system to the screen
	if(TLInput::g_pInputSystem.IsValid())
	{
		// DB - Subscribe the input system to the screen (should be the manager but only one screen atm)
		// The input system will then be notified if changes to the 'display' occur which is needed because the devices 
		// need access to this at the platform specific level.
		TLInput::g_pInputSystem->SubscribeTo(pScreen);
	}
	/////////////////////////////////////////////////////////////
	
	
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
	TStateMachine::Update();
		
	return TManager::Update(fTimeStep);
}

//-----------------------------------------------------------
//	Application shutdown
//-----------------------------------------------------------
SyncBool TApplication::Shutdown()
{
	SyncBool Result = DestroyGameObject();
	
	if(Result != SyncTrue)
		return Result;
	
	return TManager::Shutdown();
}


//-----------------------------------------------------------
//	process messages
//-----------------------------------------------------------
void TApplication::ProcessMessage(TPtr<TLMessaging::TMessage>& pMessage)
{
	
	TManager::ProcessMessage(pMessage);
}


void TApplication::OnEventChannelAdded(TRefRef refPublisherID, TRefRef refChannelID)
{

	TManager::OnEventChannelAdded(refPublisherID, refChannelID);
}



SyncBool TApplication::CreateGameObject()
{
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
		TPtr<TLMessaging::TMessage> pMessage = new TLMessaging::TMessage(TLCore::InitialiseRef);
		
		if(pMessage)
			PublishMessage(pMessage);
		
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
	TPtr<TLMessaging::TMessage> pMessage = new TLMessaging::TMessage(TLCore::ShutdownRef);
	
	if(pMessage)
		PublishMessage(pMessage);
			
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
	TPtr<TLMessaging::TMessage> pMessage = new TLMessaging::TMessage("OptChanged");
	pMessage->Write( OptionRef );
	
	PublishMessage( pMessage );
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
	
	// Setup language
	// Setup Audio - language specific
	// Load Text - language specific
	// Setup global settings
	// Obtain list of files to load at startup
	
	m_fTimer = 0.0f;
	
	return TStateMode::OnBegin(PreviousMode);
}

Bool TApplication::TApplicationState_Bootup::CreateIntroScreen()
{
	TPtr<TLRender::TScreen> pScreen = TLRender::g_pScreenManager->GetInstance(TRef("Screen"), TRUE, TRef("Screen") );
	
	if(!pScreen)
	{
		TLDebug_Break("Error: Failed to get screen");
		return FALSE;
	}
		
	//	create a render target 
	TPtr<TLRender::TRenderTarget> pRenderTarget = pScreen->CreateRenderTarget( TRef("Intro") );
	if(!pRenderTarget)
	{
		TLDebug_Break("Error: Failed to create logo render target");
		return FALSE;
	}
	pRenderTarget->GetClearColour().Set( 1.f, 1.f, 1.f, 1.f );
	
	
	TPtr<TLRender::TCamera> pCamera = new TLRender::TOrthoCamera;
	pRenderTarget->SetCamera( pCamera );
	pCamera->SetPosition( float3( 0, 0, -10.f ) );
	
	TPtr<TLRender::TRenderNode> pRootRenderNode = new TLRender::TRenderNode("Intro");
	pRenderTarget->SetRootRenderNode(pRootRenderNode);
	TLRender::g_pRendergraph->AddNode( pRootRenderNode );
	
	
	//	create background graphic
	TPtr<TLAsset::TAsset>& pBgAsset = TLAsset::LoadAsset("logo", TRUE);
	if ( pBgAsset )
	{
		TPtr<TLRender::TRenderNode> pBgNode = new TLRender::TRenderNode("logo");
		pBgNode->SetMeshRef( pBgAsset->GetAssetRef() );
		pBgNode->SetTranslate( float3( 0.f, 0.f, -50.f ) );
		pBgNode->SetLineWidth( 3.f );
		
		TLRender::g_pRendergraph->AddNode( pBgNode, pRootRenderNode );
	}
	else
	{
		TLDebug_Break("Error: Failed to load logo asset");
		return FALSE;
	}	
	
	// All done
	return TRUE;
}


TRef TApplication::TApplicationState_Bootup::Update()
{

	m_fTimer += 1/60.0f;//fTimeStep;
	
	if((m_fTimer > BOOTUP_TIME_MIN) && ArePreloadFilesLoaded())
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
		
		//	waiting to load
		if ( pAsset->GetLoadingState() == TLAsset::LoadingState_Loading )
			return FALSE;
		
		//	file failed to load
		if ( pAsset->GetLoadingState() == TLAsset::LoadingState_Failed )
		{
			TTempString DebugString("Preload: Asset failed to load: ");
			refFileID.GetString( DebugString );
			TLDebug_Print( DebugString );
			continue;
		}
	}
	
	return TRUE;
}



void TApplication::TApplicationState_Bootup::OnEnd(TRefRef NextMode)
{
	// Remove the intro screen
	TPtr<TLRender::TRenderNode>& pRootObject = TLRender::g_pRendergraph->FindNode("Intro");
	
	if(pRootObject)
	{
		// Remove render graph nodes
		TLRender::g_pRendergraph->RemoveNode( pRootObject );
	}

	TPtr<TLRender::TScreen> pScreen = TLRender::g_pScreenManager->GetInstance(TRef("Screen"), TRUE, TRef("Screen") );

	if(pScreen)
	{
		// Remove render target
		pScreen->DeleteRenderTarget( TRef("Intro") );
	}

	TLAsset::DeleteAsset("logo");
}



// Front End state
Bool TApplication::TApplicationState_FrontEnd::OnBegin(TRefRef PreviousMode)
{
	// TODO:
	// Load front end scheme
	
	return TStateMode::OnBegin(PreviousMode);
}

TRef TApplication::TApplicationState_FrontEnd::Update()
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

TRef TApplication::TApplicationState_EnterGame::Update()
{	
	// Wait for game files to laod
	return "Game";
};



// Game active state
TRef TApplication::TApplicationState_Game::Update()
{
	return TRef();
};



// Game paused state - may be moved to a state on the TGame object instead
TRef TApplication::TApplicationState_Pause::Update()
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

TRef TApplication::TApplicationState_ExitGame::Update()
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




