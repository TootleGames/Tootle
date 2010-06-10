/*
 *  IPadRandom.h
 *  TootleCore
 *
 *  Created by Duane Bradbury on 15/05/2010.
 *  Copyright 2010 Tootle. All rights reserved.
 *
 */

#include "../TLTypes.h"

#include <stdlib.h>		

namespace TLMaths
{
	
	FORCEINLINE u32				Rand(u32 Max);							//	number between 0 and Max inclusive
	FORCEINLINE void			SRand(u32 uSeed)	{ srand(uSeed); }
}


//-----------------------------------------------
//	get random u32 up to max (NOT inclusive... 0 <= rand < Max)
//-----------------------------------------------
FORCEINLINE u32 TLMaths::Rand(u32 Max)
{
	u32 RandInt = rand() % Max;
	return RandInt;
}

