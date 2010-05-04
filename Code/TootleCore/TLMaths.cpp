#include "TLMaths.h"

#include "TString.h"

#include "TLTime.h"
#include "TColour.h"


namespace TLMaths
{
	namespace Platform
	{
		void GenerateSinLookupTable();
	}
}


namespace TLColour
{
	TFixedArray<TColour,8>	g_Debug_Colours;	//	static list of debug colours
}



//	gr: do not use this on ipod!
//USE_OPENGL_MATRIX_MULT


//-------------------------------------------------------------
//	maths system init, just seeds random number
//-------------------------------------------------------------
void TLMaths::Init()
{
	//	init debug colours
//	TLColour::g_Debug_Colours.Add( TColour( 1.0f, 1.0f, 1.0f ) );	//	white
	TLColour::g_Debug_Colours.Add( TColour( 1.0f, 0.0f, 0.0f ) );	//	red
	TLColour::g_Debug_Colours.Add( TColour( 0.0f, 1.0f, 0.0f ) );	//	green
	TLColour::g_Debug_Colours.Add( TColour( 0.0f, 0.0f, 1.0f ) );	//	blue
	TLColour::g_Debug_Colours.Add( TColour( 1.0f, 1.0f, 0.0f ) );	//	yellow
	TLColour::g_Debug_Colours.Add( TColour( 1.0f, 0.0f, 1.0f ) );	//	pink
	TLColour::g_Debug_Colours.Add( TColour( 0.0f, 1.0f, 1.0f ) );	//	cyan
//	TLColour::g_Debug_Colours.Add( TColour( 0.0f, 0.0f, 0.0f ) );	//	black

	// generate the sin lookup table
	Platform::GenerateSinLookupTable();

}


//-----------------------------------------------------------
//	get angle from a vector
//-----------------------------------------------------------
void TLMaths::TAngle::SetAngle(const float2& Direction)
{
	//	doesn't need to be normalised
	//	gr: Atan2f is supposed to be y/x but that's wrong. Maybe because of our world co-ordinates...
	//	y is inverted as math co-ordinates go +ve when they go UP, but in our world, -Y is UP
	float AngRad = TLMaths::Atan2f( Direction.x, -Direction.y );
	
	SetRadians( AngRad );
	
	//	make sure value is in range
	SetLimit180();
}





