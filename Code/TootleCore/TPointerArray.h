/*
 *  TPointerArray.h
 *  TootleCore
 *
 *  Created by Duane Bradbury on 28/10/2009.
 *  Copyright 2009 Tootle. All rights reserved.
 *
 */



#pragma once

#include "TArray.h"

// Definition of the TPointerArray 
template<typename TYPE, class SORTPOLICY=TLArray::SortPolicy_None< TYPE* >, class ALLOCATORPOLICY=TLArray::AllocatorPolicy_Default< TYPE* > >
class TPointerArray : public TArray<TYPE*, SORTPOLICY, ALLOCATORPOLICY> 
{};

