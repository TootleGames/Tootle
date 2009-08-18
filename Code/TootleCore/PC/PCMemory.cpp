/*------------------------------------------------------

	PC memory code - Will be moved into it's own library eventually
 
-------------------------------------------------------*/

#include "PCMemory.h"

#include "PCCore.h"

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


void*	TLMemory::Platform::MemAlloc(u32 Size)								{	return HeapAlloc( g_MemHeap, 0x0, Size );	}
void	TLMemory::Platform::MemDealloc(void* pMem)							{	HeapFree( g_MemHeap, 0x0, pMem );	}	//	free
void	TLMemory::Platform::MemCopy(void* pDest,const void* pSrc,u32 Size)	{	memcpy( pDest, pSrc, Size );	}	//	memcpy
void	TLMemory::Platform::MemMove(void* pDest,const void* pSrc,u32 Size)	{	memmove( pDest, pSrc, Size );	}	//	memcpy
void	TLMemory::Platform::MemValidate(void* pMem)							{	HeapValidate( g_MemHeap, 0x0, pMem );	}
