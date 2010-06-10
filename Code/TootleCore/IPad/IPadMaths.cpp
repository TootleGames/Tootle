/*
 *  IPadMaths.cpp
 *  TootleCore
 *
 *  Created by Duane Bradbury on 04/05/2010.
 *  Copyright 2010 Tootle. All rights reserved.
 *
 */


#include "IPadMaths.h"
#include "../TLMaths.h"


namespace TLMaths
{

	float	g_SineLookupTable[360];

	
	namespace Platform
	{
		void GenerateSinLookupTable();
	}
}



void TLMaths::Platform::GenerateSinLookupTable()
{

	//	init co/sine lookup table
	for ( u32 i=0;	i<360;	i++ )
	{
		//	calc value for this angle
		float Rad = TLMATHS_LOOKUP_TO_RAD( i );
		
		float Sine = TLMaths::Sinf( Rad );
		g_SineLookupTable[i] = Sine;
	}
	
	/*
	 for ( float f=0.f;	f<360.f;	f++ )
	 {
	 TLMaths::TAngle Angle(f);
	 float2 fastxy;
	 fastxy.x = TLMaths::Cosf( Angle.GetRadians() );
	 fastxy.y = TLMaths::Sinf( Angle.GetRadians() );
	 
	 float2 realxy;
	 realxy.x = TLMaths::Cosf( Angle.GetRadians() );
	 realxy.y = TLMaths::Sinf( Angle.GetRadians() );
	 
	 float2 diff = fastxy - realxy;
	 
	 TTempString Debug_String;
	 //		Debug_String.Appendf("deg: %2.2f. fast(%2.2f/%2.2f) = real(%2.2f/%2.2f)", f, fastxy.x, fastxy.y, realxy.x, realxy.y );
	 Debug_String.Appendf("deg: %2.2f. diff(%2.2f/%2.2f)", f, diff.x, diff.y );
	 TLDebug_Print( Debug_String );
	 }
	 */
	
}