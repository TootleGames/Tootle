/*
 *  IPadDebug.mm
 *  TootleCore
 *
 *  Created by Duane Bradbury on 14/10/2009.
 *  Copyright 2009 Tootle. All rights reserved.
 *
 */

#include "IPadDebug.h"
#include "IPadString.h"

#import <UIKit/UIKit.h>


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
	// Clear out the buffer contents with final print
	FlushBuffer();

	return SyncTrue;
}


void TLDebug::Platform::PrintToBuffer(const TString& String)
{
	// Check to see if we can add to the buffer (less one for the terminator)
	if(consolebuffer.GetLength() + String.GetLength() >= consolebuffer.GetMaxAllocSize()-1 )
	{
		// will excedd buffer length so flush the buffer and start fresh
		FlushBuffer();
	}
	
	//	append to console buffer
	consolebuffer << '\n' << String;	
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
	
	/*
	 // Create an alert so that we can skip debugger or breaking
	 NSString* errornsstring = [[NSString alloc] initWithCString:String.GetData() ];
	 
	 AlertsViewController *alertsViewController = [[AlertsViewController alloc] init];
	 [alertsViewController dialogueOKCancelAction: errornsstring];
	 
	 // Wait for the error dialogue to be dismissed
	 SyncBool Res = SyncWait;
	 
	 do
	 {
	 Res = [alertsViewController dialogueResult];
	 } while(Res == SyncWait);
	 
	 [errornsstring release];
	 [alertsViewController release];	
	 */
	
	// Drop into debugger if possible
	// DB:	Found solution to getting the CoreServices linked *only* for the iphone simulator build
	//		In the target's info goto the Linking->Other Link Flags option and add -framework CoreServices
	//		via the cog at the bottom of the pane (add build setting condition) then select Any iPhone Simulator for the SDK option
	// and this should link correctly only on a simulator build :)
#if !TARGET_OS_IPHONE || TARGET_IPHONE_SIMULATOR
	// NOTE: CoreSerives is now not available in either the deice or simulator (used to be simulator only)
	//Debugger();
#elif defined(_DEBUG)

	// assert will kill off the application therefore doesn't continue :(
	//assert(FALSE);
	
	//	gr: new method, untested, see https://devforums.apple.com/message/99580
	/*
	 __builtin_trap() is one option. If the debugger is running then you'll stop in the debugger, 
	 otherwise you'll crash with a crash log. But there's no guarantee that you can tell the debugger 
	 to continue running your program after that - the compiler thinks __builtin_trap() halts the process 
	 so it may optimize away any code after it.
	 
	 asm("trap") or asm("int3") is an architecture-specific option. The behavior is the same as __builtin_trap(), 
	 except the compiler doesn't optimize around it so you should be able to continue running afterwards.
	 */
	// Device only - use asm("int3") for simulator builds.
	// NOTE: Can't continue from this asm instruction :(
	//asm("trap");
	
	// [18/05/10] DB - Found a new type of break that seems to work, kicking the device into the debugger and allowing it to continue
	// See: http://stackoverflow.com/questions/1149113/breaking-into-the-debugger-on-iphone
	kill( getpid(), SIGINT ) ;
	
#endif
	
	
	//	fail
	return FALSE;
}
