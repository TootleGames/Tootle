/*------------------------------------------------------
	Core include header

-------------------------------------------------------*/
#pragma once

#include "TLTypes.h"

// Forward declarations
class TString;	


namespace TLCore
{
	Bool			TootMain();						//	all-ecompassing loop if just using WinMain() and nothing clever
	Bool			TootUpdate();					//	manager update invoked from TootLoop

	Bool			TootInit();						//	engine init
	Bool			TootLoop(Bool InitResult);		//	engine update
	Bool			TootShutdown(Bool InitResult);	//	engine shutdown


	namespace Platform
	{
		SyncBool			Init();				//	platform init
		SyncBool			Update();			//	platform update
		SyncBool			Shutdown();			//	platform shutdown

		void				DoQuit();			// Notification of app quit
		const TString&		GetAppExe();		//	get the application exe (full path)
		
		void				OpenWebURL(TString& urlstr);
	}
	
};

//	include the platform specific header
#if defined(_MSC_EXTENSIONS)&&defined(TL_TARGET_PC)	//	dont include PC stuff when doing an ANSI build
	#include "PC/PCCore.h"
#endif

#if defined(TL_TARGET_IPOD)
	#include "IPod/IPodCore.h"
#endif


