/*
 *  IPadGui.mm
 *  TootleGui
 *
 *  Created by Graham Reeves on 17/02/2010.
 *  Copyright 2010 Tootle. All rights reserved.
 *
 */

#include "IPadGui.h"
#include "IPadWindow.h"
#include <TootleCore/IPad/IPadString.h>
#include <TootleInput/IPad/IPadInput.h>
#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>


int2 TLGui::Platform::GetScreenMousePosition(TLGui::TWindow& Window,u8 MouseIndex)
{
	// Get cursor position from array
	return TLInput::Platform::IPod::GetCursorPosition(MouseIndex);
}



//----------------------------------------------------------
//	get the desktop dimensions. note: need a window so we can decide which desktop?
//----------------------------------------------------------
void TLGui::Platform::GetDesktopSize(Type4<s32>& DesktopSize)
{
	//	gr: this will just be the resolution, but we need to know if it's
	//		wide or portrait... so maybe we need the screen here?
	//	do this properly!

	//CGRect Bounds = [[UIScreen mainScreen] bounds];
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


#import "IPadApp.h"


//---------------------------------------------------
//	app entry
//---------------------------------------------------
int main(int argc, char *argv[])
{
	//NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];

	[TAppDelegate RedundantJustToEnsureSymbolExported];
	
//	int retVal = UIApplicationMain(argc, argv, nil, [TAppDelegate class]);	//	
	int retVal = UIApplicationMain(argc, argv, nil, @"TAppDelegate");	//	[TAppDelegate class]
//	int retVal = UIApplicationMain(argc, argv, nil, nil);
	
	//[pool release];
	
	return retVal;
}

