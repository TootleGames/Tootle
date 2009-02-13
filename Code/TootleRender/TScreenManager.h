
#pragma once

#include <TootleCore/TManager.h>
#include "TScreen.h"

namespace TLRender
{
	class TScreenManager;
	class TRenderTarget;

	extern TPtr<TScreenManager> g_pScreenManager;
}

class TLRender::TScreenManager : public TManager, public TObjectFactory<TLRender::TScreen>
{
public:
	TScreenManager(TRefRef refManagerID);
	
	virtual SyncBool		Update(float fTimeStep);
	virtual SyncBool		Shutdown();
	
	virtual void			ProcessMessage(TPtr<TLMessaging::TMessage>& pMessage);	//	process messages
	virtual void			OnEventChannelAdded(TRefRef refPublisherID, TRefRef refChannelID);

	const TLMaths::TAngle&	GetScreenAngle();													//	returns the screen angle for the FIRST screen. this is just to make it easier and saves us fetching the right screen
	
	TPtr<TRenderTarget>&	GetRenderTarget(TRefRef RenderTargetRef);							//	find the specified render target in all our screens
	TPtr<TRenderTarget>&	GetRenderTarget(TRefRef RenderTargetRef,TPtr<TScreen>& pScreen);	//	find the specified render target in all our screens - also sets the screen pointer

protected:
	virtual TScreen*		CreateObject(TRefRef InstanceRef,TRefRef TypeRef);
};

