/*------------------------------------------------------

	PC memory code - Will be moved into it's own library eventually
 
-------------------------------------------------------*/

#include "PCMemory.h"


namespace TLMemory
{
	namespace Platform
	{
		HANDLE	g_MemHeap = INVALID_HANDLE_VALUE;
	}
}


void TLMemory::Platform::Initialise()
{
	g_MemHeap = HeapCreate( 0x0, 0, 0 );
}


void TLMemory::Platform::Shutdown()
{
	if ( g_MemHeap != INVALID_HANDLE_VALUE )
	{
		HeapDestroy( g_MemHeap );
		g_MemHeap = INVALID_HANDLE_VALUE;
	}
}


