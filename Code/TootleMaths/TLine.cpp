#include "TLine.h"
#include "TSphere.h"
#include <TootleCore/TString.h>




//-----------------------------------------------------------
//	create an outside and inside linestrip for an existing linestrip
//-----------------------------------------------------------
void TLMaths::ExpandLineStrip(const TArray<float3*>& LineStrip,const TArray<float*>& Widths,const TArray<float*>& Offsets,TArray<float3>& OutsideLineStrip,TArray<float3>& InsideLineStrip)
{
	const float3* pPrevPoint = NULL;

	for ( u32 i=0;	i<LineStrip.GetSize();	i++ )
	{
		const float3& ThisPoint = *LineStrip[i];
		const float3* pNextPoint = ((s32)i >= LineStrip.GetLastIndex()) ? NULL : LineStrip[i+1];
		const float& Width = *Widths[i];

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

		//	offset
		float3 Offset;
		if ( Offsets.GetSize() )
			Offset = Outset.Normal( *Offsets[i] );

		//	add to outside lines
		OutsideLineStrip.Add( ThisPoint + Offset - Outset );
		InsideLineStrip.Add( ThisPoint + Offset + Outset );

		//	store prev point
		pPrevPoint = &ThisPoint;
	}	
}


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
	if ( LengthSq < TLMaths_NearZero )
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



//-----------------------------------------------------------
//	work out which two ends of these lines are closest. Set's a combination of bools to TRUE when it's a start, FALSE if its nearer the end
//	returns the distance sq of the nearest points too
//-----------------------------------------------------------
void TLMaths::GetNearestLinePoints(const TLMaths::TLine2D& LineA,const TLMaths::TLine2D& LineB,Bool& LineANearStart,Bool& LineBNearStart,float& DistanceSq)
{
	//	work out which two ends are closest to each other
	float StartStartDistSq = (LineA.GetStart() - LineB.GetStart()).LengthSq();
	float StartEndDistSq = (LineA.GetStart() - LineB.GetEnd()).LengthSq();
	float EndStartDistSq = (LineA.GetEnd() - LineB.GetStart()).LengthSq();
	float EndEndDistSq = (LineA.GetEnd() - LineB.GetEnd()).LengthSq();

	//	assume is start/start at first
	LineANearStart = TRUE;
	LineBNearStart = TRUE;
	DistanceSq = StartStartDistSq;

	//	see if any other pair of corners are nearer
	if ( StartEndDistSq < DistanceSq )
	{
		LineANearStart = TRUE;
		LineBNearStart = FALSE;
		DistanceSq = StartEndDistSq;
	}
	
	if ( EndStartDistSq < DistanceSq )
	{
		LineANearStart = FALSE;
		LineBNearStart = TRUE;
		DistanceSq = EndStartDistSq;
	}
	
	if ( EndEndDistSq < DistanceSq )
	{
		LineANearStart = FALSE;
		LineBNearStart = FALSE;
		DistanceSq = EndEndDistSq;
	}
	

}



TLMaths::TLine::TLine(const float3& Start,const float3& End) : 
	m_Start	( Start ),
	m_End	( End )
{
}


TLMaths::TLine::TLine(const TLMaths::TLine2D& Line,float z) : 
	m_Start	( Line.GetStart().xyz(z) ),
	m_End	( Line.GetEnd().xyz(z) )
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

//-----------------------------------------------------------
//	turn a Z along the line into a factor
//-----------------------------------------------------------
float TLMaths::TLine::GetFactorAlongZ(float z) const
{
	float Lengthz = m_End.z - m_Start.z;

	//	catch div by zero
	if ( Lengthz == 0.f )
	{
		return 0.f;
	}

	if ( Lengthz < 0.f )
	{
		TLDebug_Break("gr: check result of following code...");
	}

	z -= m_Start.z;
	return z / Lengthz;
}

	

//-----------------------------------------------------------
//	get a point along the line from 0..1 (factor)
//-----------------------------------------------------------
void TLMaths::TLine::GetPointAlongLine(float3& PointAlongLine,float Factor) const
{
	//	set to direction
	PointAlongLine = GetEnd();
	PointAlongLine -= GetStart();	//	point is now dir from start to end
	PointAlongLine *= Factor;		//	scale line to the factor
	PointAlongLine += GetStart();	//	move back to relative to the line start
}

//-----------------------------------------------------------
//	find the point along the line closest to Position
//-----------------------------------------------------------
float2 TLMaths::TLine2D::GetNearestPoint(const float2& Position,float& PointAlongLine) const
{
	float2 LineDir = GetDirection();
	float LineDirDotProduct = LineDir.DotProduct(LineDir);
	
	//	avoid div by zero
	if ( LineDirDotProduct == 0.f )
	{
		PointAlongLine = 0.f;
		return GetStart();
	}

	float2 Dist = Position - GetStart();

	float LineDirDotProductDist = LineDir.DotProduct(Dist);

	PointAlongLine = LineDirDotProductDist / LineDirDotProduct;

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

	if ( (TestIntersectionPos - IntersectionPos).Length() > TLMaths_NearZero )
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
//	get distance to another line
//-----------------------------------------------------------
float TLMaths::TLine2D::GetDistanceSq(const TLMaths::TLine2D& Line) const
{
	//	first, see if they intersect
	float IntersectionAlongThis, IntersectionAlongLine;
	SyncBool Intersection = GetIntersectionDistance( Line, IntersectionAlongThis, IntersectionAlongLine );

	//	intersected, distance is 0.
	if ( Intersection == SyncTrue )
		return 0.f;

	const float2* pNearestPointOnThis = NULL;
	const float2* pNearestPointOnLine = NULL;
	float2 TempPoint;

	//	use the capsule distance func to get distance and points
	float DistanceSq = GetDistanceSq( Line, TempPoint, &pNearestPointOnThis, &pNearestPointOnLine, Intersection, IntersectionAlongThis, IntersectionAlongLine );

	return DistanceSq;

	/*
	//	if lines keep going then they will intersect at some point
	if ( Intersection == SyncWait )
	{
		//	get distance as a factor
		float Distance = IntersectionAlongThis;

		//	if <0 then negate, if >1 then the distance from the end is the "extra"
		if ( IntersectionAlongThis < 0.f )
			Distance = -IntersectionAlongThis;
		else if ( IntersectionAlongThis >= 1.f )
			Distance = 1.f - IntersectionAlongThis;
		
		//	scale to line's length
		//	gr: can use LengthSq here i think
		//	gr: changed - should be THIS's length, not line's length
		//Distance *= Line.GetLength();
		Distance *= GetLengthSq();

		if ( Distance < 0.f )
		{
			TLDebug_Break("Distance should not be negative");
		}

		return Distance;
	}

	//	no possible intersection, find nearest point to either end and return shortest distance
	float2 StartNearest = GetNearestPoint( Line.GetStart() );
	float2 EndNearest = GetNearestPoint( Line.GetEnd() );

	float StartDistanceSq = (StartNearest - Line.GetStart()).LengthSq();
	float EndDistanceSq = (EndNearest - Line.GetEnd()).LengthSq();

	//	return shortest
	return (StartDistanceSq < EndDistanceSq) ? StartDistanceSq : EndDistanceSq;
	*/
}


//-----------------------------------------------------------
//	like GetIntersectionPos... return WAIT if the lines intersect past their extents
//-----------------------------------------------------------
SyncBool TLMaths::TLine2D::GetIntersectionDistance(const TLMaths::TLine2D& Line,float& IntersectionAlongThis,float& IntersectionAlongLine) const	
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
            return SyncFalse;//COINCIDENT;
        }
        return SyncFalse;// PARALLEL;
    }

	float& ua = IntersectionAlongThis;
	float& ub = IntersectionAlongLine;

    ua = numerator / denom;
    ub = numerator2/ denom;

	//	intersection will be past the ends of these lines
	if ( ua < 0.f || ua > 1.f )	return SyncWait;
	if ( ub < 0.f || ub > 1.f )	return SyncWait;

	return SyncTrue;
}



//-----------------------------------------------------------
//	checks for intersection and returns where along the line on both cases where it intersects
//-----------------------------------------------------------
Bool TLMaths::TLine2D::GetIntersectionPos(const TLine2D& Line,float& IntersectionAlongThis,float& IntersectionAlongLine) const
{
	SyncBool IntersectionResult = GetIntersectionDistance( Line, IntersectionAlongThis, IntersectionAlongLine );

	return ( IntersectionResult == SyncTrue );
}


//-----------------------------------------------------------
//	calculate angle of line
//-----------------------------------------------------------
TLMaths::TAngle TLMaths::TLine2D::GetAngle() const
{
	float2 Direction = GetDirection();

	//	gr: not sure why but have to invert this
	float AngRad = atan2f( -Direction.x, -Direction.y );

	TLMaths::TAngle Angle;
	Angle.SetRadians( AngRad );
	Angle.SetLimit180();

	return Angle;
}


//-----------------------------------------------------------
//	move the start point along the direction by an amount (NOT a factor)
//-----------------------------------------------------------
void TLMaths::TLine2D::MoveStart(float Distance)
{
	//	no change
	if ( Distance == 0.f )
		return;

	float2 DirectionNormal = GetDirectionNormal( Distance );
	m_Start += DirectionNormal;
}


//-----------------------------------------------------------
//	move the end point along the direction by an amount (NOT a factor)
//-----------------------------------------------------------
void TLMaths::TLine2D::MoveEnd(float Distance)
{
	//	no change
	if ( Distance == 0.f )
		return;

	float2 DirectionNormal = GetDirectionNormal( Distance );
	m_End += DirectionNormal;
}

//-----------------------------------------------------------
//	transform points of line
//-----------------------------------------------------------
void TLMaths::TLine2D::Transform(const TLMaths::TTransform& Transform)
{
	if ( Transform.HasAnyTransform() )
	{
		Transform.Transform( m_Start );
		Transform.Transform( m_End );
	}
}


//-----------------------------------------------------------
//	get a point along the line from 0..1 (factor)
//-----------------------------------------------------------
void TLMaths::TLine2D::GetPointAlongLine(float2& PointAlongLine,float Factor) const
{
	//	set to direction
	PointAlongLine = GetEnd();
	PointAlongLine -= GetStart();	//	point is now dir from start to end
	PointAlongLine *= Factor;		//	scale line to the factor
	PointAlongLine += GetStart();	//	move back to relative to the line start
}


//-----------------------------------------------------------
//	return whether this point is nearer to the start, or to the end. DistanceSq is set to the distance (commonly used afterwards)
//-----------------------------------------------------------
Bool TLMaths::TLine2D::GetIsPointNearestToStart(const float2& Pos,float& DistanceSq) const
{
	//	get distance to start
	float DistToStartSq = (Pos - m_Start).LengthSq();

	//	get distance to end
	DistanceSq = (Pos - m_End).LengthSq();

	//	is start closer?
	if ( DistToStartSq < DistanceSq )
	{
		DistanceSq = DistToStartSq;
		return TRUE;
	}
	else
	{
		//	closer to end
		return FALSE;
	}
}


//-----------------------------------------------------------
//	
//-----------------------------------------------------------
float TLMaths::TLine2D::GetDistanceSq(const TLMaths::TLine2D& Line,float2& TempPoint,const float2** ppNearestOnThis,const float2** ppNearestOnLine,SyncBool IntersectionResult,float IntersectionAlongThis,float IntersectionAlongLine) const
{
	const float2*& pNearestPointOnThis = *ppNearestOnThis;
	const float2*& pNearestPointOnShape = *ppNearestOnLine;
	//float2 TempPoint;	//	use temp point when we're not using a start or end

	//	distance between the two nearest points
	float DistanceSq = 0.f;

	const TLMaths::TLine2D& ThisCapsuleLine = *this;
	const TLMaths::TLine2D& ShapeCapsuleLine = Line;

	//	if extended, the lines WILL intersect, so we see if the sphere's on the end intersect (if a capsule doesnt intersect then the nearest two points on the lines MUST be end points)
	if ( IntersectionResult == SyncWait )
	{
		Bool OnThisLine = (IntersectionAlongThis>=0.f) && (IntersectionAlongThis<=1.f);
		Bool OnShapeLine = (IntersectionAlongLine>=0.f) && (IntersectionAlongLine<=1.f);

		//	shouldnt get this case
		if ( OnThisLine && OnShapeLine )
		{
			TLDebug_Break("error: lines are intersecting??");
		}
		
		if ( OnThisLine )
		{
			//	work out nearest end of ShapeLine to the point on ThisLine
			ThisCapsuleLine.GetPointAlongLine( TempPoint, IntersectionAlongThis );
			pNearestPointOnThis = &TempPoint;

			//	work out if start or end is closest
			Bool ShapeNearStart = ShapeCapsuleLine.GetIsPointNearestToStart( *pNearestPointOnThis, DistanceSq );

			//	do early distance check - and check we're not too embedded (eg. capsule are positioned exactly the same)
		//	if ( DistanceSq > TotalRadiusSq || DistanceSq < TLMaths_NearZero )
		//		return FALSE;

			pNearestPointOnShape = ShapeNearStart ? &ShapeCapsuleLine.GetStart() : &ShapeCapsuleLine.GetEnd();
			/*
			float RearNearestAlong;
			float2 RealNearest = ThisCapsuleLine.GetNearestPoint( *pNearestPointOnShape, RearNearestAlong );
			if ( fabsf(RearNearestAlong - IntersectionAlongThis) > 0.1f )
				TLDebug_Print("err");
				*/
		}
		else if ( OnShapeLine )
		{
			//	work out nearest end of ThisLine to the point on ShapeLine
			ShapeCapsuleLine.GetPointAlongLine( TempPoint, IntersectionAlongLine );
			pNearestPointOnShape = &TempPoint;

			//	work out if start or end is closest
			Bool ThisNearStart = ThisCapsuleLine.GetIsPointNearestToStart( *pNearestPointOnShape, DistanceSq );
			
			//	do early distance check - and check we're not too embedded (eg. capsule are positioned exactly the same)
		//	if ( DistanceSq > TotalRadiusSq || DistanceSq < TLMaths_NearZero )
		//		return FALSE;

			pNearestPointOnThis = ThisNearStart ? &ThisCapsuleLine.GetStart() : &ThisCapsuleLine.GetEnd();

			/*
			//	see if this really is the nearest point along SHAPE
			float RearNearestAlong;
			float2 RealNearest = ShapeCapsuleLine.GetNearestPoint( *pNearestPointOnThis, RearNearestAlong );
			if ( fabsf(RearNearestAlong - IntersectionAlongShape) > 0.1f )
				TLDebug_Print("err");
				*/
		}
		else
		{
			//	work out the nearest points of both lines
			Bool ThisNearStart = TRUE;
			Bool ShapeNearStart = TRUE;
			TLMaths::GetNearestLinePoints( ThisCapsuleLine, ShapeCapsuleLine, ThisNearStart, ShapeNearStart, DistanceSq );
			
			//	do early distance check - and check we're not too embedded (eg. capsule are positioned exactly the same)
		//	if ( DistanceSq > TotalRadiusSq || DistanceSq < TLMaths_NearZero )
		//		return FALSE;

			pNearestPointOnThis = ThisNearStart ? &ThisCapsuleLine.GetStart() : &ThisCapsuleLine.GetEnd();
			pNearestPointOnShape = ShapeNearStart ? &ShapeCapsuleLine.GetStart() : &ShapeCapsuleLine.GetEnd();
		}
	}
	else if ( IntersectionResult == SyncFalse )	//	lines will never intersect, so paralel, so we need to do some sort of box intersection test to see if they're close enough when side by side
	{
		//	work out the nearest points of both lines
		Bool ThisNearStart = TRUE;
		Bool ShapeNearStart = TRUE;
		TLMaths::GetNearestLinePoints( ThisCapsuleLine, ShapeCapsuleLine, ThisNearStart, ShapeNearStart, DistanceSq );

		//	do early distance check - and check we're not too embedded (eg. capsule are positioned exactly the same)
	//	if ( DistanceSq > TotalRadiusSq || DistanceSq < TLMaths_NearZero )
	//		return FALSE;

		pNearestPointOnThis = ThisNearStart ? &ThisCapsuleLine.GetStart() : &ThisCapsuleLine.GetEnd();
		pNearestPointOnShape = ShapeNearStart ? &ShapeCapsuleLine.GetStart() : &ShapeCapsuleLine.GetEnd();
	}
	else
	{
		TLDebug_Break("Invalid Intersection result");
		return 0.f;
	}

	if ( !pNearestPointOnThis || !pNearestPointOnShape )
	{
		TLDebug_Break("Missing pointer assignment");
	}

	/*
	//	do distance check to see if we intersect - and check we're not too embedded (eg. capsule are positioned exactly the same)
	if ( DistanceSq > TotalRadiusSq || DistanceSq < TLMaths_NearZero )
	{
		TLDebug_Break("This should have already been caught");
		return FALSE;
	}
	*/
	return DistanceSq;
}



void TLMaths::EvaluateQuadraticCurve(TArray<float3>& BezierPoints,const float3& A,const float3& B,const float3& C,u32 BezierSteps)
{
	//	note: starts at 1 as first point is NOT a point on the curve, it's the ON from before
	for( u32 i=1;	i<BezierSteps;	i++)
    {
        float t = (float)i / (float)BezierSteps;
		float invt = 1.f - t;
		
        float3 U = A * invt + B * t;
        float3 V = B * invt + C * t;
		float3 Point = U * invt + V * t;
        
		BezierPoints.Add( Point );
    }
}