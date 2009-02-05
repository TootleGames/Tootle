#include "TLine.h"
#include "TSphere.h"
#include <TootleCore/TString.h>




//-----------------------------------------------------------
//	create an outside and inside linestrip for an existing linestrip
//-----------------------------------------------------------
void TLMaths::ExpandLineStrip(const TArray<float3>& LineStrip,float Width,TArray<float3>& OutsideLineStrip,TArray<float3>& InsideLineStrip)
{
	const float3* pPrevPoint = NULL;

	for ( u32 i=0;	i<LineStrip.GetSize();	i++ )
	{
		const float3& ThisPoint = LineStrip[i];
		const float3* pNextPoint = ((s32)i >= LineStrip.GetLastIndex()) ? NULL : &LineStrip[i+1];

		float3 Outset;

		//	calc outsets
		if ( pPrevPoint && pNextPoint )
		{
			Outset = TLMaths::GetLineStripOutset( *pPrevPoint, ThisPoint, *pNextPoint, Width/2.f );
		}
		else if ( pPrevPoint )
		{
			Outset = TLMaths::GetLineOutset( *pPrevPoint, ThisPoint, Width/2.f );
		}
		else if ( pNextPoint )
		{
			Outset = TLMaths::GetLineOutset( ThisPoint, *pNextPoint, Width/2.f );
		}
		else
		{
			TLDebug_Break("Line strip with one point?");
			Outset.Set( 0.f, 0.f, 0.f );
		}

		//	add to outside lines
		OutsideLineStrip.Add( ThisPoint - Outset );
		InsideLineStrip.Add( ThisPoint + Outset );

		//	store prev point
		pPrevPoint = &ThisPoint;
	}	
}


//-----------------------------------------------------------
//	calculates the (normalised by default) outset for the Middle of a line strip
//	gr: assumes line is clockwise. if not (and you have to detect that) then invert result
//	
// This function is a bit tricky. Given a path ABC, it returns the
// coordinates of the outset point facing B on the left at a distance
// of 64.0.
//                                         M
//                            - - - - - - X
//                             ^         / '
//                             | 64.0   /   '
//  X---->-----X     ==>    X--v-------X     '
// A          B \          A          B \   .>'
//               \                       \<'  64.0
//                \                       \                  .
//                 \                       \                 .
//                C X                     C X
//
//-----------------------------------------------------------
float3 TLMaths::GetLineStripOutset(const float3& Start,const float3& Middle,const float3& End,float OutsetLength)
{
	const float3& A = Start;
	const float3& B = Middle;
	const float3& C = End;
	TLDebug_CheckFloat( A );
	TLDebug_CheckFloat( B );
	TLDebug_CheckFloat( C );
	
    //	Build the rotation matrix from 'ba' vector
    float2 ba( A.x-B.x, A.y-B.y );
	ba.Normalise();
    float2 bc( C.x-B.x, C.y-B.y );
	
    //	Rotate bc to the left
    float2 tmp(bc.x * -ba.x + bc.y * -ba.y,
			   bc.x * ba.y + bc.y * -ba.x );
	
    //	Compute the vector bisecting 'abc'
	float norm = TLMaths::Sqrtf(tmp.x * tmp.x + tmp.y * tmp.y);
    
	float dist = 0;
	float normplusx = norm + tmp.x;
	float normminusx = norm - tmp.x;
	if ( normplusx != 0.0 )
	{
		float xdiv = normminusx / normplusx;
		if ( xdiv != 0 )
		{
			float sqrtxdiv = TLMaths::Sqrtf(xdiv);
			dist = OutsetLength * sqrtxdiv;
		}
	}

	//	gr: if tmp.y is positive when shrinking a poly, the new point is on the outside instead of inside

    tmp.x = (tmp.y<0.f) ? dist : -dist;
    tmp.y = OutsetLength;
	
    //	Rotate the new bc to the right
	float3 Result( tmp.x * -ba.x + tmp.y * ba.y, 
				  tmp.x * -ba.y + tmp.y * -ba.x,
				  B.z );


	return Result;
}


//-----------------------------------------------------------
//	calculates the outset for a line
//-----------------------------------------------------------
float3 TLMaths::GetLineOutset(const float3& Start,const float3& End,float OutsetLength)
{
	float3 Dir = End - Start;

	//	catch non-lines
	float LengthSq = Dir.LengthSq();
	if ( LengthSq < TLMaths::g_NearZero )
		return Start;

	//	z is interp'd between the start and end z's. (usually same z though)
	float z = Dir.z * 0.5f;

	//	normalise dir
	Dir.Normalise( TLMaths::Sqrtf(LengthSq), OutsetLength );

	//	rotate right 90 degrees to match the 3-point outset code
	//float3 Outset( Dir.y, -Dir.x, z );
	float3 Outset( -Dir.y, Dir.x, z );

	return Outset;
}





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