
#pragma once

#include <TootleCore/TManager.h>
#include "TScreen.h"

namespace TLRender
{
	class TScreenManager;

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

protected:
	virtual TScreen*		CreateObject(TRefRef InstanceRef,TRefRef TypeRef);
};

