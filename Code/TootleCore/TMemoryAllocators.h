/*
 *  TMemoryAllocators.h
 *  TootleCore
 *
 *  Created by Duane Bradbury on 26/10/2009.
 *  Copyright 2009 Tootle. All rights reserved.
 *
 */

#pragma once

#include "TLMemory.h"

namespace TLMemory
{
	
	template<typename TYPE>
	class AllocatorPolicy_Default;

}

// Defualt allocator policy
template<typename TYPE>
class TLMemory::AllocatorPolicy_Default
{
		
	FORCEINLINE TYPE* Allocate(std::size_t size)
	{
		// Allocate via our custom memory system
		return TLMemory::TMemorySystem::Instance().Allocate( size );
	}
	
	FORCEINLINE void Deallocate(TYPE* pData)
	{
		// Deallocate via our custom memory system
		return TLMemory::TMemorySystem::Instance().Deallocate( pData );
	}
	
protected:	
	~AllocatorPolicy_Default()	{}	
};