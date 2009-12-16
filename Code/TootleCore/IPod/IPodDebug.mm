/*
 *  IPodDebug.mm
 *  TootleCore
 *
 *  Created by Duane Bradbury on 14/10/2009.
 *  Copyright 2009 Tootle. All rights reserved.
 *
 */

#include "IPodDebug.h"
#include "IPodString.h"

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
	if(consolebuffer.GetLength() + String.GetLength() < 511)
	{
		consolebuffer.Appendf("\n%S", String.GetData());	
	}
	else 
	{
		// will excedd buffer length so flush the buffer and start fresh
		FlushBuffer();
		
		consolebuffer.Appendf("\n%S", String.GetData());	
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
	Debugger();
#elif defined(_DEBUG)
	//	gr: hacky break for debug builds
	//	gr: removed because there's no way to get out of this in xcode if we trigger it..
	int* pNull = 0x0;
	//*pNull = 99;
	
	
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
	//__builtin_trap();
	//asm("int3");		//	note: same as PC version
	
#endif
	
	
	//	fail
	return FALSE;
}
