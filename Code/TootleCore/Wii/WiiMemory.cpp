/*------------------------------------------------------

	Wii memory code - Will be moved into it's own library eventually
 
-------------------------------------------------------*/

#include "WiiMemory.h"

namespace TLMemory
{
	namespace Platform
	{
	}
}


void TLMemory::Platform::Initialise()
{
}


void TLMemory::Platform::Shutdown()
{
}


void*	TLMemory::Platform::MemAlloc(u32 Size)								{	return malloc(Size);	}
void	TLMemory::Platform::MemDealloc(void* pMem)							{	free(pMem);	}	//	free
void	TLMemory::Platform::MemCopy(void* pDest,const void* pSrc,u32 Size)	{	memcpy( pDest, pSrc, Size );	}	//	memcpy
void	TLMemory::Platform::MemMove(void* pDest,const void* pSrc,u32 Size)	{	memmove( pDest, pSrc, Size );	}	//	memcpy
void*	TLMemory::Platform::MemRealloc(void* pMem,u32 Size)					{	realloc(pMem, Size);	}	//	realloc
void	TLMemory::Platform::MemValidate(void* pMem)							{	}
