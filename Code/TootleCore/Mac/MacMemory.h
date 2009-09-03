/*------------------------------------------------------

	Ipod memory header - will be moved into it's own library eventually
 
-------------------------------------------------------*/
#pragma once

//	include low level ipod stuff
//#import <Foundation/Foundation.h>
//#import <UIKit/UIKit.h>

#include "MacCore.h"

#include <stdlib.h>	//	needed for malloc
#include <string.h>	//	needed for memcpy


namespace TLMemory
{
	namespace Platform
	{
		void Initialise();
		void Shutdown();
		
		FORCEINLINE void*	MemAlloc(u32 Size)								{	return malloc( Size );	}
		FORCEINLINE void	MemDealloc(void* pMem)							{	free( pMem );	}
		FORCEINLINE void	MemCopy(void* pDest,const void* pSrc,u32 Size)	{	memcpy( pDest, pSrc, Size );	}
		FORCEINLINE void	MemMove(void* pDest,const void* pSrc,u32 Size)	{	memmove( pDest, pSrc, Size );	}
		FORCEINLINE void	MemValidate(void* pMem)							{	}
		
	}
}

