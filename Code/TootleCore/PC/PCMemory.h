/*------------------------------------------------------

	PC memory header - will be moved into it's own library eventually
 
-------------------------------------------------------*/
#pragma once

#include "PCCore.h"


namespace TLMemory
{
	namespace Platform
	{
		extern HANDLE		g_MemHeap;

		void				Initialise();
		void				Shutdown();

		FORCEINLINE void*	MemAlloc(u32 Size)								{	return HeapAlloc( g_MemHeap, 0x0, Size );	}
		FORCEINLINE void	MemDealloc(void* pMem)							{	HeapFree( g_MemHeap, 0x0, pMem );	}	//	free
		FORCEINLINE void	MemCopy(void* pDest,const void* pSrc,u32 Size)	{	memcpy( pDest, pSrc, Size );	}	//	memcpy
		FORCEINLINE void	MemMove(void* pDest,const void* pSrc,u32 Size)	{	memmove( pDest, pSrc, Size );	}	//	memcpy
		FORCEINLINE void	MemValidate(void* pMem)							{	HeapValidate( g_MemHeap, 0x0, pMem );	}
	}
}

