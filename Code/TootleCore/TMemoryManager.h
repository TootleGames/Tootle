/*
	TMemoryManager

*/
#pragma once

#include "TManager.h"
#include "TLMemory.h"


namespace TLMemory
{
	class TMemoryManager;

	int					OutOfMemory_Handler(std::size_t size);		// Out of memory handler
	FORCEINLINE void	OutOfMemory_Handler_std()					{	OutOfMemory_Handler(0);	}
}	


class TLMemory::TMemoryManager : public TLCore::TManager
{
public:
	TMemoryManager(TRefRef ManagerRef) :
		TLCore::TManager	( ManagerRef )
	{
	}

protected:
	SyncBool Initialise();
};


