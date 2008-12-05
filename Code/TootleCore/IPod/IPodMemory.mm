/*------------------------------------------------------

	Ipod memory code - Will be moved into it's own library eventually
 
-------------------------------------------------------*/

#include "IPodMemory.h"

//#import <UIKit/UIKit.h>

namespace TLMemory
{
	namespace Platform
	{
		NSAutoreleasePool*			g_pMemoryAutoReleasePool = NULL;
	}
}

using namespace TLMemory;

void Platform::Initialise()
{
	g_pMemoryAutoReleasePool = [[NSAutoreleasePool alloc] init];
}

void Platform::Shutdown()
{
	[TLMemory::Platform::g_pMemoryAutoReleasePool release];	
}


