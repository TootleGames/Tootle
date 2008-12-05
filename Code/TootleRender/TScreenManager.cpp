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
	if ( TypeRef == "Screen" )
	{
		return new TLRender::Platform::Screen( InstanceRef );
	}

	return NULL;
}