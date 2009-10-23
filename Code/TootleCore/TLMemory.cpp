#include "TLMemory.h"
#include "TString.h"



//	gr: enable this to enable memory debugging
//	this incorporates pointer and counting how much memory is allocated
#if defined(_DEBUG)
	//#define DEBUG_CHECK_MEMORY
#endif



namespace TLMemory
{
	//static TMemorySystem	g_sMemorySystem;		// global static memory system

	TLArray::SortResult	SortMemoryTracks(const TMemoryTrack& a,const TMemoryTrack& b,const void* pTestMemoryTrack);	//	sort memory trackers
	TMemoryTrack*		FindMemoryTrack(void* pAddress);			//	find a memory tracker from an address
	Bool				RemoveMemoryTrack(u32 Address);			//	remove a memory tracker from the list by address
	Bool				RemoveMemoryTrack(void* pAddress)			{	return RemoveMemoryTrack( TLCore::PointerToInteger( pAddress ) );	}
	Bool				RemoveMemoryTrack(TMemoryTrack*& pTrack)	{	if ( !RemoveMemoryTrack( pTrack->m_Address ) )	return FALSE;	pTrack = NULL;	return TRUE;	}

	#ifdef DEBUG_CHECK_MEMORY
	TFixedArray<TMemoryTrack,1000>		g_MemoryTracks( 0, &SortMemoryTracks );		//	array of data we've allocated - to search, use Find/Exists with a u32-memory address
	u32									g_MemoryAllocated = 0;	//	total amount of memory we've allocated
	#endif
};

TLMemory::TMemorySystem*	TLMemory::TMemorySystem::ms_pMemorySystem = NULL;
Bool						TLMemory::TMemorySystem::m_bDestroyed = FALSE;


TLMemory::TMemorySystem::TMemorySystem()
{
	Initialise();	
}


void TLMemory::TMemorySystem::Initialise()
{
	m_totalAlloc = 0;
	TLMemory::Platform::Initialise();
}

void TLMemory::TMemorySystem::Shutdown()
{
	ms_pMemorySystem->~TMemorySystem();
}




TLMemory::TMemorySystem::~TMemorySystem()
{
#ifdef _DEBUG
	if(m_totalAlloc > 0)
	{
		// We still have things allocated
		//TLDebug_Break("Memory system still has memory allocated");

		Platform::MemOuputAllocations();
	}
#endif

	TLMemory::Platform::Shutdown();

	ms_pMemorySystem = NULL;
	m_bDestroyed = TRUE;
}


//------------------------------------------------------
//	sort memory trackers
//------------------------------------------------------
TLArray::SortResult TLMemory::SortMemoryTracks(const TMemoryTrack& a,const TMemoryTrack& b,const void* pTestAddress)
{
	const u32& TestA = a.m_Address;
	const u32& TestB = pTestAddress ? *(const u32*)pTestAddress : b.m_Address;
	return (TestA < TestB) ? TLArray::IsLess : (TLArray::SortResult)(TestA == TestB);
}


//------------------------------------------------------
//	find a memory tracker from an address
//------------------------------------------------------
TLMemory::TMemoryTrack* TLMemory::FindMemoryTrack(void* pAddress)
{
#ifdef DEBUG_CHECK_MEMORY
	u32 Address = TLCore::PointerToInteger( pAddress );
	return g_MemoryTracks.Find( Address );
#else
	return NULL;
#endif
}


//------------------------------------------------------
//	find a memory tracker from an address
//------------------------------------------------------
Bool TLMemory::RemoveMemoryTrack(u32 Address)
{
#ifdef DEBUG_CHECK_MEMORY
	s32 Index = g_MemoryTracks.FindIndex( Address );
	if ( Index == -1 )
		return FALSE;
	g_MemoryTracks.RemoveAt( Index );
#endif
	return TRUE;
}


//------------------------------------------------------
//	log the allocation of some data (POST alloc)
//------------------------------------------------------
void TLMemory::Debug::Debug_Alloc(void* pDataAddress,u32 Size)
{
#ifdef DEBUG_CHECK_MEMORY
	//	check that this data hasn't already been allocated
	TMemoryTrack* pMemoryTrack = FindMemoryTrack( pDataAddress );
	if ( pMemoryTrack )
	{
		Debug::Break("Error in memory tracking system? this address has already been allocated and not deleted...", __FUNCTION__ );
	}
	else
	{
		//	create new memory track entry
		TMemoryTrack NewTrack( pDataAddress, Size );
		s32 Index = g_MemoryTracks.Add( NewTrack );
		if ( Index != -1 )
			pMemoryTrack = &g_MemoryTracks[Index];
	}

	//	update memory allocated
	g_MemoryAllocated += Size;
#endif
}

//------------------------------------------------------
//	log/debug deleting of data, return FALSE to abort the delete (PRE delete)
//------------------------------------------------------
Bool TLMemory::Debug::Debug_Delete(void* pDataAddress)
{
#ifdef DEBUG_CHECK_MEMORY
	//	check that this data still exists in our tracking, 
	TMemoryTrack* pMemoryTrack = FindMemoryTrack( pDataAddress );

	//	if it doesn't exist we're potentially deleting some memory twice (assuming this memory has been deleted and was removed from the tracker the first time)
	if ( !pMemoryTrack )
	{
		//	stop the platform delete/free
		if ( !Debug::Break("Potentially deleting memory twice... retry will ALLOW system to delete the memory", __FUNCTION__ ) )
			return FALSE;
	}

	//	update memory-allocated if we're deleting this data
	if ( pMemoryTrack )
	{
		g_MemoryAllocated -= pMemoryTrack->m_Size;

		//	delete tracker
		RemoveMemoryTrack( pMemoryTrack );
	}

#endif
	//	continue and delete data
	return TRUE;
}


//------------------------------------------------------
//	added this wrapper for TLDebug_Break so we don't include the string type or debug header
//------------------------------------------------------
Bool TLMemory::Debug::Break(const char* pErrorString,const char* pSourceFunction)
{
	TTempString BreakString = pErrorString;
	return TLDebug::Break( BreakString, pSourceFunction );
}

