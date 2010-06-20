#include "TTessellate.h"
#include <TootleAsset/TMesh.h>



namespace TLMaths
{
	namespace TLTessellator
	{
		const u32	g_BezierStepMin = 1;		//	
		const float	g_BezierStepRate = 1.15f;	//	make beizer step count relative to the lngth of the curve.  a bezier every 0.1m. Less value is more divisions
	}
}
	


//-----------------------------------------------------------------
//	based on a distance, work out how many bezier steps to produce
//-----------------------------------------------------------------
u32 TLMaths::GetBezierStepCount(float LineDistance)
{
	float BezierStepsf = (LineDistance / TLMaths::TLTessellator::g_BezierStepRate) + 0.5f;	//	round up
	
	u32 BezierSteps = (u32)BezierStepsf;
	if ( BezierSteps < TLMaths::TLTessellator::g_BezierStepMin )
		BezierSteps = TLMaths::TLTessellator::g_BezierStepMin;
	
	return BezierSteps;
}


TLMaths::TContour::TContour(const TLMaths::TContour& Contour) :
	m_IsClockwise	( Contour.IsClockwise() )
{
	m_Points.Copy( Contour.GetPoints() );
}



TLMaths::TContour::TContour(const TArray<float3>& Contours,const TArray<TLMaths::TContourCurve>* pContourCurves) :
	m_IsClockwise	( TRUE )
{
	u32 n = Contours.GetSize();
	float3 cur = Contours[(n - 1) % n];
	float3 next = Contours[0];
    float3 prev;
    float3 a;
	float3 b = next - cur;
	float olddir, dir = TLMaths::Atan2f((next - cur).y, (next - cur).x);
    float angle = 0.0f;
	
	
    // See http://freetype.sourceforge.net/freetype2/docs/glyphs/glyphs-6.html
    // for a full description of FreeType tags.
    for( u32 i = 0; i < n; i++)
    {
        prev = cur;
        cur = next;
        next = Contours[(i + 1) % n];
        olddir = dir;
		dir = TLMaths::Atan2f( (next - cur).y, (next - cur).x );
		
        // Compute our path's new direction.
        float t = dir - olddir;
        if(t < -PI) t += 2 * PI;
        if(t > PI) t -= 2 * PI;
        angle += t;
		
        // Only process point tags we know.
		if ( !pContourCurves )
		{
			m_Points.Add(cur);
			continue;
		}
		
		TLMaths::TContourCurve CurveType = pContourCurves->ElementAtConst(i);
		TLMaths::TContourCurve NextCurveType = pContourCurves->ElementAtConst( (i + 1) % n );
		TLMaths::TContourCurve PrevCurveType = pContourCurves->ElementAtConst( (i - 1 + n) % n );
		
		if( n < 2 || CurveType == TLMaths::ContourCurve_On )
        {
            m_Points.Add(cur);
        }
        else if( CurveType == TLMaths::ContourCurve_Conic )
        {
            float3 prev2 = prev;
			float3 next2 = next;
			
            // Previous point is either the real previous point (an "on"
            // point), or the midpoint between the current one and the
            // previous "conic off" point.
            if ( PrevCurveType == TLMaths::ContourCurve_Conic)
            {
                prev2 = (cur + prev) * 0.5f;
                m_Points.Add(prev2);
            }
			
            // Next point is either the real next point or the midpoint.
            if ( NextCurveType == TLMaths::ContourCurve_Conic)
            {
                next2 = (cur + next) * 0.5f;
            }
			
            evaluateQuadraticCurve(prev2, cur, next2);
        }
        else if( CurveType == TLMaths::ContourCurve_Cubic && NextCurveType == TLMaths::ContourCurve_Cubic )
        {
			float3 f = Contours[(i + 2) % n];
            evaluateCubicCurve(prev, cur, next, f );
			
			//	gr: the 2nd cubic here should be ignored shouldnt it??
        }
		else
		{
			//	ignore this combination - eg. second cubic in curve
		}
    }
	
    // If final angle is positive (+2PI), it's an anti-clockwise contour,
    // otherwise (-2PI) it's clockwise.
	m_IsClockwise = (angle < 0.0) ? TRUE : FALSE;
}


void TLMaths::TContour::evaluateQuadraticCurve(const float3& FromOn,const float3& FromControl,const float3& ToOn)
{
	const float3& A = FromOn;
	const float3& B = FromControl;
	const float3& C = ToOn;
	
	//	work out how many bezier steps to do
	float PointDistanceSq = (A - B).LengthSq() + (B - C).LengthSq();
	u32 BezierSteps = GetBezierStepCount( TLMaths::Sqrtf( PointDistanceSq ) );
	
	TLMaths::EvaluateQuadraticCurve( m_Points, A, B, C, BezierSteps );
}


void TLMaths::TContour::evaluateCubicCurve(const float3& FromOn,const float3& FromControl,const float3& ToControl,const float3& ToOn)
{
	const float3& A = FromOn;
	const float3& B = FromControl;
	const float3& C = ToControl;
	const float3& D = ToOn;
	
	//	work out how many bezier steps to do
	float PointDistanceSq = (A - B).LengthSq() + (B - C).LengthSq() + (C - D).LengthSq();
	u32 BezierSteps = GetBezierStepCount( TLMaths::Sqrtf( PointDistanceSq ) );

	//	gr: should this start at 1? Isn't FromOn already in the contour?...
    for( u32 i=0;	i<BezierSteps;	i++ )
    {
        float t = static_cast<float>(i) / (float)BezierSteps;
		float invt = 1.f - t;
		
        float3 U = A * invt + B * t;
        float3 V = B * invt + C * t;
        float3 W = C * invt + D * t;
		
        float3 M = U * invt + V * t;
        float3 N = V * invt + W * t;
		
		float3 Point = M * invt + N * t;
		
        m_Points.Add( Point );
    }
}


//----------------------------------------------------------
//	scale down the shape using outset calculations
//----------------------------------------------------------
void TLMaths::TContour::Shrink(float OutsetDistance)
{
	THeapArray<float3> OldPoints;
	OldPoints.Copy( m_Points );

	//	get a bounds sphere for the old points and we can use that radius to detect
	//	shrinking intersections (rather than some arbirtry massive length for the line)
	TLMaths::TSphere2D BoundsSphere;
	BoundsSphere.Accumulate( m_Points );
	
	THeapArray<TLMaths::TLine2D> EdgeLines;
	GetEdgeLines( EdgeLines );

	//	calculate the new outset point for each point
	for ( s32 i=0;	i<(s32)m_Points.GetSize();	i++ )
	{
		OutsetPoint( i, -OutsetDistance, m_Points, OldPoints, IsClockwise() );

		//	check intersection on shrink,
		//	dont allow this line past half way in the shape...
		//	do this by... extending the line until it hits the other side? 
		//	WHEN it does, half the intersection distance, if we go past this point then
		//	snap
		float TestLineLength = (BoundsSphere.GetRadius() * 2.f);
		float2 LineStart( OldPoints[i].xy() );
		float2 LineDir( m_Points[i].x - LineStart.x, m_Points[i].y - LineStart.y );
		LineDir.Normalise();
		TLMaths::TLine2D OutsetLine( LineStart, LineStart );
		OutsetLine.m_End += LineDir * TestLineLength;

		Bool ShortestIntersectionAlongOutsetIsValid = FALSE;
		float ShortestIntersectionAlongOutset = 0.f;

		//	find shortest intersection with the old shape
		for ( u32 e=0;	e<EdgeLines.GetSize();	e++ )
		{
			//	skip check if 
			if ( LineStart == EdgeLines[e].GetStart() )
				continue;
			if ( LineStart == EdgeLines[e].GetEnd() )
				continue;
			/*
			//	edge starts on i
			if ( e == i )	continue;

			//	edge ends on i
			s32 iplus = i+1;
			TLMaths::Wrap( iplus, 0, (s32)m_Points.GetSize() );
			if ( e == iplus )	continue;
			*/

			//	check to see if our new outset intersects with this edge
			float IntersectionAlongOutset,IntersectionAlongEdge;
			if ( !OutsetLine.GetIntersectionPos( EdgeLines[e], IntersectionAlongOutset, IntersectionAlongEdge ) )
				continue;

			//	intersected, check to see if its the new shortest
			if ( !ShortestIntersectionAlongOutsetIsValid || IntersectionAlongOutset<ShortestIntersectionAlongOutset )
			{
				ShortestIntersectionAlongOutsetIsValid = TRUE;
				ShortestIntersectionAlongOutset = IntersectionAlongOutset;
			}
		}

		//	didn't intersect (gr; wierd, I think we always intersect because of how far we test...)
		if ( !ShortestIntersectionAlongOutsetIsValid )
			continue;

		//	intersected, move the outset
		
		//	get how far it is to the other line...
		float IntersectionDist = ( ShortestIntersectionAlongOutset * TestLineLength );

		//	half it, because that's the furthest we'd want to move...
		//	if we hit the edge, we wanna stop half way between our old pos and this edge
		IntersectionDist *= 0.5f;

		//	we weren't going to move that far anyway!
		if ( IntersectionDist > OutsetDistance )
			continue;

		//	get the pos of the edge we hit
		float2 DirToIntersection = LineDir * IntersectionDist;	//	* fraction * len

		m_Points[i].xy() += OldPoints[i].xy() + DirToIntersection;
	}
}


//----------------------------------------------------------
//	scale down the shape using outset calculations
//----------------------------------------------------------
void TLMaths::TContour::Grow(float OutsetDistance)
{
	THeapArray<float3> OldPoints;
	OldPoints.Copy( m_Points );

	//	calculate the new outset point for each point
	for ( s32 i=0;	i<(s32)m_Points.GetSize();	i++ )
	{
		OutsetPoint( i, OutsetDistance, m_Points, OldPoints, IsClockwise() );
	}

}

//----------------------------------------------------------
//	move point in/out with outset
//	static
//----------------------------------------------------------
void TLMaths::TContour::OutsetPoint(u32 Index,float Distance,TArray<float3>& NewPoints,const TArray<float3>& OriginalPoints,Bool ContourIsClockwise)
{
	s32 IndexPrev = (s32)Index-1;
	s32 IndexNext = Index+1;
	TLMaths::Wrap( IndexPrev, 0, (s32)OriginalPoints.GetSize() );
	TLMaths::Wrap( IndexNext, 0, (s32)OriginalPoints.GetSize() );

	float3 Outset = TLMaths::GetLineStripOutset(OriginalPoints[IndexPrev], OriginalPoints[Index], OriginalPoints[IndexNext], Distance );

	if ( !ContourIsClockwise )
	{
		Outset.x *= -1.f;
		Outset.y *= -1.f;
	}

	NewPoints[Index] += Outset;
}

	
//-------------------------------------------------------------
//	get all the lines around the edge of the contour
//-------------------------------------------------------------
void TLMaths::TContour::GetEdgeLines(TArray<TLMaths::TLine>& EdgeLines) const
{
	//	generate the edge lines
	for ( u32 a=0;	a<m_Points.GetSize();	a++ )
	{
		s32 aa = a+1;
		
		//	loop around
		if ( aa == m_Points.GetSize() )
			aa = 0;

		const float3& PointA = m_Points[a];
		const float3& PointAA = m_Points[aa];

		EdgeLines.Add( TLMaths::TLine( PointA, PointAA ) );
	}
}


	
//-------------------------------------------------------------
//	get all the lines around the edge of the contour
//-------------------------------------------------------------
void TLMaths::TContour::GetEdgeLines(TArray<TLMaths::TLine2D>& EdgeLines) const
{
	//	generate the edge lines
	for ( u32 a=0;	a<m_Points.GetSize();	a++ )
	{
		s32 aa = a+1;
		
		//	loop around
		if ( aa == m_Points.GetSize() )
			aa = 0;

		const float3& PointA = m_Points[a];
		const float3& PointAA = m_Points[aa];

		EdgeLines.Add( TLMaths::TLine2D( PointA, PointAA ) );
	}
}


//-------------------------------------------------------------
//	check to make sure any lines along the contour dont intersect each other (a self intersecting polygon). returns TRUE if they do
//	gr: note: this is 2D only at the moment
//-------------------------------------------------------------
Bool TLMaths::TContour::HasIntersections() const
{
	//	generate the edge lines
	THeapArray<TLMaths::TLine2D> ContourEdges;
	GetEdgeLines( ContourEdges );

	for ( u32 e=0;	e<ContourEdges.GetSize();	e++ )
	{
		TLMaths::TLine2D& Edgee = ContourEdges[e];

		//	gr: start at +2... we don't check against the line connected directly to us (the end of the line WILL intersect)
		for ( u32 f=0;	f<m_Points.GetSize();	f++ )
		{
			if ( e==f )
				continue;

			TLMaths::TLine2D& Edgef = ContourEdges[f];

			if ( Edgee.GetEnd() == Edgef.GetStart() )	
				continue;
			if ( Edgef.GetEnd() == Edgee.GetStart() )	
				continue;

			if ( Edgee.GetIntersection( Edgef ) )
				return TRUE;
		}
	}

	return FALSE;

}



void TLMaths::TContour::SetParity(u32 parity)
{
    u32 size = m_Points.GetSize();
	
    if(((parity & 1) && IsClockwise()) || (!(parity & 1) && !IsClockwise()))
    {
        // Contour orientation is wrong! We must reverse all points.
        // FIXME: could it be worth writing FTVector::reverse() for this?
		// [26/01/10] DB - No.  This should be done internally within the array using the array sort order policy.
		// It shoudl use the array SetOrder() interface routine (when complete) to swap the order as required and will be
		// policy specific: 
		// * In a no sort order policy class it should simply flip the array back and forth like it is done here.  
		// * In a sorted policy class it should re-sort the array with the order required or do nothing if necessary.
		//TODO: This needs moving into the array handling
        for ( u32 i=0;	i<size/2;	i++ )
        {
			//	gr: use swap func
			m_Points.SwapElements( i, size - 1 - i );
            //float3 tmp = m_Points[i];
            //m_Points[i] = m_Points[size - 1 - i];
            //m_Points[size - 1 -i] = tmp;
        }
		
		m_IsClockwise = !m_IsClockwise;
    }

/*
    for ( u32 i=0;	i<size;	i++ )
    {
        u32 prev, cur, next;
		
        prev = (i + size - 1) % size;
        cur = i;
        next = (i + size + 1) % size;
		
		const float3& a = Point(prev);
		const float3& b = Point(cur);
		const float3& c = Point(next);
        float3 vOutset = ComputeOutsetPoint( a, b, c, 64.f );
        AddOutsetPoint(vOutset);
    }
	*/
}


//-----------------------------------------------
//	get area of the shape
//-----------------------------------------------
float TLMaths::TContour::GetArea() const
{
	float Area = 0.f;

	for ( s32 i=0;	i<m_Points.GetLastIndex();	i++ )
	{
		Area += ( m_Points[i+1].x * m_Points[i].y - m_Points[i].x * m_Points[i+1].y) * 0.5f;
	}

	TLDebug_Break("gr: don't think this is accurate... may vary based on clockwise/ccw?");

	//	if the shape is clockwise, Area will be negative, so abs it
	return TLMaths::Absf( Area );
}


//-----------------------------------------------
//	get bounds of contour
//-----------------------------------------------
void TLMaths::TContour::GetBoundsBox(TLMaths::TBox& BoundsBox)
{
	BoundsBox.SetInvalid();
	BoundsBox.Accumulate( m_Points );
}


//-----------------------------------------------
//	get center of contour - from bounds, possibly not true center
//-----------------------------------------------
float3 TLMaths::TContour::GetCenter()
{
	TLMaths::TBox BoundsBox;
	GetBoundsBox( BoundsBox );

	return BoundsBox.GetCenter();
}




TLMaths::TTessellator::TTessellator(TPtr<TLAsset::TMesh>& pMesh) : 
	m_pMesh				( pMesh ),
	m_VertexColourValid	( FALSE )
{
}













TLMaths::TSimpleTessellator::TSimpleTessellator(TPtr<TLAsset::TMesh>& pMesh) :
	TLMaths::TTessellator	( pMesh )
{
}



Bool TLMaths::TSimpleTessellator::GenerateTessellations(TLMaths::TLTessellator::TWindingMode WindingMode,float zNormal)
{
	return FALSE;
}

