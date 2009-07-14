/*------------------------------------------------------
	
	todo: move all the render targets into this factory/manager

	screens can then keep a list of render targets theyre going to render
	and render targets can have a list of screens (or 1 screen) theyre "subscribed" to
	
	this is to remove the need to find screens all the time which is a bit redundant
	and needlessly expensive, especially on the ipod where we only ever have one screen 
	anyway

-------------------------------------------------------*/
#pragma once

#include <TootleCore/TManager.h>
#include "TScreen.h"

namespace TLRender
{
	class TScreenManager;
	class TRenderTarget;

	extern TPtr<TScreenManager> g_pScreenManager;
}

class TLRender::TScreenManager : public TLCore::TManager, public TObjectFactory<TLRender::TScreen>
{
public:
	TScreenManager(TRefRef ManagerRef);
	
	const TLMaths::TAngle&		GetScreenAngle();			//	returns the screen angle for the FIRST screen. this is just to make it easier and saves us fetching the right screen
	TPtr<TLRender::TScreen>&	GetDefaultScreen()	{	return (GetInstanceArray().GetSize() > 0) ? GetInstanceArray().ElementAt( 0 ) : TLPtr::GetNullPtr<TLRender::TScreen>();	}	//	returns the default (first) screen
	
	TPtr<TRenderTarget>&		GetRenderTarget(TRefRef RenderTargetRef);							//	find the specified render target in all our screens
	TPtr<TRenderTarget>&		GetRenderTarget(TRefRef RenderTargetRef,TPtr<TScreen>& pScreen);	//	find the specified render target in all our screens - also sets the screen pointer
	Bool						DeleteRenderTarget(TRefRef RenderTargetRef);						//	find this render target and delete it

protected:
	virtual SyncBool			Initialise(); 
	virtual SyncBool			Update(float fTimeStep);
	virtual SyncBool			Shutdown();
	
	virtual void				ProcessMessage(TLMessaging::TMessage& Message);	//	process messages
	virtual void				OnEventChannelAdded(TRefRef refPublisherID, TRefRef refChannelID);

	virtual TScreen*			CreateObject(TRefRef InstanceRef,TRefRef TypeRef);
};

