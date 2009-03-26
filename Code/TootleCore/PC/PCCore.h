/*------------------------------------------------------
	PC Core include header

-------------------------------------------------------*/
#pragma once

#ifndef _MSC_EXTENSIONS
	#error PC file should not be included in ansi builds
#endif

#include "../TLTypes.h"

//	include windows stuff
#define WIN32_LEAN_AND_MEAN
#define _WIN32_WINNT 0x0501	// specify the minimum windows OS we are supporting (0x0501 == Windows XP)


#include <math.h>
#include <stdio.h>
#include <typeinfo>

#include <windows.h>
#include <windowsx.h>


//	forward declarations
class TString;


namespace TLDebug
{
	namespace Platform
	{
		void		Print(const TString& String);	//	platform specific debug output
		Bool		Break(const TString& String);	//	return FALSE to stop app, TRUE and will attempt to continue
		Bool		CheckWin32Error();				//	checks for a win32 error and does a break
	}
};


namespace TLTime
{
	class TTimestamp;

	namespace Platform
	{
		SyncBool			Init();				//	time init
	}
}

namespace TLCore
{
	namespace Platform
	{
		extern HINSTANCE	g_HInstance;
		extern HWND			g_HWnd;
		inline u32			HandleWin32Message(u32 Message,WPARAM wParam,LPARAM lParam)		{	return 0;	}

		SyncBool			Init();				//	platform init
		SyncBool			Update();			//	platform update
		SyncBool			Shutdown();			//	platform shutdown
		FORCEINLINE void	Sleep(u32 Millisecs);	//	platform thread/process sleep

		void				DoQuit();			// Notification of app quit
		const TString&		GetAppExe();		//	get the application exe (full path)
	}
}



//--------------------------------------------------
//	platform thread/process sleep
//--------------------------------------------------
FORCEINLINE void TLCore::Platform::Sleep(u32 Millisecs)
{
	::Sleep( Millisecs );
}


