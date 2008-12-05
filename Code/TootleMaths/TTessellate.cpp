#include "TTessellate.h"
#include <TootleAsset/TMesh.h>



namespace TLMaths
{
	namespace TLTessellator
	{
		const u32	g_BezierStepMin = 1;		//	
		const float	g_BezierStepRate = 1.0f;	//	make beizer step count relative to the lngth of the curve.  a bezier every 0.1m
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

	
TLMaths::TContour::TContour(const TArray<float3>& Contours,const TArray<TLMaths::TContourCurve>* pContourCurves) :
	m_Clockwise	( SyncWait )
{
	u32 n = Contours.GetSize();
	float3 cur = Contours[(n - 1) % n];
	float3 next = Contours[0];
    float3 prev;
    float3 a;
	float3 b = next - cur;
    float olddir, dir = atan2f((next - cur).y, (next - cur).x);
    float angle = 0.0;
	
	
    // See http://freetype.sourceforge.net/freetype2/docs/glyphs/glyphs-6.html
    // for a full description of FreeType tags.
    for( u32 i = 0; i < n; i++)
    {
        prev = cur;
        cur = next;
        next = Contours[(i + 1) % n];
        olddir = dir;
        dir = atan2f( (next - cur).y, (next - cur).x );
		
        // Compute our path's new direction.
        float t = dir - olddir;
        if(t < -PI) t += 2 * PI;
        if(t > PI) t -= 2 * PI;
        angle += t;
		
        // Only process point tags we know.
		if ( !pContourCurves )
		{
			AddPoint(cur);
			continue;
		}
		
		TLMaths::TContourCurve CurveType = pContourCurves->ElementAtConst(i);
		TLMaths::TContourCurve NextCurveType = pContourCurves->ElementAtConst( (i + 1) % n );
		TLMaths::TContourCurve PrevCurveType = pContourCurves->ElementAtConst( (i - 1 + n) % n );
		
		if( n < 2 || CurveType == TLMaths::ContourCurve_On )
        {
            AddPoint(cur);
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
                AddPoint(prev2);
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
	m_Clockwise = (angle < 0.0) ? SyncTrue : SyncFalse;
}



void TLMaths::TContour::AddPoint(const float3& point)
{
    if ( pointList.GetSize() )
	{
		if ( point == pointList.ElementLast() )
			return;
		
		if ( point == pointList[0] )
			return;
	}
	
	TLDebug_CheckFloat(point);
	pointList.Add(point);
}


void TLMaths::TContour::AddOutsetPoint(const float3& point)
{
	TLDebug_CheckFloat(point);
    outsetPointList.Add(point);
}


void TLMaths::TContour::AddFrontPoint(const float3& point)
{
	TLDebug_CheckFloat(point);
    frontPointList.Add(point);
}


void TLMaths::TContour::AddBackPoint(const float3& point)
{
	TLDebug_CheckFloat(point);
    backPointList.Add(point);
}


void TLMaths::TContour::evaluateQuadraticCurve(const float3& FromOn,const float3& FromControl,const float3& ToOn)
{
	const float3& A = FromOn;
	const float3& B = FromControl;
	const float3& C = ToOn;
	
	//	work out how many bezier steps to do
	float PointDistanceSq = (A - B).LengthSq() + (B - C).LengthSq();
	u32 BezierSteps = GetBezierStepCount( TLMaths::Sqrtf( PointDistanceSq ) );
	
	//	note: starts at 1 as first point is NOT a point on the curve, it's the ON from before
	for( u32 i=1;	i<BezierSteps;	i++)
    {
        float t = static_cast<float>(i) / (float)BezierSteps;
		float invt = 1.f - t;
		
        float3 U = A * invt + B * t;
        float3 V = B * invt + C * t;
		float3 Point = U * invt + V * t;
        
		AddPoint( Point );
    }
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
		
        AddPoint( Point );
    }
}


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
float3 TLMaths::TContour::ComputeOutsetPoint(const float3& A,const float3& B,const float3& C)
{
	TLDebug_CheckFloat( A );
	TLDebug_CheckFloat( B );
	TLDebug_CheckFloat( C );
	
    /* Build the rotation matrix from 'ba' vector */
    float3 ba = (A - B);
	ba.Normalise();
    float3 bc = C - B;
	
	TLDebug_CheckFloat( ba );
	TLDebug_CheckFloat( bc );
	
    /* Rotate bc to the left */
    float3 tmp(bc.x * -ba.x + bc.y * -ba.y,
			   bc.x * ba.y + bc.y * -ba.x,
			   0.f );
	
	TLDebug_CheckFloat( tmp );
	
    /* Compute the vector bisecting 'abc' */
    float norm = sqrt(tmp.x * tmp.x + tmp.y * tmp.y);
    
	float dist = 0;
	float normplusx = norm + tmp.x;
	float normminusx = norm - tmp.x;
	if ( normplusx != 0.0 )
	{
		float xdiv = normminusx / normplusx;
		if ( xdiv != 0 )
		{
			float sqrtxdiv = sqrtf(xdiv);
			dist = 64.f * sqrtxdiv;
		}
	}
	
    tmp.x = tmp.y < 0.0 ? dist : -dist;
    tmp.y = 64.f;
	
	TLDebug_CheckFloat( tmp );
	
    /* Rotate the new bc to the right */
    return float3(tmp.x * -ba.x + tmp.y * ba.y,
				  tmp.x * -ba.y + tmp.y * -ba.x,
				  0.f );
}


void TLMaths::TContour::SetParity(u32 parity)
{
    u32 size = PointCount();
    float3 vOutset;
	
    if(((parity & 1) && m_Clockwise==SyncTrue) || (!(parity & 1) && m_Clockwise==SyncFalse))
    {
        // Contour orientation is wrong! We must reverse all points.
        // FIXME: could it be worth writing FTVector::reverse() for this?
        for ( u32 i=0;	i<size/2;	i++ )
        {
			//	gr: use swap func
			pointList.SwapElements( i, size - 1 - i );
            //float3 tmp = pointList[i];
            //pointList[i] = pointList[size - 1 - i];
            //pointList[size - 1 -i] = tmp;
        }
		
		//clockwise = !clockwise;
		if ( m_Clockwise == SyncTrue )
			m_Clockwise = SyncFalse;
		else
			m_Clockwise = SyncTrue;
        
    }
	
    for ( u32 i=0;	i<size;	i++ )
    {
        u32 prev, cur, next;
		
        prev = (i + size - 1) % size;
        cur = i;
        next = (i + size + 1) % size;
		
		const float3& a = Point(prev);
		const float3& b = Point(cur);
		const float3& c = Point(next);
        float3 vOutset = ComputeOutsetPoint( a, b, c );
        AddOutsetPoint(vOutset);
    }
}


void TLMaths::TContour::buildFrontOutset(float outset)
{
    for( u32 i=0;	i<PointCount();	i++)
    {
		const float3& p = Point(i);
		const float3& o = Outset(i);
        AddFrontPoint(p + o * outset);
    }
}


void TLMaths::TContour::buildBackOutset(float outset)
{
    for( u32 i=0;	i<PointCount();	i++)
    {
		const float3& p = Point(i);
  		const float3& o = Outset(i);
		AddBackPoint(p + o * outset);
    }
}







TLMaths::TTessellator::TTessellator(TPtr<TLAsset::TMesh>& pMesh) : 
	m_pMesh				( pMesh ),
	m_VertexColourValid	( FALSE )
{
}



