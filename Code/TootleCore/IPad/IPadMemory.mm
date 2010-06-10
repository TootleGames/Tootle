/*
 *  IPadMemory.mm
 *  TootleCore
 *
 *	Ipad memory code - Will be moved into it's own library eventually
 *
 *  Created by Duane Bradbury on 15/12/2009.
 *  Copyright 2009 Tootle. All rights reserved.
 *
 */

#include "IPadMemory.h"

//	include low level ipod stuff
#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>



namespace TLMemory
{
	namespace Platform
	{
		NSAutoreleasePool*			g_pMemoryAutoReleasePool = NULL;
	}
}



void TLMemory::Platform::Initialise()
{
	g_pMemoryAutoReleasePool = [[NSAutoreleasePool alloc] init];
}

void TLMemory::Platform::Shutdown()
{
	[TLMemory::Platform::g_pMemoryAutoReleasePool release];	
}


