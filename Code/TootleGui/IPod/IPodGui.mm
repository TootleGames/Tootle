/*
 *  IPodGui.mm
 *  TootleGui
 *
 *  Created by Graham Reeves on 17/02/2010.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#include "IPodGui.h"
#include <TootleCore/IPod/IPodString.h>
#include <TootleInput/IPod/IPodInput.h>



int2 TLGui::Platform::GetScreenMousePosition(TLGui::TWindow& Window,u8 MouseIndex)
{
	if ( MouseIndex >= TLInput::Platform::IPod::g_aCursorPositions.GetSize() )
		return int2(-1,-1);
	
	// Get cursor position from array
	return TLInput::Platform::IPod::g_aCursorPositions[uIndex];
}



//----------------------------------------------------------
//	get the desktop dimensions. note: need a window so we can decide which desktop?
//----------------------------------------------------------
void TLGui::Platform::GetDesktopSize(Type4<s32>& DesktopSize)
{
	//	gr: this will just be the resolution, but we need to know if it's
	//		wide or portrait... so maybe we need the screen here?
	//	do this properly!

	//NSRect Frame = [[UIScreen mainScreen] bounds];
	CGRect ScreenRect = [[UIScreen mainScreen] applicationFrame];
	DesktopSize.x = ScreenRect.origin.x;
	DesktopSize.y = ScreenRect.origin.y;
	DesktopSize.Width() = ScreenRect.size.width;
	DesktopSize.Height() = ScreenRect.size.height;

/* mac:
	DesktopSize.x = 0;
	DesktopSize.y = 0;
	
	NSRect screenRect = [[NSScreen mainScreen] frame];
	DesktopSize.Width() = screenRect.size.width;
	DesktopSize.Height() = screenRect.size.height;
*/
}





//----------------------------------------------------
//	init platform implementation
//----------------------------------------------------
SyncBool TLGui::Platform::Init()
{
	return SyncTrue;
}


//----------------------------------------------------
//	shutdown platform implementation
//----------------------------------------------------
SyncBool TLGui::Platform::Shutdown()
{
	return SyncTrue;
}


//---------------------------------------------------------------
//	get the cursor position in the default screen's client space
//---------------------------------------------------------------
int2 TLGui::Platform::GetScreenMousePosition(TLGui::TWindow& Window,u8 MouseIndex)
{
#define USE_MOUSELOC_OUTSIDE_OF_EVENT	//	this IS defined in macinput.mm
	
	TLGui::Platform::Window& PlatformWindow = static_cast<TLGui::Platform::Window&>( Window );
	NSWindow* pWindow = PlatformWindow.m_pWindow;
	
	NSRect FrameRect = [pWindow frame];
	NSRect ContentRect = [pWindow contentRectForFrameRect:FrameRect];
		
	TTempString Debug_String;

	// Get mouse pos relative to window
	NSPoint pos = [pWindow mouseLocationOutsideOfEventStream];
	
	//	osx window coords from bttom left so invert y
	pos.y = ContentRect.size.height - pos.y;
	Debug_String.Appendf("Mouse pos (%.2f, %.2f)", pos.x, pos.y);
		
//	NSPoint windowpos = [window convertScreenToBase:pos];
		
	TLDebug_Print( Debug_String );
	
	return int2( (int)pos.x, (int)pos.y );
}



//---------------------------------------------------
//	app entry
//---------------------------------------------------
int main(int argc, char *argv[])
{
	//	get the root directory that the app is in
	NSString *HomeDir = NSHomeDirectory();
	TTempString HomeDirString;
	HomeDirString << HomeDir << "/myapp.app"

//	int retVal = UIApplicationMain(argc, argv, nil, [TAppDelegate class]);	//	
	int retVal = UIApplicationMain(argc, argv, nil, @"TAppDelegate");	//	[TAppDelegate class]
//	int retVal = UIApplicationMain(argc, argv, nil, nil);
		
	return retVal;
}

