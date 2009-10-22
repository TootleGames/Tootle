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

#include "TLCoreMisc.h"
//#include "TLTypes.h"

#if defined(__GNUG__)
	#include <new>
#else
	#include <cstddef>
	#include <new.h>
#endif


// Memory system callback definition
// Used for calling a routine at strategic points during allocation and deallocation
typedef void (*memcallback)();


//#define ENABLE_SOA


#ifdef ENABLE_SOA
#include "TSmallObjectAllocator.h"
#endif

namespace TLMemory
{
	template<typename TYPE>	FORCEINLINE TYPE*	AllocArray(u32 Elements)				{	return new TYPE[ Elements ];	}	//	alloc an array of objects
	template<typename TYPE> FORCEINLINE void	Delete(TYPE*& pData);					//	delete an object if not-null and NULLs the pointer provided
	template<typename TYPE> FORCEINLINE void	DeleteArray(TYPE*& pData);				//	delete an array if not-null and NULLs the pointer provided
	template<typename TYPE> FORCEINLINE void	CopyData(TYPE* pToData,const TYPE* pFromData,u32 Elements);	//	(memcpy) copy a load of raw data
	template<typename TYPE> FORCEINLINE void	MoveData(TYPE* pToData,const TYPE* pFromData,u32 Elements);	//	(memmove) move a load of raw data

	// SOA versions - if ENABLE_SOA is undefined they will use the memory system routines
	template<typename TYPE> FORCEINLINE TYPE*	SOAAllocate(std::size_t size, Bool bThrow);
	template<typename TYPE> FORCEINLINE void	SOADelete(TYPE*& pData);					//	delete an object if not-null and NULLs the pointer provided

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
		
		FORCEINLINE void*		MemAlloc(u32 Size);								//	malloc
		FORCEINLINE void		MemDealloc(void* pMem);							//	free
		FORCEINLINE void		MemCopy(void* pDest,const void* pSrc,u32 Size);	//	memcpy
		FORCEINLINE void		MemMove(void* pDest,const void* pSrc,u32 Size);	//	memmove
		FORCEINLINE void*		MemRealloc(void* pMem,u32 Size);				//	realloc
		FORCEINLINE void		MemValidate(void* pMem=NULL);					//	validate memory heaps

		FORCEINLINE void		MemFillPattern(void* pMem, u32 Size, u8 Pattern);	// Fills memory with a specifiec pattern to spot uninitialised/deleted/overwritten memory
	}

	class TMemoryTrack;								//	allocation tracking entry
	class TMemorySystem;							//	memory interface...


	const u8 Debug_AllocPattern		= 0xfa;				// Memory Allocation pattern (Debug only)
	const u8 Debug_DeallocPattern		= 0xfe;				// Memory Deallocation pattern (Debug only)
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

#if defined(__GNUG__)
	#define throwfunc(exceptionfunc)	throw (exceptionfunc)
#else
	#define	throwfunc(exceptionfunc)	throw ()
#endif


class TLMemory::TMemorySystem
{

public:

	static TMemorySystem& Instance()
	{
		if(!ms_pMemorySystem)
		{
			if(m_bDestroyed)
			{
				OnDeadReference();
			}
			else
			{
				Create();
			}

		}

		return *ms_pMemorySystem;
	}

	// Custom memory allocation
	FORCEINLINE void*		Allocate(std::size_t size)
	{
#ifdef _DEBUG
		if(m_prealloc_callback)
			m_prealloc_callback();
#endif

		void* pData = Platform::MemAlloc( size );
		if ( !pData )
		{
			TLMemory::Debug::Break("Failed to allocate memory", __FUNCTION__ );
			return NULL;
		}

		// Fill the memory with a pattern to be able to spot un-initialised data
		Platform::MemFillPattern(pData, size, TLMemory::Debug_AllocPattern);

		//	do memory debugging/logging
		//	this is per-lib
		#ifdef _DEBUG


		TLMemory::Debug::Debug_Alloc( pData, size );

		#endif

		m_totalAlloc += size;

#ifdef _DEBUG
		if(m_postalloc_callback)
			m_postalloc_callback();
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
#ifdef _DEBUG
		if(m_predealloc_callback)
			m_predealloc_callback();
#endif
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

		/////////////////////////////////////////////////////////////////////////////////////////////
		// DB - OK this doesn't work at all because by the point the object itself has been destroyed (destructor called) 
		// and we are simply getting the sizeof a pointer but not the object.  I have no idea how the free()
		// figures out how much memory to deallocate and can't find anything to tell me.
		// Therefore the only way to know the size of any specific object is to overload the new and delete per class
		// and call the Deallocate specifically passing in the size which is possible because the class
		// operator delete can have a size parameter whereas the global one cannot
		// Alternatively we could custom allocate a structure that stores the size and the pointer to the mem 
		// used by the requested object to keep track of objects all of the time
		// but this may also have issues when freeing the memory and also with performance?
		// Either way a lot more work required here to be able to track memory correctly ourselves.
		/////////////////////////////////////////////////////////////////////////////////////////////
		std::size_t size = sizeof(pObj);
		m_totalAlloc -= size;
		/////////////////////////////////////////////////////////////////////////////////////////////

		// Fill the memory with a pattern to be able to spot deleted and overwritten data
		// NOTE: Currently the size is incorrect - see comment above.
		Platform::MemFillPattern(pObj, size, TLMemory::Debug_DeallocPattern);

		//	delete memory
		Platform::MemDealloc( pObj );
		
#ifdef _DEBUG
		if(m_postdealloc_callback)
			m_postdealloc_callback();
#endif
	}

	// Memory callback handlers
#ifdef _DEBUG
	FORCEINLINE void SetPreAllocCallback(memcallback pCallback)		{ m_prealloc_callback = pCallback; }
	FORCEINLINE void SetPostAllocCallback(memcallback pCallback)	{ m_postalloc_callback = pCallback; }

	FORCEINLINE void SetPreDeallocCallback(memcallback pCallback)	{ m_predealloc_callback = pCallback; }
	FORCEINLINE void SetPostDeallocCallback(memcallback pCallback)	{ m_postdealloc_callback = pCallback; }

	FORCEINLINE void SetAllCallbacks(memcallback pCallback)	
	{
		m_prealloc_callback = pCallback;
		m_postalloc_callback = pCallback;
		m_predealloc_callback = pCallback;
		m_postdealloc_callback = pCallback; 
	}
#else
	FORCEINLINE void SetPreAllocCallback(memcallback pCallback)		{}
	FORCEINLINE void SetPostAllocCallback(memcallback pCallback)	{}

	FORCEINLINE void SetPreDeallocCallback(memcallback pCallback)	{}
	FORCEINLINE void SetPostDeallocCallback(memcallback pCallback)	{}

	FORCEINLINE void SetAllCallbacks(memcallback pCallback)			{}

#endif


#ifdef ENABLE_SOA
	
	FORCEINLINE TSmallObjectAllocator& GetSmallObjectAllocator()	 
	{ 
		if(!m_pSOA) 
			m_pSOA = new TSmallObjectAllocator(LOKI_DEFAULT_CHUNK_SIZE, LOKI_MAX_SMALL_OBJECT_SIZE, LOKI_DEFAULT_OBJECT_ALIGNMENT); 
		
		return *m_pSOA; 
	}
	
private:
	TSmallObjectAllocator*	m_pSOA;	// Small Object Allocator
#endif

private:
	TMemorySystem();										// Prevent outside creation - can only be accessed via the Instance() routine
	TMemorySystem(const TMemorySystem&)	{}					// Prevent automatic copy constructor creation
	TMemorySystem& operator=(const TMemorySystem&)	{ return *this; }		// Prevent assignment
	~TMemorySystem();										// Prevent outside destruction - automated by atexit via compiler

	void Initialise();
	void Shutdown();

	static void Create()
	{
		/*
		// Instance the single static memory system
		// Use malloc as the new would cause a recursive call to the Instance routine
		ms_pMemorySystem = static_cast<TMemorySystem*>(malloc(sizeof(TMemorySystem)));
		// call contructor
		ms_pMemorySystem->Initialise();
		*/

		static TMemorySystem theMemorySystem;
		ms_pMemorySystem = &theMemorySystem;
	}

	static void OnDeadReference()
	{
		// Memory system being used after it has been deleted. We *could* recreate the memory system and add it to the atexit list
		// (known as the pheonix pattern) which would probably work whilst we are simply allocating from the 
		// system but if we had our own pool/heap then this would probably not help much.
		//throw std::runtime_error("Allocation from Memory system when it has been deleted");
	}

private:

	static TMemorySystem*	ms_pMemorySystem;		//	global static memory system
	static Bool				m_bDestroyed;

	std::size_t				m_totalAlloc;

#ifdef _DEBUG
	// Callback function pointers for memory changes
	// Debug only due to speed implications and for thorough 
	// inspection of the callstack and variables
	memcallback				m_prealloc_callback;
	memcallback				m_postalloc_callback;

	memcallback				m_predealloc_callback;
	memcallback				m_postdealloc_callback;
#endif
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




FORCEINLINE void* operator new(std::size_t size) throwfunc(std::bad_alloc)
{
	return TLMemory::TMemorySystem::Instance().Allocate( size );
}

FORCEINLINE void* operator new[](std::size_t size) throwfunc(std::bad_alloc)
{
	return TLMemory::TMemorySystem::Instance().Allocate( size );
}

FORCEINLINE void operator delete(void* pObj) throw()
{
	return TLMemory::TMemorySystem::Instance().Deallocate( pObj );
}

FORCEINLINE void operator delete[](void* pObj) throw()
{
	return TLMemory::TMemorySystem::Instance().Deallocate( pObj );
}

//	include the platform specific header
#if defined(_MSC_EXTENSIONS) && defined(TL_TARGET_PC)
	#include "PC/PCMemory.h"
#endif

#if defined(TL_TARGET_IPOD)
	#include "IPod/IPodMemory.h"
#endif

#if defined(TL_TARGET_MAC)
	#include "Mac/MacMemory.h"
#endif




#include "TLMemory.inc.h"
