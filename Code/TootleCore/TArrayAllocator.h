/*
 *  TArrayAllocator.h
 *  TootleCore
 *
 *  Created by Duane Bradbury on 29/10/2009.
 *  Copyright 2009 Tootle. All rights reserved.
 *
 */

#pragma once

#include "TLMemory.h"

namespace TLArray
{
	template<typename TYPE>
	class AllocatorPolicy_Default;
}


template<typename TYPE>
class TLArray::AllocatorPolicy_Default
{
public:
		
	FORCEINLINE TYPE* Allocate(std::size_t size)
	{
		// Code copied from TLMemory::AllocArray<TYPE>(size) 
		return new TYPE[ size ];
	}
	
	FORCEINLINE void Deallocate(TYPE*& pData)
	{
		// Code copied from TLMemory::DeleteArray( pData )
		if ( !pData )
			return;
		
		//	sometimes we want to NULL the source pointer before we delete it because of destructors
		TYPE* pTmpData = pData;
		pData = NULL;
		
		delete[] pTmpData;
	}
	
	
	FORCEINLINE void CopyData(TYPE* pToData,const TYPE* pFromData,u32 Elements)
	{
		TLMemory::CopyData( pToData, pFromData, Elements );
	}
	
	
	FORCEINLINE void MoveData(TYPE* pToData,const TYPE* pFromData,u32 Elements)
	{
		TLMemory::MoveData( pToData, pFromData, Elements );
	}
	
	
	FORCEINLINE void Validate()
	{
		TLMemory::Platform::MemValidate();
	}
	
protected:		
	~AllocatorPolicy_Default() {}

};
