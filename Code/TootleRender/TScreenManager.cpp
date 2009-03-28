#include "TScreenManager.h"
#include <TootleCore/TLTime.h>


#if defined(TL_TARGET_PC) && defined(_MSC_EXTENSIONS)
	#include "PC/PCScreen.h"
#elif defined(TL_TARGET_IPOD)
	#include "IPod/IPodScreen.h"
#elif defined(TL_TARGET_MAC)
	#include "Mac/MacScreen.h"
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

SyncBool TScreenManager::Initialise() 
{	
	if(TLMessaging::g_pEventChannelManager)
	{
		TLMessaging::g_pEventChannelManager->RegisterEventChannel(this, GetManagerRef(), "ScreenChanged");
		TLMessaging::g_pEventChannelManager->RegisterEventChannel(this, GetManagerRef(), "OnWindowChanged");

		return SyncTrue;
	}

	return SyncWait; 
}


//----------------------------------------------------------
//	instance an asset
//----------------------------------------------------------
TScreen* TScreenManager::CreateObject(TRefRef InstanceRef,TRefRef TypeRef)
{
	TScreen* pScreen = NULL;

	//	default to screen
	if ( !TypeRef.IsValid() || TypeRef == "Screen" )
		pScreen = new TLRender::Platform::Screen( InstanceRef, TLRender::ScreenShape_Portrait );
	else if ( TypeRef == "wideleft" )
		pScreen = new TLRender::Platform::Screen( InstanceRef, TLRender::ScreenShape_WideLeft );
	else if ( TypeRef == "wideright" )
		pScreen = new TLRender::Platform::Screen( InstanceRef, TLRender::ScreenShape_WideRight );
	else if ( TypeRef == "widescreen" )
	{
#if defined(TL_TARGET_PC)
		pScreen = new TLRender::Platform::ScreenWide( InstanceRef );
#elif defined(TL_TARGET_IPOD)
		pScreen = new TLRender::Platform::Screen( InstanceRef, TLRender::ScreenShape_WideRight );
#endif
	}

	if(pScreen)
	{
		// Subscribe to the screen so we can get the screen messages
		SubscribeTo(pScreen);
	}

	return pScreen;
}


//----------------------------------------------------------
//	render screens
//----------------------------------------------------------
void TScreenManager::ProcessMessage(TLMessaging::TMessage& Message)
{
	TRefRef MessageRef = Message.GetMessageRef();

	//	gr: ipod does no update in the screen - just break out now
	#ifdef TL_TARGET_IPOD
	{
		if ( MessageRef == TLCore::UpdateRef )
			return;
	}
	#endif // TL_TARGET_IPOD

	if ( MessageRef == TRef_Static(R,e,n,d,e) )
	{
		TLTime::TScopeTimer Timer( TRef_Static4(D,r,a,w) );

		//	render our screens
		TPtrArray<TLRender::TScreen>& Screens = GetInstanceArray();
		for ( u32 s=0;	s<Screens.GetSize();	s++ )
		{
			Screens[s]->Draw();
		}
	}
	else if(MessageRef == TRef_Static(O,n,W,i,n) )	//	"OnWindowChanged"
	{
		// Forward on any window messages
		PublishMessage(Message);
	}

	TManager::ProcessMessage( Message );
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
			TLMessaging::TMessage Message("ScreenChanged", "ScreenManager");
			Message.ExportData("State", TRef("Deleted") );
			Message.ExportData("ScreenRef", pScreen->GetRef() );

			//	shutdown and delete
			pScreen->Shutdown();
			pScreen = NULL;
			TObjectFactory<TLRender::TScreen>::RemoveAt( i );

			//	send out message
			PublishMessage( Message );
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


//----------------------------------------------------------
//	find the specified render target in all our screens
//----------------------------------------------------------
TPtr<TLRender::TRenderTarget>& TLRender::TScreenManager::GetRenderTarget(TRefRef RenderTargetRef,TPtr<TScreen>& pScreen)
{
	TPtrArray<TScreen>& ScreenArray = GetInstanceArray();

	//	search screens for render targets
	for ( u32 s=0;	s<ScreenArray.GetSize();	s++ )
	{
		TPtr<TLRender::TScreen>& pScreenChild = ScreenArray.ElementAt( s );

		//	find specified rendertarget in this screen
		TPtr<TLRender::TRenderTarget>& pRenderTarget = pScreenChild->GetRenderTarget( RenderTargetRef );
		if ( pRenderTarget )
		{
			pScreen = pScreenChild;
			return pRenderTarget;
		}
	}

	return TLPtr::GetNullPtr<TLRender::TRenderTarget>();
}


//----------------------------------------------------------
//	find the specified render target in all our screens
//----------------------------------------------------------
TPtr<TLRender::TRenderTarget>& TLRender::TScreenManager::GetRenderTarget(TRefRef RenderTargetRef)
{
	TPtr<TScreen> pScreen;
	return GetRenderTarget( RenderTargetRef, pScreen );
}


//----------------------------------------------------------
//	find this render target and delete it
//----------------------------------------------------------
Bool TLRender::TScreenManager::DeleteRenderTarget(TRefRef RenderTargetRef)
{
	TPtrArray<TScreen>& ScreenArray = GetInstanceArray();

	for ( u32 s=0;	s<ScreenArray.GetSize();	s++ )
	{
		TPtr<TLRender::TScreen>& pScreenChild = ScreenArray.ElementAt( s );

		//	find specified rendertarget in this screen
		if ( pScreenChild->DeleteRenderTarget( RenderTargetRef ) != SyncFalse )
			return TRUE;
	}

	return FALSE;
}



//----------------------------------------------------------
//	returns the screen angle for the FIRST screen. this is just to make it easier and saves us fetching the right screen
//----------------------------------------------------------
const TLMaths::TAngle& TLRender::TScreenManager::GetScreenAngle()
{
	TPtr<TLRender::TScreen>& pDefaultScreen = GetDefaultScreen();

	//	no screens... erk
	if ( !pDefaultScreen )
	{
		TLDebug_Break("No screens in screen manager");
		static TLMaths::TAngle DummyAngle(0.f);
		return DummyAngle;
	}
	
	//	get rotation of first screen
	return pDefaultScreen->GetScreenAngle();
}


