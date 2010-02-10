/*------------------------------------------------------
	PC Core include header

-------------------------------------------------------*/
#pragma once

#ifndef _MSC_EXTENSIONS
	#error PC file should not be included in ansi builds
#endif

/*
//	include the wx headers
//	disable "This function or variable may be unsafe." warning which isn't corrected in wx yet
#pragma warning(disable : 4996)	
#include <wx/wx.h>

*/

//	include windows stuff
#define WIN32_LEAN_AND_MEAN
#define _WIN32_WINNT 0x0501	// specify the minimum windows OS we are supporting (0x0501 == Windows XP)

#ifndef _UNICODE
#define _UNICODE
#endif

#ifndef UNICODE
#define UNICODE
#endif

#include <math.h>
#include <stdio.h>
#include <typeinfo>

#include <windows.h>
#include <windowsx.h>




#include "../TLTypes.h"

//	forward declarations
class TString;
class TBinaryTree;


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
		FORCEINLINE u32			HandleWin32Message(u32 Message,WPARAM wParam,LPARAM lParam)		{	return 0;	}

		SyncBool			Init();				//	platform init
		SyncBool			Update();			//	platform update
		SyncBool			Shutdown();			//	platform shutdown
		void				Sleep(u32 Millisecs);	//	platform thread/process sleep

		void				DoQuit();			// Notification of app quit
		const TString&		GetAppExe();		//	get the application exe (full path)
		
		void				QueryHardwareInformation(TBinaryTree& Data);
		void				QueryLanguageInformation(TBinaryTree& Data);

	}
};


