#include "TScreenManager.h"


#if defined(TL_TARGET_PC)
#include "PC/PCScreen.h"
#endif

#if defined(TL_TARGET_IPOD)
#include "IPod/IPodScreen.h"
#endif


namespace TLRender
{
	TPtr<TScreenManager> g_pScreenManager = NULL;
}


using namespace TLRender;

TScreenManager::TScreenManager(TRefRef refManagerID) :
	TManager	(refManagerID)
{
}


//----------------------------------------------------------
//	instance an asset
//----------------------------------------------------------
TScreen* TScreenManager::CreateObject(TRefRef InstanceRef,TRefRef TypeRef)
{
	//	default to screen
	if ( !TypeRef.IsValid() || TypeRef == "Screen" )
	{
		return new TLRender::Platform::Screen( InstanceRef );
	}

	return NULL;
}


//----------------------------------------------------------
//	render screens
//----------------------------------------------------------
void TScreenManager::ProcessMessage(TPtr<TLMessaging::TMessage>& pMessage)
{
	TRefRef MessageRef = pMessage->GetMessageRef();

	if ( MessageRef == "Render" )
	{
		//	render our screens
		TPtrArray<TLRender::TScreen>& Screens = GetInstanceArray();
		for ( u32 s=0;	s<Screens.GetSize();	s++ )
		{
			Screens[s]->Draw();
		}
	}

	TManager::ProcessMessage( pMessage );
}



//----------------------------------------------------------
//	update screens and report if a screen has been deleted
//----------------------------------------------------------
SyncBool TScreenManager::Update(float fTimeStep)
{
	//	do manager update
	SyncBool ManagerUpdate = TManager::Update( fTimeStep );
	if ( ManagerUpdate == SyncFalse )
		return SyncFalse;

	//	update screens
	for ( s32 i=TObjectFactory<TLRender::TScreen>::GetSize()-1;	i>=0;	i-- )
	{
		//	get object
		TPtr<TScreen>& pScreen = TObjectFactory<TLRender::TScreen>::ElementAt(i);
		if ( !pScreen )
			continue;

		//	update screen
		SyncBool ScreenUpdate = pScreen->Update();
		
		//	if update has failed, delete the screen
		if ( ScreenUpdate == SyncFalse )
		{
			//	create message
			TPtr<TLMessaging::TMessage> pMessage = new TLMessaging::TMessage("Deleted", "ScreenManager" );
			pMessage->AddChannelID("ScreenManager");
			pMessage->AddChildAndData("ScreenRef", pScreen->GetRef() );

			//	shutdown and delete
			pScreen->Shutdown();
			pScreen = NULL;
			TObjectFactory<TLRender::TScreen>::RemoveAt( i );

			//	send out message
			PublishMessage( pMessage );
		}
	}

	return SyncTrue;
}


//----------------------------------------------------------
//	shutdown 
//----------------------------------------------------------
SyncBool TScreenManager::Shutdown()
{
	SyncBool ManagerShutdown = TManager::Shutdown();
	SyncBool FactoryShutdown = TObjectFactory<TLRender::TScreen>::Shutdown();

	//	one shutdown has failed
	if ( ManagerShutdown == SyncFalse || FactoryShutdown == SyncFalse )
		return SyncFalse;

	//	still waiting for one to shutdown
	if ( ManagerShutdown == SyncWait || FactoryShutdown == SyncWait )
		return SyncWait;

	return SyncTrue;
}


//----------------------------------------------------------
//	
//----------------------------------------------------------
void TScreenManager::OnEventChannelAdded(TRefRef refPublisherID,TRefRef refChannelID)
{
	if(refPublisherID == "CORE")
	{
		// Subscribe to the render messages
		if(refChannelID == TLCore::UpdateRef)
			TLMessaging::g_pEventChannelManager->SubscribeTo(this, refPublisherID, refChannelID); 

		if(refChannelID == TLCore::RenderRef)
			TLMessaging::g_pEventChannelManager->SubscribeTo(this, refPublisherID, refChannelID); 
	}
	
	//	Super event channel routine
	TManager::OnEventChannelAdded(refPublisherID, refChannelID);
}