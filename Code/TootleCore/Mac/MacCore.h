/*------------------------------------------------------

	Ipod core header
 
-------------------------------------------------------*/
#pragma once

#include "../TLTypes.h"

//	include low level ipod stuff
#import <Foundation/Foundation.h>


//	forward declarations
class TString;
class TBinaryTree;


namespace TLDebug
{
	namespace Platform
	{
		void		Print(const TString& String);	//	platform specific debug output
		Bool		Break(const TString& String);	//	return FALSE to stop app, TRUE and will attempt to continue
	}
};

namespace TLMemory
{
	namespace Platform
	{
//		inline void*		MemAlloc(u32 Size)								{	return malloc( Size );	}			//	malloc
//		inline void			MemDealloc(void* pMem)							{	free( pMem );	}					//	free
//		inline void			MemCopy(void* pDest,const void* pSrc,u32 Size)	{	memcpy( pDest, pSrc, Size );	}	//	memcpy
//		inline void			MemMove(void* pDest,const void* pSrc,u32 Size)	{	memmove( pDest, pSrc, Size );	}	//	memcpy
		void*			MemAlloc(u32 Size);					//	malloc
		void			MemDealloc(void* pMem);				//	free
		void			MemCopy(void* pDest,const void* pSrc,u32 Size);	//	memcpy
		void			MemMove(void* pDest,const void* pSrc,u32 Size);	//	memcpy
	}
}

namespace TLTime
{
	class TTimestamp;
	
	namespace Platform
	{
		SyncBool			Init();				//	time init
	}
}

namespace TLCore
{
	namespace Platform
	{
		SyncBool			Init();				//	platform init
		SyncBool			Update();			//	platform update
		SyncBool			Shutdown();			//	platform shutdown
		
		void				DoQuit();			// Notification of app quit
		const TString&		GetAppExe();		//	get the application exe (full path)

		//	ipoddy specific funcs
		void				GetString(TString& String, const NSString* pNSString);	//	append ipod foundation string to TString
		
		void				QueryHardwareInformation(TBinaryTree& Data);
		void				QueryLanguageInformation(TBinaryTree& Data);

	}
}
