#include "TOblong.h"
#include "TLine.h"
#include "TBox.h"
#include <TootleCore/TString.h>




TLMaths::TOblong::TOblong() :
	m_IsValid	( FALSE ),
	m_Corners	( 8 )
{
}




//----------------------------------------
//	get the center of the box
//----------------------------------------
float2 TLMaths::TOblong2D::GetCenter() const
{
	if ( !IsValid() )
	{
		TLDebug_Break("GetCenter() for invalid oblong shape");
		return float2( 0.f, 0.f );
	}

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



//-------------------------------------------------------
//	transform this shape
//-------------------------------------------------------
void TLMaths::TOblong2D::Transform(const TLMaths::TTransform& Transform)
{
	//	transform edges
	if ( !IsValid() || !Transform.HasAnyTransform() )
		return;

	for ( u32 i=0;	i<m_Corners.GetSize();	i++ )
	{
		Transform.Transform( m_Corners[i] );
	}
}


//-------------------------------------------------------
//	construct oblong from a transformed box
//-------------------------------------------------------
void TLMaths::TOblong2D::Set(const TLMaths::TBox2D& Box,const TLMaths::TTransform& Transform)
{
	if ( !Box.IsValid() )
	{
		TLDebug_Break("Constructing oblong from invalid box");
		SetValid( FALSE );
		return;
	}

	//	get initial corners
	m_Corners.SetSize(0);
	Box.GetBoxCorners( m_Corners );

	//	is now valid
	SetValid( TRUE );

	//	transform oblong's corners
	this->Transform( Transform );
}


