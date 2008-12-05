#include "TCapsule.h"
#include "TSphere.h"
#include <TootleCore/TString.h>



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

	if ( Transform.HasMatrix() )
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

