/*
 *  IPadMemory.h
 *  TootleCore
 *
 *	Ipad memory header - will be moved into it's own library eventually
 *
 *  Created by Duane Bradbury on 15/12/2009.
 *  Copyright 2009 Tootle. All rights reserved.
 *
 */

#pragma once

#include "IPadCore.h"
#include <cstdlib>	//	needed for malloc and size_t
#include <string.h>	//	needed for memcpy


namespace TLMemory
{
	namespace Platform
	{
		//extern NSAutoreleasePool*			g_pMemoryAutoReleasePool;

		void Initialise();
		void Shutdown();
				
		FORCEINLINE void*	MemAlloc(u32 Size)								{	return malloc( Size );	}
		FORCEINLINE void	MemDealloc(void* pMem)							{	free( pMem );	}
		FORCEINLINE void	MemCopy(void* pDest,const void* pSrc,u32 Size)	{	memcpy( pDest, pSrc, Size );	}
		FORCEINLINE void	MemMove(void* pDest,const void* pSrc,u32 Size)	{	memmove( pDest, pSrc, Size );	}
		FORCEINLINE void	MemValidate(void* pMem)							{	}

		FORCEINLINE std::size_t		MemSize(void* pMem)						{	return sizeof(pMem); }

		FORCEINLINE void	MemOuputAllocations()							{}

#ifdef _DEBUG
		FORCEINLINE void	MemFillPattern(void* pMem, u32 Size, u8 Pattern) { memset(pMem, Pattern, Size); }
#else
		FORCEINLINE void	MemFillPattern(void* pMem, u32 Size, u8 Pattern)	{}
#endif

	}
}


