/*------------------------------------------------------

	PC memory header - will be moved into it's own library eventually
 
-------------------------------------------------------*/
#pragma once

#include "PCCore.h"


namespace TLMemory
{
	namespace Platform
	{
		extern HANDLE	g_MemHeap;

		void			Initialise();
		void			Shutdown();
//		inline void*	MemAlloc(u32 Size)								{	return malloc( Size );	}			//	malloc
//		inline void		MemDealloc(void* pMem)							{	free( pMem );	}					//	free
		inline void*	MemAlloc(u32 Size)								{	return HeapAlloc( g_MemHeap, 0x0, Size );	}
		inline void		MemDealloc(void* pMem)							{	HeapFree( g_MemHeap, 0x0, pMem );	}	//	free
		inline void		MemCopy(void* pDest,const void* pSrc,u32 Size)	{	memcpy( pDest, pSrc, Size );	}	//	memcpy
		inline void		MemMove(void* pDest,const void* pSrc,u32 Size)	{	memmove( pDest, pSrc, Size );	}	//	memcpy
		inline void		MemValidate(void* pMem)							{	HeapValidate( g_MemHeap, 0x0, pMem );	}

	}
}

