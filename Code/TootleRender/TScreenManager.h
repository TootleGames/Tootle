
#pragma once

#include <TootleCore/TManager.h>
#include "TScreen.h"

namespace TLRender
{
	class TScreenManager;

	extern TPtr<TScreenManager> g_pScreenManager;
}

class TLRender::TScreenManager : public TManager, public TClassFactory<TLRender::TScreen>
{
public:
	TScreenManager(TRefRef refManagerID);

	virtual TScreen*		CreateObject(TRefRef InstanceRef,TRefRef TypeRef);
};

