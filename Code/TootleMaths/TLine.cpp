#include "TLine.h"
#include "TSphere.h"
#include <TootleCore/TString.h>



TLMaths::TLine::TLine(const float3& Start,const float3& End) : 
	m_Start	( Start ),
	m_End	( End )
{
}



	
//-----------------------------------------------------------
//	find the point along the line closest to Position
//-----------------------------------------------------------
float3 TLMaths::TLine::GetNearestPoint(const float3& Position) const
{
	float3 LineDir = GetDirection();
	float LineDirDotProduct = LineDir.DotProduct(LineDir);
	
	//	avoid div by zero
	if ( LineDirDotProduct == 0.f )
		return GetStart();

	float3 Dist = Position - GetStart();

	float LineDirDotProductDist = LineDir.DotProduct(Dist);

	float PointAlongLine = LineDirDotProductDist / LineDirDotProduct;

	if ( PointAlongLine <= 0.f )
		return GetStart();

	if ( PointAlongLine >= 1.f )
		return GetEnd();

	return GetStart() + LineDir * PointAlongLine;
}

//-----------------------------------------------------------
//	gr: this needs testing
//-----------------------------------------------------------
float TLMaths::TLine::GetDistanceSq(const float3& Position) const
{
	float3 vNearestPoint = GetNearestPoint(Position);

	// Vector between the position and the nearest point on the line
	float3 vAB = Position - vNearestPoint;

	// Use pythaguras to work out distance between the two points
	float fDist = (vAB.x * vAB.x) + (vAB.y * vAB.y) + (vAB.z * vAB.z); 

	return fDist;
}





TLMaths::TLine2D::TLine2D(const float2& Start,const float2& End) : 
	m_Start	( Start ),
	m_End	( End )
{
}

TLMaths::TLine2D::TLine2D(const float3& Start,const float3& End) : 
	m_Start	( Start.x, Start.y ),
	m_End	( End.x, End.y )
{
}



	
//-----------------------------------------------------------
//	find the point along the line closest to Position
//-----------------------------------------------------------
float2 TLMaths::TLine2D::GetNearestPoint(const float2& Position) const
{
	float2 LineDir = GetDirection();
	float LineDirDotProduct = LineDir.DotProduct(LineDir);
	
	//	avoid div by zero
	if ( LineDirDotProduct == 0.f )
		return GetStart();

	float2 Dist = Position - GetStart();

	float LineDirDotProductDist = LineDir.DotProduct(Dist);

	float PointAlongLine = LineDirDotProductDist / LineDirDotProduct;

	if ( PointAlongLine <= 0.f )
		return GetStart();

	if ( PointAlongLine >= 1.f )
		return GetEnd();

	return GetStart() + LineDir * PointAlongLine;
}

	
//-----------------------------------------------------------
//	find the point along the line closest to Position
//-----------------------------------------------------------
float2 TLMaths::TLine2D::GetNearestPoint(const float3& Position) const
{
	float2 LineDir = GetDirection();
	float LineDirDotProduct = LineDir.DotProduct(LineDir);
	
	//	avoid div by zero
	if ( LineDirDotProduct == 0.f )
		return GetStart();

	float2 Dist(Position.x, Position.y);
	Dist -= GetStart();

	float LineDirDotProductDist = LineDir.DotProduct(Dist);

	float PointAlongLine = LineDirDotProductDist / LineDirDotProduct;

	if ( PointAlongLine <= 0.f )
		return GetStart();

	if ( PointAlongLine >= 1.f )
		return GetEnd();

	return GetStart() + LineDir * PointAlongLine;
}

//-----------------------------------------------------------
//	gr: this needs testing
//-----------------------------------------------------------
float TLMaths::TLine2D::GetDistanceSq(const float2& Position) const
{
	float2 vNearestPoint = GetNearestPoint(Position);

	// Vector between the position and the nearest point on the line
	float2 vAB = Position - vNearestPoint;

	// Use pythaguras to work out distance between the two points
	//float fDist = vAB.DotProduct();
	float fDist = (vAB.x * vAB.x) + (vAB.y * vAB.y); 

	return fDist;
}


//-----------------------------------------------------------
//	
//-----------------------------------------------------------
Bool TLMaths::TLine2D::GetIntersection(const TLMaths::TSphere2D& Sphere) const
{
	return Sphere.GetIntersection( *this );
}




//---------------------------------------------------
//	Check to see if a line intersects another
//---------------------------------------------------


//-----------------------------------------------------------
//	get the point where this line crosses the other
//-----------------------------------------------------------
Bool TLMaths::TLine2D::GetIntersectionPos(const TLine2D& Line,float2& IntersectionPos) const
{
	//	get intersection points
	float IntersectionAlongThis,IntersectionAlongLine;
	if ( !GetIntersectionPos( Line, IntersectionAlongThis, IntersectionAlongLine ) )
		return FALSE;

	//	calc the intersection pos	
	IntersectionPos = GetStart();
	TLMaths::InterpThis( IntersectionPos, GetEnd(), IntersectionAlongThis );

	//	test that the results are the same...
#ifdef _DEBUG
	float2 TestIntersectionPos( Line.GetStart() );
	TLMaths::InterpThis( TestIntersectionPos, Line.GetEnd(), IntersectionAlongLine );

	if ( (TestIntersectionPos - IntersectionPos).Length() > TLMaths::g_NearZero )
	{
		TLDebug_Break("These intersection positions should be exactly the same...");
	}
#endif

	return TRUE;
}


//-----------------------------------------------------------
//	get the point where this line crosses the other
//-----------------------------------------------------------
Bool TLMaths::TLine2D::GetIntersection(const TLine2D& Line) const
{
	//	test intersection
	float IntersectionAlongThis,IntersectionAlongLine;
	return GetIntersectionPos( Line, IntersectionAlongThis, IntersectionAlongLine );
}


//-----------------------------------------------------------
//	checks for intersection and returns where along the line on both cases where it intersects
//-----------------------------------------------------------
Bool TLMaths::TLine2D::GetIntersectionPos(const TLine2D& Line,float& IntersectionAlongThis,float& IntersectionAlongLine) const
{
	const float2& v1 = GetStart();
	const float2& v2 = GetEnd();
	const float2& v3 = Line.GetStart();
	const float2& v4 = Line.GetEnd();

	float2 v2MinusV1( v2-v1 );
	float2 v1Minusv3( v1-v3 );
	float2 v4Minusv3( v4-v3 );

	float denom =		((v4Minusv3.y) * (v2MinusV1.x)) - ((v4Minusv3.x) * (v2MinusV1.y));
    float numerator =	((v4Minusv3.x) * (v1Minusv3.y)) - ((v4Minusv3.y) * (v1Minusv3.x));
    float numerator2 =	((v2MinusV1.x) * (v1Minusv3.y)) - ((v2MinusV1.y) * (v1Minusv3.x));

    if ( denom == 0.0f )
    {
        if ( numerator == 0.0f && numerator2 == 0.0f )
        {
            return FALSE;//COINCIDENT;
        }
        return FALSE;// PARALLEL;
    }

	float& ua = IntersectionAlongThis;
	float& ub = IntersectionAlongLine;

    ua = numerator / denom;
    ub = numerator2/ denom;

	//	intersection will be past the ends of these lines
	if ( ua < 0.f || ua > 1.f )	return FALSE;
	if ( ub < 0.f || ub > 1.f )	return FALSE;

	return TRUE;
}