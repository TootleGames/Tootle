/*
 *  TLRandom.h
 *  TootleCore
 *
 *  Created by Duane Bradbury on 15/05/2010.
 *  Copyright 2010 Tootle. All rights reserved.
 *
 */

#pragma once

#include "TLTypes.h"

//	include the platform specific header
#include PLATFORMHEADER(Random.h)


namespace TLMaths
{
	//	gr: all random functions are max INCLUSIVE. Want a random element in an array? use GetGetLastIndex()!
	FORCEINLINE float			Randf(float Max=1.f);									//	fraction between 0.f and X inclusive (float has 4 decimal places)
	FORCEINLINE float			Randf(float Min,float Max);								//	fraction between X and Y inclusive (float has 4 decimal places)
	FORCEINLINE s32				Rand(s32 Min,s32 Max);									//	Min <= N <= Max (inclusive!)
	
}

//-----------------------------------------------
//	fraction between 0.f and 1.f inclusive
//-----------------------------------------------
FORCEINLINE float TLMaths::Randf(float Max)
{
	//return (rand() / RAND_MAX) * Max;
	
	u32 RandInt = Rand( 0, 10000 );
	
	return ((float)RandInt * Max) / 10000.f;
}


//-----------------------------------------------
//	fraction between X and Y inclusive
//-----------------------------------------------
FORCEINLINE float TLMaths::Randf(float Min,float Max)
{
	return Randf( Max - Min ) + Min;
}

//-----------------------------------------------
//	get random number inclusive Min <= N <= Max
//-----------------------------------------------
FORCEINLINE s32 TLMaths::Rand(s32 Min,s32 Max)
{
	s32 Random = Rand( Max - Min + 1 );
	Random += Min;
	return Random;
}




