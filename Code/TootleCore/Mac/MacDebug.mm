/*
 *  MacDebug.mm
 *  TootleCore
 *
 *  Created by Duane Bradbury on 14/10/2009.
 *  Copyright 2009 Tootle. All rights reserved.
 *
 */

#include "MacDebug.h"
#include "MacString.h"

#import <Cocoa/Cocoa.h>

#define DEBUG_CONSOLE_BUFFER_SIZE 512

#if defined(TL_USER_GRAHAM)
	#define TL_DEBUG_AUTOFLUSH
#endif

// Internal routines and variables
namespace TLDebug
{
	namespace Platform
	{
		static TBufferString<DEBUG_CONSOLE_BUFFER_SIZE>	g_ConsoleBuffer;
		
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
	// Final buffer size is buffer size less the size of a newline special character '\n' and 
	// a terminator '\0' which would be added automatically 
	u32 uSize = g_ConsoleBuffer.GetAllocSize() - 4; 
	
	// Check to see if we can add to the buffer
	if(g_ConsoleBuffer.GetLength() + String.GetLength() >= uSize)
		FlushBuffer(); // will exceed the buffer length so flush the buffer and start fresh
	
	g_ConsoleBuffer.Append( String.GetData() );
	g_ConsoleBuffer.Append( "\n" );
	
#if defined(TL_DEBUG_AUTOFLUSH)
	FlushBuffer();
#endif	
}


void TLDebug::Platform::FlushBuffer()
{
	Print(g_ConsoleBuffer);
	
	g_ConsoleBuffer.Empty();
}


//--------------------------------------------------
//	platform specific debug text output
//--------------------------------------------------
void TLDebug::Platform::Print(const TString& String)
{
	NSString *logString = TLString::ConvertToUnicharString(String);
	NSLog(@"%@", logString );
	[logString release];
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