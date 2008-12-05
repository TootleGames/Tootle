
#include "TMemoryManager.h"

using namespace TLMemory;

/*
	Handler for when we run out of memory
*/
int TLMemory::OutOfMemory_Handler(std::size_t size)
{
	TLDebug_Break("Out of memory!");
	return 0;
}


SyncBool TMemoryManager::Initialise()
{
	#if defined(__GNUG__)
		//	std version
		std::set_new_handler(TLMemory::OutOfMemory_Handler_std);
		return SyncTrue;
	#else
		_set_new_handler(TLMemory::OutOfMemory_Handler);
		return SyncTrue;
	#endif
	
	return SyncFalse;
}