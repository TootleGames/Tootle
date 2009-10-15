/*
 *  MacDebug.mm
 *  TootleCore
 *
 *  Created by Duane Bradbury on 14/10/2009.
 *  Copyright 2009 Tootle. All rights reserved.
 *
 */

#include "MacDebug.h"


#include "../TString.h"

#import <Cocoa/Cocoa.h>



// Internal routines and variables
namespace TLDebug
{
	namespace Platform
	{
		static TTempString			consolebuffer;
		
		void	FlushBuffer();
		
		void	Print(const TString& String);	//	platform specific debug output - immediate

	}
}



SyncBool TLDebug::Platform::Initialise()
{
	return SyncTrue;
}

SyncBool TLDebug::Platform::Shutdown()
{
	FlushBuffer();
	return SyncTrue;
}



void TLDebug::Platform::PrintToBuffer(const TString& String)
{
	// Check to see if we can add to the buffer (less one for the terminator)
	if(consolebuffer.GetLength() + String.GetLength() < 511)
	{
		consolebuffer.Appendf("\n%s", String.GetData());	
	}
	else 
	{
		// will excedd buffer length so flush the buffer and start fresh
		FlushBuffer();
		
		consolebuffer.Appendf("\n%s", String.GetData());	
	}
}


void TLDebug::Platform::FlushBuffer()
{
	Print(consolebuffer);
	
	consolebuffer.Empty();
}


//--------------------------------------------------
//	platform specific debug text output
//--------------------------------------------------
void TLDebug::Platform::Print(const TString& String)
{
	NSString *logString = [[NSString alloc] initWithUTF8String: String.GetData()];
	NSLog(@"%@", logString );
	[logString release];
	
	//	printf( String.GetData() );
	//	printf("\n");
}


//--------------------------------------------------
//	return FALSE to stop app, TRUE and will attempt to continue
//--------------------------------------------------
Bool TLDebug::Platform::Break(const TString& String)
{
	FlushBuffer();
	Print( String );
	
	// Drop into debugger if possible
	// DB:	Found solution to getting the CoreServices linked *only* for the iphone simulator build
	//		In the target's info goto the Linking->Other Link Flags option and add -framework CoreServices
	//		via the cog at the bottom of the pane (add build setting condition) then select Any iPhone Simulator for the SDK option
	// and this should link correctly only on a simulator build :)
#if !TARGET_OS_IPHONE || TARGET_IPHONE_SIMULATOR
	Debugger();
#elif defined(_DEBUG)
	//	gr: hacky break for debug builds
	//	gr: removed because there's no way to get out of this in xcode if we trigger it..
	int* pNull = 0x0;
	//*pNull = 99;
#endif
	
	//	fail
	return FALSE;
}