/*------------------------------------------------------

	PC memory header - will be moved into it's own library eventually
 
-------------------------------------------------------*/
#pragma once

#include "../TLTypes.h"

namespace TLMemory
{
	namespace Platform
	{
		void				Initialise();
		void				Shutdown();

		void*	MemAlloc(u32 Size);
		void	MemDealloc(void* pMem);
		void	MemCopy(void* pDest,const void* pSrc,u32 Size);
		void	MemMove(void* pDest,const void* pSrc,u32 Size);
		void	MemValidate(void* pMem);
	}
}

