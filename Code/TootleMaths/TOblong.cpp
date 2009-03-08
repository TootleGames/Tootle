#include "TOblong.h"
#include "TLine.h"
#include <TootleCore/TString.h>




TLMaths::TOblong::TOblong() :
	m_IsValid	( FALSE )
{
}




TLMaths::TOblong2D::TOblong2D() :
	m_IsValid	( FALSE )
{
}


//----------------------------------------
//	get the center of the box
//----------------------------------------
float2 TLMaths::TOblong2D::GetCenter() const
{
	//	gr: not sure if this is the best way of doing it... but it's some way.
	//	take a line from top left, to bottom right
	//	then from top right to bottom left
	//	see where they cross... and bingo! center!
	TLMaths::TLine2D TopLeftBottomRight( m_Corners[0], m_Corners[2] );
	TLMaths::TLine2D TopRightBottomLeft( m_Corners[1], m_Corners[3] );

	float2 Intersection;
	if ( !TopLeftBottomRight.GetIntersectionPos( TopRightBottomLeft, Intersection ) )
	{
		TLDebug_Break("Corners of 2D oblong failed to intersect... invalid shape?");
		return float2( 0.f, 0.f );
	}

	return Intersection;
}


