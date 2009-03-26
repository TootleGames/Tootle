/*------------------------------------------------------
	Core include header

-------------------------------------------------------*/
#pragma once

#include "TLTypes.h"
#include "TRef.h"



namespace TLCore
{
	//	forward declaration of the generic app loop
	Bool			TootMain();			//	
	Bool			TootUpdate();		//	lib update

	//	useful functions
	u32				PointerToInteger(void* pAddress);	//	convert a pointer to an integer
	
	// DB - Global TRef's that get used often.  Performance optimisation
	const TRef InitialiseRef	= "Initialise";
	const TRef UpdateRef		= "Update";
	const TRef RenderRef		= "Render";
	const TRef ShutdownRef		= "Shutdown";
	const TRef TimeStepRef		= "Timestep";
	const TRef TimeStepModRef	= "TSMod";
	const TRef QuitRef			= "Quit";

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


