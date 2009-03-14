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

