/*
 *  TArrayMisc.h
 *  TootleCore
 *
 *  Created by Duane Bradbury on 01/11/2009.
 *  Copyright 2009 Tootle. All rights reserved.
 *
 */

#pragma once

#include "TArray.h"
#include "TLMaths.h"

namespace TLArray
{
	
	template<typename TYPE>
	FORCEINLINE TYPE& GetRandomElement(TArray<TYPE>& array);

	
	template<typename TYPE>
	FORCEINLINE const TYPE&	GetRandomElementConst(const TArray<TYPE>& array);
	
}



template<typename TYPE>
FORCEINLINE TYPE& TLArray::GetRandomElement(TArray<TYPE>& array)
{
	s32 uRandomIndex = TLMaths::Rand( array.GetSize() );
	
	return array.ElementAt(uRandomIndex);	
}


template<typename TYPE>
FORCEINLINE const TYPE&	TLArray::GetRandomElementConst(const TArray<TYPE>& array)
{	
	s32 uRandomIndex = TLMaths::Rand( array.GetSize() );
	
	return array.ElementAtConst( uRandomIndex );	
}
