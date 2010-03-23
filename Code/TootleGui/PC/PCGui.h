/*
 *  MacGui.h
 *  TootleGui
 *
 *  Created by Graham Reeves on 17/02/2010.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */
#include "../TLGui.h"


#ifndef _MSC_EXTENSIONS
	#error PC file should not be included in ansi builds
#endif


#if defined(TL_ENABLE_WX)

//	include the wx headers
//	disable "This function or variable may be unsafe." warning which isn't corrected in wx yet
//#pragma warning(disable : 4996)	
#include <wx/wx.h>


//	gr: move all these lib links to TLGui
//	link to wx libs
#if defined(_DEBUG)
	//	*ud libs (unicode debug)
	#pragma comment(lib,"../../../Tootle/Code/wxWidgets/lib/vc_lib/wxbase29ud.lib")
	#pragma comment(lib,"../../../Tootle/Code/wxWidgets/lib/vc_lib/wxmsw29ud_core.lib")
	#pragma comment(lib,"../../../Tootle/Code/wxWidgets/lib/vc_lib/wxmsw29ud_adv.lib")
#else
	//	*u libs (unicode)
	#pragma comment(lib,"../../../Tootle/Code/wxWidgets/lib/vc_lib/wxbase29u.lib")
	#pragma comment(lib,"../../../Tootle/Code/wxWidgets/lib/vc_lib/wxmsw29u_core.lib")
	#pragma comment(lib,"../../../Tootle/Code/wxWidgets/lib/vc_lib/wxmsw29u_adv.lib")
#endif

#else // TL_ENABLE_WX

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
#include <string.h>
#include <typeinfo>
#include <windows.h>
#include <windowsx.h>
#include <mmsystem.h>

#endif

namespace TLGui
{
	namespace Platform
	{
		SyncBool		Init();
		SyncBool		Shutdown();

		int2			GetScreenMousePosition(TLGui::TWindow& Window,u8 MouseIndex);
		void			GetDesktopSize(Type4<s32>& DesktopSize);	//	get the desktop dimensions. note: need a window so we can decide which desktop?	
	}

}


