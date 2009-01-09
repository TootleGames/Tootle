/*------------------------------------------------------
	Memory manager functions
	Currently wrappers for common

	TYPE	the arrays object type
	SORTED	true/false, inserts objects in order 
			benefit is that searches are quicker
			requires the > operator
			array is always ascending

-------------------------------------------------------*/
#pragma once

#include "TLCore.h"

#if defined(__GNUG__)
	#include <new>
#else
	#include <new.h>
#endif



namespace TLMemory
{
	template<typename TYPE>	FORCEINLINE TYPE*	AllocArray(u32 Elements)				{	return new TYPE[ Elements ];	}	//	alloc an array of objects
	template<typename TYPE> FORCEINLINE void	Delete(TYPE*& pData);					//	delete an object if not-null and NULLs the pointer provided
	template<typename TYPE> FORCEINLINE void	DeleteArray(TYPE*& pData);				//	delete an array if not-null and NULLs the pointer provided
	template<typename TYPE> FORCEINLINE void	CopyData(TYPE* pToData,const TYPE* pFromData,u32 Elements);	//	(memcpy) copy a load of raw data
	template<typename TYPE> FORCEINLINE void	MoveData(TYPE* pToData,const TYPE* pFromData,u32 Elements);	//	(memmove) move a load of raw data


	//	added this wrapper for TLDebug_Break so we don't include the string type or debug header
	namespace Debug
	{
		Bool				Break(const char* pErrorString,const char* pSourceFunction);
		void				Debug_Alloc(void* pDataAddress,u32 Size);	//	log the allocation of some data (POST alloc)
		Bool				Debug_Delete(void* pDataAddress);			//	log/debug deleting of data, return FALSE to abort the delete (PRE delete)
	}

	//	platform implemenations
	namespace Platform
	{
		void		Initialise();
		void		Shutdown();
		
		void*		MemAlloc(u32 Size);								//	malloc
		void		MemDealloc(void* pMem);							//	free
		void		MemCopy(void* pDest,const void* pSrc,u32 Size);	//	memcpy
		void		MemMove(void* pDest,const void* pSrc,u32 Size);	//	memmove
		void		MemValidate(void* pMem=NULL);					//	validate memory heaps
	}

	class TMemoryTrack;								//	allocation tracking entry
	class TMemorySystem;							//	memory interface...
	extern TMemorySystem	g_sMemorySystem;		//	global static memory system
};




class TLMemory::TMemoryTrack
{
public:
	TMemoryTrack()						:	m_Address( 0 ), m_Size( 0 )				{}
	TMemoryTrack(u32 Address,u32 Size)	:	m_Address( Address ), m_Size( Size )	{}
	TMemoryTrack(void* pAddress,u32 Size) :	m_Address( TLCore::PointerToInteger( pAddress ) ), m_Size( Size )	{}

	FORCEINLINE Bool	operator==(const u32& Address) const	{	return (m_Address == Address);	}
	FORCEINLINE Bool	operator<(const u32& Address) const		{	return (m_Address < Address);	}

public:
	u32				m_Address;	//	pointer address
	u32				m_Size;		//	size of data we allocated
};


class TLMemory::TMemorySystem
{
public:
	TMemorySystem();
	~TMemorySystem();

	// Custom memory allocation
	FORCEINLINE void*		Allocate(u32 Size)
	{
		void* pData = Platform::MemAlloc( Size );
		if ( !pData )
		{
			TLMemory::Debug::Break("Failed to allocate memory", __FUNCTION__ );
			return NULL;
		}

		//	do memory debugging/logging
		//	this is per-lib
		#ifdef _DEBUG
		{
			TLMemory::Debug::Debug_Alloc( pData, Size );
		}
		#endif

		return pData;
		/*
		void* pmem = malloc(size);

		while(!pmem)
		{
			// no out of memory handler?  throw exception error
			if(!_get_new_handler())
				throw std::bad_alloc;
			
			// call out of memory handler - may be able to free some memory
			_get_new_handler(size);

			// try and allocate again...
			pmem = malloc(size);
		}

		return pmem;
		*/
	};

	// Custom memory deallocation
	FORCEINLINE void	Deallocate(void* pObj)
	{
		if ( !pObj )
			return;

		//	do memory debugging/logging
		//	this is per-lib
		//	gr: if this fails, we abort the deleting, just to stop breaking the memory system if we've detected a problem
		#ifdef _DEBUG
		{
			if ( !TLMemory::Debug::Debug_Delete( pObj ) )
				return;
		}
		#endif

		//	delete memory
		Platform::MemDealloc( pObj );
	}

};


//------------------------------------------------
//	Oveloaded new and delete operators which call the memory system allocation
//------------------------------------------------
/*
void* operator new(size_t size);
void* operator new[](size_t size);

void operator delete(void* pObj);
void operator delete[](void* pObj);
*/

#if defined(__GNUG__)
	#define throwfunc(exceptionfunc)	throw (exceptionfunc)
#else
	#define	throwfunc(exceptionfunc)	throw ()
#endif


FORCEINLINE void* operator new(std::size_t size) throwfunc(std::bad_alloc)
{
	return TLMemory::g_sMemorySystem.Allocate( size );
}

FORCEINLINE void* operator new[](std::size_t size) throwfunc(std::bad_alloc)
{
	return TLMemory::g_sMemorySystem.Allocate( size );
}

FORCEINLINE void operator delete(void* pObj) throw()
{
	return TLMemory::g_sMemorySystem.Deallocate( pObj );
}

FORCEINLINE void operator delete[](void* pObj) throw()
{
	return TLMemory::g_sMemorySystem.Deallocate( pObj );
}


//	include the platform specific header
#if defined(TL_TARGET_PC)
	#include "PC/PCMemory.h"
#endif


#include "TLMemory.inc.h"
