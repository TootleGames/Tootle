/*------------------------------------------------------

	Ipod memory header - will be moved into it's own library eventually
 
-------------------------------------------------------*/
#pragma once

//	include low level ipod stuff
//#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>

namespace TLMemory
{
	namespace Platform
	{
		void Initialise();
		void Shutdown();

		extern NSAutoreleasePool*			g_pMemoryAutoReleasePool;
	}
}

