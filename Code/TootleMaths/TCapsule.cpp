#include "TCapsule.h"
#include "TSphere.h"
#include <TootleCore/TString.h>
#include "TBox.h"



TLMaths::TCapsule::TCapsule() :
	m_Line				( float3(0,0,0), float3(0,0,0) ),
	m_Radius			( -1.f )	//	invalid
{
}


//---------------------------------------------------------
//	
//---------------------------------------------------------
float TLMaths::TCapsule::GetDistanceSq(const float3& Pos) const
{
	if ( !this->IsValid() )
	{
		TLDebug_Break("Distance test between invalid shapes");
		return 0.f;
	}

	//	get the vector between the spheres
	float3 Diff( GetPosNearestTo(Pos) - Pos );

	//	get the extents of the spheres (squared)
	float SphereRadSq = GetRadius();
	SphereRadSq *= SphereRadSq;

	//	return the gap
	return Diff.LengthSq() - SphereRadSq;
}


//---------------------------------------------------------
//	
//---------------------------------------------------------
float TLMaths::TCapsule::GetDistanceSq(const TCapsule& Capsule) const
{
	if ( !this->IsValid() || !Capsule.IsValid() )
	{
		TLDebug_Break("Distance test between invalid shapes");
		return 0.f;
	}

	//	gr: this hasn't been tested, just assuming it's right...
	TLDebug_Break("untested");

	//		there's probably a better method, eg, line/line interesection test
	float3 ThisPos2 = GetLine().GetEnd();
	float3 ThisNearPos = Capsule.GetPosNearestTo( GetPos() );
	float3 ThisNearPos2 = Capsule.GetPosNearestTo( ThisPos2 );

	float3 CapsulePos2 = Capsule.GetLine().GetEnd();
	float3 CapsuleNearPos = Capsule.GetPosNearestTo( GetPos() );
	float3 CapsuleNearPos2 = Capsule.GetPosNearestTo( ThisPos2 );

	//	the middle of the our two nearest positions should be the closest point (eg. interesection)
	ThisNearPos += ThisNearPos2;
	ThisNearPos *= 0.5f;
	CapsuleNearPos += CapsuleNearPos2;
	CapsuleNearPos *= 0.5f;

	//	now treat as sphere
	float3 Diff( ThisNearPos - CapsuleNearPos );

	//	get the extents of the spheres (squared)
	float SphereRadSq = GetRadius() + Capsule.GetRadius();
	SphereRadSq *= SphereRadSq;

	//	return the gap
	return Diff.LengthSq() - SphereRadSq;
}


//--------------------------------------------------------
//	transform capsule
//--------------------------------------------------------
void TLMaths::TCapsule::Transform(const TLMaths::TTransform& Transform)
{
	if ( Transform.HasScale() )
	{
		TLDebug_Break("todo");
	}

	if ( Transform.HasRotation() )
	{
		TLDebug_Break("todo");
	}

	if ( Transform.HasTranslate() )
	{
		TLDebug_Break("todo");
	}
}




//-----------------------------------------------------------
//	create sphere from capsule
//-----------------------------------------------------------
TLMaths::TSphere TLMaths::TCapsule::GetSphere() const
{
	//	get center of sphere
	float3 Center = GetCenter();

	//	get radius of line
	float Radius = GetLine().GetLength();
	Radius += GetRadius();

	return TSphere( Center, Radius );
}


//-----------------------------------------------------------
//	accumulate other capsule. copies other if this is invalid
//-----------------------------------------------------------
void TLMaths::TCapsule::Accumulate(const TCapsule& Capsule)
{
	if ( !Capsule.IsValid() )
	{
		TLDebug_Break("Warning: trying to accumulate invalid capsule");
		return;
	}

	if ( !IsValid() )
	{
		Set( Capsule );
		return;
	}

	//	get the two points on the surface of the capsule furthest away from each other...
	//	furthest point on line + diraway*radius
}


//-----------------------------------------------------------
//	grow the box to these extents
//-----------------------------------------------------------
void TLMaths::TCapsule::Accumulate(const float3& Point)
{
	if ( !IsValid() )
	{
		Set( Point, Point, 0.f );
		return;
	}

	//	gr: todo: extend line, not just radius

	//	get distance from capsule - if further then extend the radius
	float DistanceSqFromCapsule = GetDistanceSq( Point );

	//	already inside capsule
	if ( DistanceSqFromCapsule <= 0.f )
		return;

	//	extend radius by distance
	m_Radius += Sqrtf( DistanceSqFromCapsule );
}


//-----------------------------------------------------------
//	get the extents of all these points
//-----------------------------------------------------------
void TLMaths::TCapsule::Accumulate(const TArray<float3>& Points)
{
	for ( u32 i=0;	i<Points.GetSize();	i++ )
	{
		Accumulate( Points[i] );
	}
}













TLMaths::TCapsule2D::TCapsule2D() :
	m_Radius	( -1.f )
{
}

//---------------------------------------------------------
//	create a capsule out of a box
//---------------------------------------------------------
void TLMaths::TCapsule2D::Set(const TLMaths::TBox2D& Box)
{
	if ( !Box.IsValid() )
	{
		TLDebug_Break("Attempted to accumulate invalid box into capsule");
		return;
	}

	//	get the longest sides to work out capsule line length/radius
	float2 BoxSize = Box.GetSize();
	float Width = BoxSize.x;
	float HalfWidth = Width / 2.f;
	float Height = BoxSize.y;
	float HalfHeight = Height / 2.f;
	const float2& BoxMin = Box.GetMin();
	const float2& BoxMax = Box.GetMax();

	if ( Height > Width )
	{
		//	set radius
		m_Radius = HalfWidth;

		//	create initial line
		float2 LineStart( BoxMin.x + HalfWidth, BoxMin.y );
		float2 LineEnd( BoxMin.x + HalfWidth, BoxMax.y );

		//	shorten line so that the edge of the radius meets the edge of the line
		LineStart.y += m_Radius;
		LineEnd.y -= m_Radius;

		m_Line.Set( LineStart, LineEnd );

	}
	else
	{
		//	set radius
		m_Radius = HalfHeight;

		//	create initial line
		float2 LineStart( BoxMin.x, BoxMin.y + HalfHeight );
		float2 LineEnd( BoxMax.x, BoxMin.y + HalfHeight );

		//	shorten line so that the edge of the radius meets the edge of the line
		LineStart.x += m_Radius;
		LineEnd.x -= m_Radius;

		m_Line.Set( LineStart, LineEnd );
	}

}


//---------------------------------------------------------
//	transform capsule
//---------------------------------------------------------
void TLMaths::TCapsule2D::Transform(const TLMaths::TTransform& Transform)
{
	if ( !IsValid() )
	{
		TLDebug_Break("Transforming invalid capsule");
		return;
	}

	//	skip jumping to line's transform code
	if ( !Transform.HasAnyTransform() )
		return;

	//	apply transforms to the line
	m_Line.Transform( Transform );

	if ( Transform.HasScale() )
	{
		//	scale radius by biggest value of the scale
		const float3& Scale = Transform.GetScale();
		float BigScale = (Scale.x>Scale.y) ? Scale.x : Scale.y;
		BigScale = (Scale.z>BigScale) ? Scale.z : BigScale;

		m_Radius *= BigScale;
	}

}

