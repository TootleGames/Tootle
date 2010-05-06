/*
 *  MacGui.cpp
 *  TootleGui
 *
 *  Created by Graham Reeves on 17/02/2010.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#include "MacGui.h"
#include "MacApp.h"
#include "MacWindow.h"
#include <TootleCore/Mac/MacString.h>

//	needed to get desktop size
#import <Foundation/Foundation.h>
#import <Appkit/Appkit.h>




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
#if defined(TL_ENABLE_WX)
	return wx::GetScreenMousePosition( Window );
#else	

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
#endif // TL_ENABLE_WX
}



//----------------------------------------------------------
//	get the desktop dimensions. note: need a window so we can decide which desktop?
//----------------------------------------------------------
void TLGui::Platform::GetDesktopSize(Type4<s32>& DesktopSize)
{
	DesktopSize.x = 0;
	DesktopSize.y = 0;
	
	NSRect screenRect = [[NSScreen mainScreen] frame];
	
	DesktopSize.Width() = screenRect.size.width;
	DesktopSize.Height() = screenRect.size.height;
}




//---------------------------------------------------
//	app entry
//---------------------------------------------------
int main(int argc, char *argv[])
{
	//	get the root directory that the app is in
	NSString *HomeDir = NSHomeDirectory();
	TTempString HomeDirString;
	HomeDirString << HomeDir;
	TLGui::SetAppExe( HomeDirString );

	//	create and assign the tootle app delegate
	[NSApplication sharedApplication];
	[NSApp setDelegate: [TootleNSApplicationDelegate new]];
	
	//	start application
	int retVal = NSApplicationMain(argc,  (const char **) argv);
	return retVal;
	
	/*	
	 [TootleNSApplication sharedApplication];
	 
	 [NSBundle loadNibNamed:@"MainMenu" owner:NSApp];
	 
	 [NSApp run];
	 
	 return 0;
	 */
}