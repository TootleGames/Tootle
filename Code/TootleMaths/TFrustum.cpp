#include "TFrustum.h"
#include "TSphere.h"
#include "TBox.h"
#include <TootleCore/TString.h>



namespace TLMaths
{
	FORCEINLINE Bool	PointOnPlane(const TLMaths::TPlane& Plane,const float2& Point);
};


FORCEINLINE Bool TLMaths::PointOnPlane(const TLMaths::TPlane& Plane,const float2& Point)
{
	float PlaneDotPoint = ( Plane.x() * Point.x + Plane.y() * Point.y );
		
	//	is on correct side
	//	if < 0 then on wrong side
	return ( PlaneDotPoint >= 0.f );
}


TLMaths::TFrustum::TFrustum() : 
	m_Near	( 0.f ),
	m_Far	( 0.f )
{
}


//-----------------------------------------------
//	check if this point is inside the frustum
//-----------------------------------------------
Bool TLMaths::TFrustum::HasIntersection(const float3& Point) const
{
	//	check point against all planes
	for ( u32 p=0;	p<m_Planes.GetSize();	p++ )
	{
		const TLMaths::TPlane& Plane = m_Planes[p];
		float PlaneDotPoint = Plane.GetNormal().DotProduct( Point );
		//float PlaneDotPoint = ( Plane.x * Point.x + Plane.y * Point.y + Plane.z * Point.z );
		
		//	wrong side
		if ( PlaneDotPoint < 0.f )
			return FALSE;

		//	== 0.f on the plane
		//	> 0.f correct side of plane
	}

	return TRUE;
}


//-----------------------------------------------
//	check if this sphere is inside the frustum
//-----------------------------------------------
Bool TLMaths::TFrustum::HasIntersection(const TLMaths::TSphere& Sphere) const
{
	//	check point against all planes
	for ( u32 p=0;	p<m_Planes.GetSize();	p++ )
	{
		const TLMaths::TPlane& Plane = m_Planes[p];
		float PlaneDotPoint = Plane.GetNormal().DotProduct( Sphere.GetPos() );
		//float PlaneDotPoint = ( Plane.x * Point.x + Plane.y * Point.y + Plane.z * Point.z );
		
		//	too far away on wrong side
		if ( PlaneDotPoint < -Sphere.GetRadius() )
			return FALSE;
	}

	return TRUE;
}



//-----------------------------------------------
//	check if this 2D point is inside the frustum
//-----------------------------------------------
Bool TLMaths::TFrustum::HasIntersection(const float2& Point) const
{
	TLDebug_Break("todo: check against left/right/top/bottom planes only - make a 3d point and have the Z same as the z center of the plane we're testing with");
	
	//	check point against all planes
	//	planes 0,1,2,3 are left/right/top/bottom so this ignores near and far
	for ( u32 p=0;	p<4;	p++ )
	{
		if ( !PointOnPlane( m_Planes[p], Point ) )
			return FALSE;
	}

	return TRUE;
}


//-----------------------------------------------
//	check to see if this box is the frustum at all
//
//	The function will produce the occasional false positive, meaning it will tell you a cube is visible when it's not. 
//	This happens in the case where all the corners of the bounding box are not behind any one plane, but it's still outside 
//	the frustum. So you might end up rendering objects that won't be visible. Depending on how your world looks and how you're 
//	using frustum culling, this is likely to be a rare event, and it shouldn't have a noticeable impact on overall rendering 
//	speed unless the false positives contain a very large number of polygons.
//	To make this completely accurate you would also have to test the eight corners of the frustum volume against the six planes 
//	that make up the sides of the bounding box. If the bounding box is axially aligned, you can dispense with the box's planes 
//	and perform some simple greater-than/less-than tests for each corner of the frustum. In any case, this is left as an 
//	exercise for those with more patience than I :-) 
//-----------------------------------------------
Bool TLMaths::TFrustum::HasIntersection(const TLMaths::TBox& Box,Bool TestNearFarPlanes) const
{
	const float3& Min = Box.GetMin();
	const float3& Max = Box.GetMax();

	//	check point against all planes
	for ( u32 p=0;	p<m_Planes.GetSize();	p++ )
	{
		//	skip near/far planes as required
		if ( !TestNearFarPlanes && (p == 4 || p == 5) )
			continue;

		const TLMaths::TPlane& Plane = m_Planes[p];
		const float4& Plane4 = Plane.xyzw();

		u32 c = 0;
		if( (Plane4.x * Min.x) + (Plane4.y * Min.y) + (Plane4.z * Min.z) + Plane4.w >= 0.f )
			c++;
		if( (Plane4.x * Max.x) + (Plane4.y * Min.y) + (Plane4.z * Min.z) + Plane4.w >= 0.f )
			c++;
		if( (Plane4.x * Min.x) + (Plane4.y * Max.y) + (Plane4.z * Min.z) + Plane4.w >= 0.f )
			c++;
		if( (Plane4.x * Max.x) + (Plane4.y * Max.y) + (Plane4.z * Min.z) + Plane4.w >= 0.f )
			c++;
		if( (Plane4.x * Min.x) + (Plane4.y * Min.y) + (Plane4.z * Max.z) + Plane4.w >= 0.f )
			c++;
		if( (Plane4.x * Max.x) + (Plane4.y * Min.y) + (Plane4.z * Max.z) + Plane4.w >= 0.f )
			c++;
		if( (Plane4.x * Min.x) + (Plane4.y * Max.y) + (Plane4.z * Max.z) + Plane4.w >= 0.f )
			c++;
		if( (Plane4.x * Max.x) + (Plane4.y * Max.y) + (Plane4.z * Max.z) + Plane4.w >= 0.f )
			c++;

		//	if all the corners are behind this plane, it's not visible
		if( c == 0 )
			return FALSE;

		//	wholly inside
		//if( c == 8 )
		//	c2++;
	}
	
	//	
	return TRUE;
}


//-----------------------------------------------
//	check to see if this box is inside/clipping the frustum - 2D so only checks against top/left/right/bottom
//-----------------------------------------------
Bool TLMaths::TFrustum::HasIntersection(const TLMaths::TBox2D& Box) const
{
	float z = TLMaths::Interp( m_Near, m_Far, 0.5f );

	//	convert to 3d box
	TLMaths::TBox Box3( Box.GetMin().xyz(z), Box.GetMax().xyz(z) );
	
	if ( HasIntersection( Box3, FALSE ) )
		return TRUE;

	return FALSE;
}
