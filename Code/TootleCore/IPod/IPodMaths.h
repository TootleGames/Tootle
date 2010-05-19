/*
 *  IPodMaths.h
 *  TootleMaths
 *
 *  Created by Duane Bradbury on 03/05/2010.
 *  Copyright 2010 Tootle. All rights reserved.
 *
 */

#pragma once

#include "../TLTypes.h"


#include <math.h>		
#include <float.h>		
#include <limits.h>	

namespace TLMaths
{
	// System sin/cos/tan routines
	FORCEINLINE float			Cosf(float RadAngle)					{ 	return ::cosf( RadAngle );	}
	FORCEINLINE float			Sinf(float RadAngle)					{	return ::sinf( RadAngle );	}
	FORCEINLINE float			Tanf(float RadAngle)					{	return ::tanf( RadAngle );	}
	
	// Quick sin/cos routines that use a lookup table
	FORCEINLINE float			Qcosf(float RadAngle);
	FORCEINLINE float			Qsinf(float RadAngle);

	
	FORCEINLINE float			Atanf(float RadAngle)					{	return atanf( RadAngle );	}
	FORCEINLINE float			Asinf(float RadAngle)					{	return asinf( RadAngle );	}
	FORCEINLINE float			Acosf(float RadAngle)					{	return acosf( RadAngle );	}		
	
	FORCEINLINE float			Atan2f(float fX, float fY)				{	return atan2f( fX, fY );	}
	
	FORCEINLINE float			Sqrtf(float SquaredValue)				{	return sqrtf( SquaredValue );	}
	
	FORCEINLINE float			Absf(float Value)						{	return fabsf( Value );	}
	FORCEINLINE float			Modf(float Value, float Mod)			{	return fmodf( Value, Mod );	}
	FORCEINLINE float			Ceilf(float Value)						{	return ceilf( Value );	}
	FORCEINLINE float			Floorf(float Value)						{	return floorf( Value );	}
	
	FORCEINLINE float			S32ToFloat(s32 Value)					{ return (float)( Value );	}
	FORCEINLINE s32				FloatToS32(float Value)					{ return (s32)( Value );	}
	
#define TLMaths_FloatMax				FLT_MAX
#define TLMaths_FloatMin				FLT_MIN

	extern float					g_SineLookupTable[360];

}



FORCEINLINE float TLMaths::Qcosf(float RadAngle)
{
	return ::cosf( RadAngle );
	/*
	 s32 Index = GetCosineLookupFromRad( RadAngle );
	 TLDebug_CheckIndex( Index, 360 );
	 if ( Index < 0 )
	 Index = -Index;
	 return g_SineLookupTable[ (Index + 90 ) % 360 ];
	 */
}

FORCEINLINE float TLMaths::Qsinf(float RadAngle)
{
	return ::sinf( RadAngle );
	/*
	 s32 Index = GetCosineLookupFromRad( RadAngle );
	 TLDebug_CheckIndex( Index, 360 );
	 if ( Index < 0 )
	 Index = -Index;
	 return g_SineLookupTable[ Index % 360 ];
	 */
}



