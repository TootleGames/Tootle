/*------------------------------------------------------

	

-------------------------------------------------------*/
#pragma once
#include "PCGui.h"
#include "../TApp.h"




#if !defined(TL_ENABLE_WX)

namespace TLGui
{
	namespace Platform
	{
		class App;
		extern HINSTANCE	g_HInstance;	//	gr: hopefully we can remove the need for the HInstance at some point
	}
}

class TLGui::Platform::App : public TLGui::TApp
{
public:
	App()		{}
	
	Bool			Init();
	SyncBool		Update();
	SyncBool		Shutdown();
};


#endif // !TL_ENABLE_WX


