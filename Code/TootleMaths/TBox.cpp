#include "TBox.h"
#include "TLine.h"
#include "TSphere.h"
#include "TCapsule.h"
#include <TootleCore/TFixedArray.h>
#include <TootleCore/TString.h>





TLMaths::TBox::TBox() :
	m_IsValid	( FALSE )
{
}


TLMaths::TBox::TBox(const float3& Min,const float3& Max) : 
	m_Min		( Min ), 
	m_Max		( Max ),
	m_IsValid	( TRUE )
{
}


//-------------------------------------------------
//	grow the box to these extents
//-------------------------------------------------
void TLMaths::TBox::Accumulate(const float3& Point)
{
	//	if we're not valid, set box to the "size" of this point
	if ( !IsValid() )
	{
		Set( Point, Point );
		return;
	}

	if ( Point.x < m_Min.x )	m_Min.x = Point.x;
	if ( Point.y < m_Min.y )	m_Min.y = Point.y;
	if ( Point.z < m_Min.z )	m_Min.z = Point.z;

	if ( Point.x > m_Max.x )	m_Max.x = Point.x;
	if ( Point.y > m_Max.y )	m_Max.y = Point.y;
	if ( Point.z > m_Max.z )	m_Max.z = Point.z;
}


//-------------------------------------------------
//	get the extents of all these points
//-------------------------------------------------
void TLMaths::TBox::Accumulate(const TArray<float3>& Points)
{
	for ( u32 i=0;	i<Points.GetSize();	i++ )
	{
		Accumulate( Points[i] );
	}
}


//-------------------------------------------------
//	merge two boxes, box will only get bigger
//-------------------------------------------------
void TLMaths::TBox::Accumulate(const TBox& Box)			
{
	//	source box is not valid!
	if ( !Box.IsValid() )
	{
		if ( !TLDebug_Break("Accumulating invalid box") )
			return;
	}

	//	if we're not valid, just copy the box
	if ( !IsValid() )
	{
		Set( Box.GetMin(), Box.GetMax() );
	}
	else
	{
		Accumulate( Box.GetMin() );	
		Accumulate( Box.GetMax() );	
	}
}	


//-------------------------------------------------
//	accumulate sphere
//-------------------------------------------------
void TLMaths::TBox::Accumulate(const TSphere& Sphere)			
{
	//	source sphere is not valid!
	if ( !Sphere.IsValid() )
	{
		if ( !TLDebug_Break("Box accumulating invalid sphere") )
			return;
	}

	float Rad = Sphere.GetRadius();
	float3 SphereMin = Sphere.GetPos() - float3( Rad, Rad, Rad );
	float3 SphereMax = Sphere.GetPos() + float3( Rad, Rad, Rad );

	//	if we're not valid, just copy the box
	if ( !IsValid() )
	{
		Set( SphereMin, SphereMax );
	}
	else
	{
		Accumulate( SphereMin );	
		Accumulate( SphereMax );	
	}
}	


//-------------------------------------------------
//	get the center of the min/max
//-------------------------------------------------
float3 TLMaths::TBox::GetCenter() const	
{
	float3 Center( m_Max );
	Center -= m_Min;
	Center /= 2.f;
	Center += m_Min;

	return Center;
}



//-------------------------------------------------
//	get the 8 corners of the box
//-------------------------------------------------
void TLMaths::TBox::GetBoxCorners(TArray<float3>& CornerPositions) const
{
	/*
			0 ____________ 1
			 /           /|
			/|   (-)    / |
		   / |         /  |
		3 /__|________/ 2 |
(-)		 |   |        |   |			(+)
		 |  4|________|___| 5
		 |   /        |   /
		 |  /         |  /
		 | /    (+)   | /
		 |/___________|/
		 7             6

 */
	//	box top
	CornerPositions.Add( float3( m_Min.x,	m_Min.y,	m_Min.z ) );
	CornerPositions.Add( float3( m_Max.x,	m_Min.y,	m_Min.z ) );
	CornerPositions.Add( float3( m_Max.x,	m_Min.y,	m_Max.z ) );
	CornerPositions.Add( float3( m_Min.x,	m_Min.y,	m_Max.z ) );

	//	box bottom
	CornerPositions.Add( float3( m_Min.x,	m_Max.y,	m_Min.z ) );
	CornerPositions.Add( float3( m_Max.x,	m_Max.y,	m_Min.z ) );
	CornerPositions.Add( float3( m_Max.x,	m_Max.y,	m_Max.z ) );
	CornerPositions.Add( float3( m_Min.x,	m_Max.y,	m_Max.z ) );
}


//-------------------------------------------------
//	transform box - this transforms each corner, then gets
//	a new/min max and hence a new box
//-------------------------------------------------
void TLMaths::TBox::Transform(const TLMaths::TMatrix& Matrix,const float3& Scale)
{
	if ( !IsValid() )
		return;

	//	gr: I'm pretty sure transforming the min & max wont work quite right, so transform each point
	//		and make a new bounds box
	TFixedArray<float3,8> BoundsPoints(0);
	GetBoxCorners( BoundsPoints );

	//	transform the points
	for ( u32 i=0;	i<BoundsPoints.GetSize();	i++ )
	{
		BoundsPoints[i] *= Scale;
		Matrix.TransformVector( BoundsPoints[i] );
	}

	//	make new bounds
	SetInvalid();
	Accumulate( BoundsPoints );
}



//-------------------------------------------------
//	transform box - this transforms each corner, then gets
//	a new/min max and hence a new box
//-------------------------------------------------
void TLMaths::TBox::Transform(const TLMaths::TTransform& Transform)
{
	//	not valid, or not going to make any changes... skip
	if ( !IsValid() || !Transform.HasAnyTransform() )
		return;

	//	gr: I'm pretty sure transforming the min & max wont work quite right, so transform each point
	//		and make a new bounds box
	TFixedArray<float3,8> BoundsPoints(0);
	GetBoxCorners( BoundsPoints );

	//	transform the points
	for ( u32 i=0;	i<BoundsPoints.GetSize();	i++ )
	{
		Transform.TransformVector( BoundsPoints[i] );
	}

	//	make new bounds
	SetInvalid();
	Accumulate( BoundsPoints );
}

//--------------------------------------------------------
//	untransform box
//	note: must be opposite order to transform!
//--------------------------------------------------------
void TLMaths::TBox::Untransform(const TLMaths::TTransform& Transform)
{
	if ( Transform.HasTranslate() )
	{
		m_Min -= Transform.GetTranslate();
		m_Max -= Transform.GetTranslate();
	}

	if ( Transform.HasMatrix() )
	{
		TLDebug_Break("todo: undo transform box");
		//Transform.m_Matrix.UnTransformVector( m_Pos );
	}

	if ( Transform.HasRotation() )
	{
		TLDebug_Break("todo: undo rotaion transform box");
		//Transform.m_Matrix.UnTransformVector( m_Pos );
	}

	if ( Transform.HasScale() )
	{
		const float3& Scale = Transform.GetScale();
		m_Min /= Scale;
		m_Max /= Scale;
	}
}


Bool TempGetIntersection( float fDst1, float fDst2,const float3& P1,const float3& P2,float3& Hit) 
{
	if ( (fDst1 * fDst2) > 0.0f)	 
		return FALSE;
	if ( fDst1 == fDst2)			
		return FALSE; 
	Hit = P1 + (P2-P1) * ( -fDst1/(fDst2-fDst1) );
	return TRUE;
}

Bool TempInBox(float3& Hit,const float3& Min, const float3& Max, const u32 Axis) 
{
	if ( Axis==1 && Hit.z >= Min.z && Hit.z <= Max.z && Hit.y >= Min.y && Hit.y <= Max.y)
		return TRUE;
	if ( Axis==2 && Hit.z >= Min.z && Hit.z <= Max.z && Hit.x >= Min.x && Hit.x <= Max.x) 
		return TRUE;
	if ( Axis==3 && Hit.x >= Min.x && Hit.x <= Max.x && Hit.y >= Min.y && Hit.y <= Max.y) 
		return TRUE;
	return FALSE;
}

//-------------------------------------------------
//	test to see if this line intersects our box
//-------------------------------------------------
Bool TLMaths::TBox::GetIntersection(const TLine& Line) const
{
	const float3& B1 = m_Min;
	const float3& B2 = m_Max;
	const float3& L1 = Line.GetStart();
	const float3& L2 = Line.GetEnd();
	float3 Hit;

	if (L2.x < B1.x && L1.x < B1.x) return FALSE;
	if (L2.x > B2.x && L1.x > B2.x) return FALSE;
	if (L2.y < B1.y && L1.y < B1.y) return FALSE;
	if (L2.y > B2.y && L1.y > B2.y) return FALSE;
	if (L2.z < B1.z && L1.z < B1.z) return FALSE;
	if (L2.z > B2.z && L1.z > B2.z) return FALSE;
	
	if ( GetIntersection( L1 ) )
		return TRUE;

	if ( TempGetIntersection( L1.x-B1.x, L2.x-B1.x, L1, L2, Hit) && TempInBox( Hit, B1, B2, 1 ) )
		return TRUE;
	if ( TempGetIntersection( L1.y-B1.y, L2.y-B1.y, L1, L2, Hit) && TempInBox( Hit, B1, B2, 2 ) )
		return TRUE;
	if ( TempGetIntersection( L1.z-B1.z, L2.z-B1.z, L1, L2, Hit) && TempInBox( Hit, B1, B2, 3 ) )
		return TRUE;
	if ( TempGetIntersection( L1.x-B2.x, L2.x-B2.x, L1, L2, Hit) && TempInBox( Hit, B1, B2, 1 ) )
		return TRUE;
	if ( TempGetIntersection( L1.y-B2.y, L2.y-B2.y, L1, L2, Hit) && TempInBox( Hit, B1, B2, 2 ) )
		return TRUE;
	if ( TempGetIntersection( L1.z-B2.z, L2.z-B2.z, L1, L2, Hit) && TempInBox( Hit, B1, B2, 3 ) )
		return TRUE;

	return FALSE;
}



Bool TempInBox(const TLMaths::TSphere& HitSphere,const float3& Min, const float3& Max, const u32 Axis) 
{
	const float& Rad = HitSphere.GetRadius();
	const float3& Hit = HitSphere.GetPos();

	if ( Axis==1 && Hit.z >= Min.z-Rad && Hit.z <= Max.z+Rad && Hit.y >= Min.y-Rad && Hit.y <= Max.y+Rad)
		return TRUE;
	if ( Axis==2 && Hit.z >= Min.z-Rad && Hit.z <= Max.z+Rad && Hit.x >= Min.x-Rad && Hit.x <= Max.x+Rad) 
		return TRUE;
	if ( Axis==3 && Hit.x >= Min.x-Rad && Hit.x <= Max.x+Rad && Hit.y >= Min.y-Rad && Hit.y <= Max.y+Rad) 
		return TRUE;
	return FALSE;
}


//-------------------------------------------------
//	test to see if this line intersects our box
//-------------------------------------------------
Bool TLMaths::TBox::GetIntersection(const TCapsule& Capsule) const
{
	const float3& B1 = m_Min;
	const float3& B2 = m_Max;
	const float3& L1 = Capsule.GetLine().GetStart();
	const float3& L2 = Capsule.GetLine().GetEnd();
	float Rad = Capsule.GetRadius();
	TSphere Hit;
	Hit.SetRadius( Rad );

	if (L2.x+Rad < B1.x && L1.x+Rad < B1.x) return FALSE;
	if (L2.x-Rad > B2.x && L1.x-Rad > B2.x) return FALSE;
	if (L2.y+Rad < B1.y && L1.y+Rad < B1.y) return FALSE;
	if (L2.y-Rad > B2.y && L1.y-Rad > B2.y) return FALSE;
	if (L2.z+Rad < B1.z && L1.z+Rad < B1.z) return FALSE;
	if (L2.z-Rad > B2.z && L1.z-Rad > B2.z) return FALSE;
	
	if ( GetIntersection( L1 ) )
		return TRUE;

	if ( TempGetIntersection( L1.x-B1.x, L2.x-B1.x, L1, L2, Hit.GetPos()) && TempInBox( Hit, B1, B2, 1 ) )
		return TRUE;
	if ( TempGetIntersection( L1.y-B1.y, L2.y-B1.y, L1, L2, Hit.GetPos()) && TempInBox( Hit, B1, B2, 2 ) )
		return TRUE;
	if ( TempGetIntersection( L1.z-B1.z, L2.z-B1.z, L1, L2, Hit.GetPos()) && TempInBox( Hit, B1, B2, 3 ) )
		return TRUE;
	if ( TempGetIntersection( L1.x-B2.x, L2.x-B2.x, L1, L2, Hit.GetPos()) && TempInBox( Hit, B1, B2, 1 ) )
		return TRUE;
	if ( TempGetIntersection( L1.y-B2.y, L2.y-B2.y, L1, L2, Hit.GetPos()) && TempInBox( Hit, B1, B2, 2 ) )
		return TRUE;
	if ( TempGetIntersection( L1.z-B2.z, L2.z-B2.z, L1, L2, Hit.GetPos()) && TempInBox( Hit, B1, B2, 3 ) )
		return TRUE;

	return FALSE;
}

float TLMaths::TBox::GetDistanceSq(const TLine& Line) const
{
	/*
	int inline GetIntersection( float fDst1, float fDst2, CVec3 P1, CVec3 P2, CVec3 &Hit) {
if ( (fDst1 * fDst2) >= 0.0f) return 0;
if ( fDst1 == fDst2) return 0; 
Hit = P1 + (P2-P1) * ( -fDst1/(fDst2-fDst1) );
return 1;
}

int inline InBox( CVec3 Hit, CVec3 B1, CVec3 B2, const int Axis) {
if ( Axis==1 && Hit.z > B1.z && Hit.z < B2.z && Hit.y > B1.y && Hit.y < B2.y) return 1;
if ( Axis==2 && Hit.z > B1.z && Hit.z < B2.z && Hit.x > B1.x && Hit.x < B2.x) return 1;
if ( Axis==3 && Hit.x > B1.x && Hit.x < B2.x && Hit.y > B1.y && Hit.y < B2.y) return 1;
return 0;
}

// returns true if line (L1, L2) intersects with the box (B1, B2)
// returns intersection point in Hit
int CheckLineBox( CVec3 B1, CVec3 B2, CVec3 L1, CVec3 L2, CVec3 &Hit)
{
if (L2.x < B1.x && L1.x < B1.x) return false;
if (L2.x > B2.x && L1.x > B2.x) return false;
if (L2.y < B1.y && L1.y < B1.y) return false;
if (L2.y > B2.y && L1.y > B2.y) return false;
if (L2.z < B1.z && L1.z < B1.z) return false;
if (L2.z > B2.z && L1.z > B2.z) return false;
if (L1.x > B1.x && L1.x < B2.x &&
    L1.y > B1.y && L1.y < B2.y &&
    L1.z > B1.z && L1.z < B2.z) 
    {Hit = L1; 
    return true;}
if ( (GetIntersection( L1.x-B1.x, L2.x-B1.x, L1, L2, Hit) && InBox( Hit, B1, B2, 1 ))
  || (GetIntersection( L1.y-B1.y, L2.y-B1.y, L1, L2, Hit) && InBox( Hit, B1, B2, 2 )) 
  || (GetIntersection( L1.z-B1.z, L2.z-B1.z, L1, L2, Hit) && InBox( Hit, B1, B2, 3 )) 
  || (GetIntersection( L1.x-B2.x, L2.x-B2.x, L1, L2, Hit) && InBox( Hit, B1, B2, 1 )) 
  || (GetIntersection( L1.y-B2.y, L2.y-B2.y, L1, L2, Hit) && InBox( Hit, B1, B2, 2 )) 
  || (GetIntersection( L1.z-B2.z, L2.z-B2.z, L1, L2, Hit) && InBox( Hit, B1, B2, 3 )))
	return true;

return false;
}
*/
	TLDebug_Break("GR: todo: check line intersection");
	return 1.f;	//	+VE = outside
}


Bool TLMaths::TBox::GetIntersection(const TLMaths::TBox& Box) const
{
	TLDebug_Break("Todo: Box<->Box intersection test");
	return FALSE;
}


Bool TLMaths::TBox::GetIntersection(const float3& Pos) const
{
	if ( Pos.x < m_Min.x || Pos.x > m_Max.x )
		return FALSE;
	
	if ( Pos.y < m_Min.y || Pos.y > m_Max.y )
		return FALSE;

	if ( Pos.z < m_Min.z || Pos.z > m_Max.z )
		return FALSE;

	return TRUE;
}


float TLMaths::TBox::GetDistanceSq(const float3& Pos) const
{
	//	accumulate distance to see how far away we are
	float3 Distance;
	float3 Center = GetCenter();
	Type3<Bool> Inside( FALSE, FALSE, FALSE );

	if ( Pos.x < m_Min.x )	
	{
		Distance.x = m_Min.x - Pos.x;	//	get distance from min
	}
	else if ( Pos.x > m_Max.x )
	{
		Distance.x = Pos.x - m_Max.x;	//	get positive distance
	}
	else
	{
		Distance.x = Pos.x - Center.x;	//	doesnt matter if negative or not
		Inside.x = TRUE;
	}

	if ( Pos.y < m_Min.y )	
	{
		Distance.y = m_Min.y - Pos.y;	//	get positive distance
	}
	else if ( Pos.y > m_Max.y )
	{
		Distance.y = Pos.y - m_Max.y;	//	get positive distance
	}
	else
	{
		Distance.y = Pos.y - Center.y;	//	doesnt matter if negative or not
		Inside.y = TRUE;
	}

	if ( Pos.z < m_Min.z )	
	{
		Distance.z = m_Min.z - Pos.z;	//	get positive distance
	}
	else if ( Pos.z > m_Max.z )
	{
		Distance.z = Pos.z - m_Max.z;	//	get positive distance
	}
	else
	{
		Distance.z = Pos.z - Center.z;	//	doesnt matter if negative or not
		Inside.z = TRUE;
	}

	
	if ( !Inside.x || !Inside.y || !Inside.z )
	{
		//	return negative distance (as we're outside)
		return -Distance.LengthSq();
	}
	else
	{
		//	get internal distance from center (positive as we're inside)
		return Distance.LengthSq();
	}
}



void TLMaths::TBox::Accumulate(const TCapsule& Capsule)			
{
	TSphere Sphere = Capsule.GetSphere();	
	Accumulate( Sphere );	
}


Bool TLMaths::TBox::GetIntersection(const TSphere& Sphere) const	
{
	return Sphere.GetIntersection( *this );	
}




//-------------------------------------------------
//	get the center of the min/max
//-------------------------------------------------
float3 TLMaths::TBox2D::GetCenter3(float z) const	
{
	float2 Center( m_Max.x, m_Max.y );
	Center.x -= m_Min.x;
	Center.y -= m_Min.y;
	Center /= 2.f;
	Center.x += m_Min.x;
	Center.y += m_Min.y;

	return float3( Center.x, Center.y, z );
}



TLMaths::TBox2D::TBox2D() :
	m_IsValid	( FALSE )
{
}


TLMaths::TBox2D::TBox2D(const float2& Min,const float2& Max) : 
	m_Min		( Min ), 
	m_Max		( Max ),
	m_IsValid	( TRUE )
{
}


//-------------------------------------------------
//	grow the box to these extents
//-------------------------------------------------
void TLMaths::TBox2D::Accumulate(const float2& Point)
{
	//	if we're not valid, set box to the "size" of this point
	if ( !IsValid() )
	{
		Set( Point, Point );
		return;
	}

	if ( Point.x < m_Min.x )	m_Min.x = Point.x;
	if ( Point.y < m_Min.y )	m_Min.y = Point.y;

	if ( Point.x > m_Max.x )	m_Max.x = Point.x;
	if ( Point.y > m_Max.y )	m_Max.y = Point.y;
}


//-------------------------------------------------
//	get the extents of all these points
//-------------------------------------------------
void TLMaths::TBox2D::Accumulate(const TArray<float2>& Points)
{
	for ( u32 i=0;	i<Points.GetSize();	i++ )
	{
		Accumulate( Points[i] );
	}
}


//-------------------------------------------------
//	get the extents of all these points
//-------------------------------------------------
void TLMaths::TBox2D::Accumulate(const TArray<float3>& Points)
{
	for ( u32 i=0;	i<Points.GetSize();	i++ )
	{
		Accumulate( Points[i] );
	}
}




//--------------------------------------------------------
//	untransform box
//	note: must be opposite order to transform!
//--------------------------------------------------------
void TLMaths::TBox2D::Untransform(const TLMaths::TTransform& Transform)
{
	if ( Transform.HasTranslate() )
	{
		m_Min -= Transform.GetTranslate();
		m_Max -= Transform.GetTranslate();
	}

	if ( Transform.HasMatrix() )
	{
		TLDebug_Break("todo: undo transform box");
		//Transform.m_Matrix.UnTransformVector( m_Pos );
	}

	if ( Transform.HasScale() )
	{
		const float3& Scale = Transform.GetScale();
		m_Min /= Scale;
		m_Max /= Scale;
	}
}


//-------------------------------------------------
//	get the 4 corners of the box
//-------------------------------------------------
void TLMaths::TBox2D::GetBoxCorners(TArray<float2>& CornerPositions) const
{
	//	box top
	CornerPositions.Add( float2( m_Min.x,	m_Min.y ) );
	CornerPositions.Add( float2( m_Max.x,	m_Min.y ) );

	//	box bottom
	CornerPositions.Add( float2( m_Min.x,	m_Max.y ) );
	CornerPositions.Add( float2( m_Max.x,	m_Max.y ) );
}


//-------------------------------------------------
//	get the 4 corners of the box
//-------------------------------------------------
void TLMaths::TBox2D::GetBoxCorners(TArray<float3>& CornerPositions,float z) const
{
	//	box top
	CornerPositions.Add( float3( m_Min.x, m_Min.y, z ) );
	CornerPositions.Add( float3( m_Max.x, m_Min.y, z ) );

	//	box bottom
	CornerPositions.Add( float3( m_Max.x, m_Max.y, z ) );
	CornerPositions.Add( float3( m_Min.x, m_Max.y, z ) );
}



//---------------------------------------------------
//	Check to see if a line intersects another
//---------------------------------------------------
FORCEINLINE Bool LineIntersectLine(const float3& v2MinusV1, const float2& v1Minusv3,float v4Minusv3x,float v4Minusv3y)
{
    float denom =		((v4Minusv3y) * (v2MinusV1.x)) - ((v4Minusv3x) * (v2MinusV1.y));
    float numerator =	((v4Minusv3x) * (v1Minusv3.y)) - ((v4Minusv3y) * (v1Minusv3.x));
    float numerator2 =	((v2MinusV1.x) * (v1Minusv3.y)) - ((v2MinusV1.y) * (v1Minusv3.x));

    if ( denom == 0.0f )
    {
        if ( numerator == 0.0f && numerator2 == 0.0f )
        {
            return FALSE;//COINCIDENT;
        }
        return FALSE;// PARALLEL;
    }
    float ua = numerator / denom;
    float ub = numerator2/ denom;

    return (ua >= 0.0f && ua <= 1.0f && ub >= 0.0f && ub <= 1.0f);
}


//-------------------------------------------------
//	test to see if this line intersects our box
//-------------------------------------------------
Bool TLMaths::TBox2D::GetIntersection(const TLine& Line) const
{
	const float3& v1 = Line.GetStart();
	const float3& v2 = Line.GetEnd();

	//	early check if either point of the line is inside the box
	if ( GetIntersection( v1 ) )	return TRUE;
	if ( GetIntersection( v2 ) )	return TRUE;

	// check each line for intersection
	float3 v2MinusV1( v2 - v1 );
	float v1xMinusLeft = v1.x-m_Min.x;
	float v1yMinusTop = v1.y-m_Min.y;
	float BoxHeight = m_Max.y-m_Min.y;
	if (LineIntersectLine( v2MinusV1, float2( v1xMinusLeft, v1yMinusTop ),		0.f,		BoxHeight ) ) return TRUE;

	float v1yMinusBottom = v1.y-m_Max.y;
	float BoxWidth = m_Max.x-m_Min.x;
	if (LineIntersectLine( v2MinusV1, float2( v1xMinusLeft, v1yMinusBottom ),	BoxWidth,	0.f			) ) return TRUE;

	if (LineIntersectLine( v2MinusV1, float2( v1xMinusLeft, v1yMinusTop ),		BoxWidth,	0.f			) ) return TRUE;

	float v1xMinusRight = v1.x-m_Max.x;
	if (LineIntersectLine( v2MinusV1, float2( v1xMinusRight, v1yMinusTop ),		0.f,		BoxHeight ) ) return TRUE;
		
	return FALSE;
}

/*
//-------------------------------------------------
//	test to see if this line intersects our box
//-------------------------------------------------
Bool TLMaths::TBox2D::GetIntersection(const TCapsule2D& Capsule) const
{
	const float3& v1 = Capsule.GetLine().GetStart();
	const float3& v2 = Capsule.GetLine().GetEnd();

	//	early check if either point of the line is inside the box
	if ( GetIntersection( v1 ) )	return TRUE;
	if ( GetIntersection( v2 ) )	return TRUE;

	// check each line for intersection
	float3 v2MinusV1( v2 - v1 );
	float v1xMinusLeft = v1.x-m_Min.x;
	float v1yMinusTop = v1.y-m_Min.y;
	float BoxHeight = m_Max.y-m_Min.y;
	if (LineIntersectLine( v2MinusV1, float2( v1xMinusLeft, v1yMinusTop ),		0.f,		BoxHeight ) ) return TRUE;

	float v1yMinusBottom = v1.y-m_Max.y;
	float BoxWidth = m_Max.x-m_Min.x;
	if (LineIntersectLine( v2MinusV1, float2( v1xMinusLeft, v1yMinusBottom ),	BoxWidth,	0.f			) ) return TRUE;

	if (LineIntersectLine( v2MinusV1, float2( v1xMinusLeft, v1yMinusTop ),		BoxWidth,	0.f			) ) return TRUE;

	float v1xMinusRight = v1.x-m_Max.x;
	if (LineIntersectLine( v2MinusV1, float2( v1xMinusRight, v1yMinusTop ),		0.f,		BoxHeight ) ) return TRUE;
		
	return FALSE;
}
*/

//-------------------------------------------------
//	test to see if this line intersects our box
//-------------------------------------------------
Bool TLMaths::TBox2D::GetIntersection(const TLMaths::TBox& Box) const
{
	const float2& Min1 = m_Min;
	const float2& Max1 = m_Max;
	const float3& Min2 = Box.GetMin();
	const float3& Max2 = Box.GetMax();

	if ( Max2.x < Min1.x )	return FALSE;
	if ( Max1.x < Min2.x )	return FALSE;

	if ( Max2.y < Min1.y )	return FALSE;
	if ( Max1.y < Min2.y )	return FALSE;
	
	return TRUE;
}


//-------------------------------------------------
//	test to see if this line intersects our box
//-------------------------------------------------
Bool TLMaths::TBox2D::GetIntersection(const TLMaths::TBox2D& Box) const
{
	const float2& Min1 = m_Min;
	const float2& Max1 = m_Max;
	const float3& Min2 = Box.GetMin();
	const float3& Max2 = Box.GetMax();

	if ( Max2.x < Min1.x )	return FALSE;
	if ( Max1.x < Min2.x )	return FALSE;

	if ( Max2.y < Min1.y )	return FALSE;
	if ( Max1.y < Min2.y )	return FALSE;
	
	return TRUE;
}


Bool TLMaths::TBox2D::GetIntersection(const float3& Pos) const
{
	if ( Pos.x < m_Min.x || Pos.x > m_Max.x )
		return FALSE;
	
	if ( Pos.y < m_Min.y || Pos.y > m_Max.y )
		return FALSE;

	return TRUE;
}

Bool TLMaths::TBox2D::GetIntersection(const float2& Pos) const
{
	if ( Pos.x < m_Min.x || Pos.x > m_Max.x )
		return FALSE;
	
	if ( Pos.y < m_Min.y || Pos.y > m_Max.y )
		return FALSE;

	return TRUE;
}


Bool TLMaths::TBox2D::GetIntersection(const TLMaths::TSphere2D& Sphere) const
{
	if ( !this->IsValid() || !Sphere.IsValid() )
	{
		TLDebug_Break("Distance test between invalid shapes");
		return FALSE;
	}

	float dmin = 0.f;
	const float2& SpherePos = Sphere.GetPos();
	float SphereRadSq = Sphere.GetRadiusSq();

//	float2 PosMinusMin( SpherePos.x - m_Min.x, SpherePos.y - m_Min.y );
//	float2 PosMinusMax( SpherePos.x - m_Max.x, SpherePos.y - m_Max.y );

	if( SpherePos.x < m_Min.x )
	{
		float PosMinusMinX = SpherePos.x - m_Min.x;
		dmin += PosMinusMinX * PosMinusMinX;
	}
	else if ( SpherePos.x > m_Max.x ) 
	{
		float PosMinusMaxX = SpherePos.x - m_Max.x;
		dmin += PosMinusMaxX * PosMinusMaxX;
	}

	if ( dmin > SphereRadSq ) 
		return FALSE;

	if( SpherePos.y < m_Min.y ) 
	{
		float PosMinusMinY = SpherePos.y - m_Min.y;
		dmin += PosMinusMinY * PosMinusMinY;
	}
	else if ( SpherePos.y > m_Max.y ) 
	{
		float PosMinusMaxY = SpherePos.y - m_Max.y;
		dmin += PosMinusMaxY * PosMinusMaxY;
	}

	if ( dmin > SphereRadSq ) 
		return FALSE;

	return TRUE;
}


Bool TLMaths::TBox2D::GetIntersection(const TLMaths::TSphere& Sphere) const
{
	if ( !this->IsValid() || !Sphere.IsValid() )
	{
		TLDebug_Break("Distance test between invalid shapes");
		return FALSE;
	}

	float dmin = 0.f;
	const float3& SpherePos = Sphere.GetPos();
	float SphereRadSq = Sphere.GetRadiusSq();

//	float2 PosMinusMin( SpherePos.x - m_Min.x, SpherePos.y - m_Min.y );
//	float2 PosMinusMax( SpherePos.x - m_Max.x, SpherePos.y - m_Max.y );

	if( SpherePos.x < m_Min.x )
	{
		float PosMinusMinX = SpherePos.x - m_Min.x;
		dmin += PosMinusMinX * PosMinusMinX;
	}
	else if ( SpherePos.x > m_Max.x ) 
	{
		float PosMinusMaxX = SpherePos.x - m_Max.x;
		dmin += PosMinusMaxX * PosMinusMaxX;
	}

	if ( dmin > SphereRadSq ) 
		return FALSE;

	if( SpherePos.y < m_Min.y ) 
	{
		float PosMinusMinY = SpherePos.y - m_Min.y;
		dmin += PosMinusMinY * PosMinusMinY;
	}
	else if ( SpherePos.y > m_Max.y ) 
	{
		float PosMinusMaxY = SpherePos.y - m_Max.y;
		dmin += PosMinusMaxY * PosMinusMaxY;
	}

	if ( dmin > SphereRadSq ) 
		return FALSE;

	return TRUE;
}



Bool TempInBox(const TLMaths::TSphere2D& HitSphere,const float2& Min, const float2& Max, const u32 Axis) 
{
	const float& Rad = HitSphere.GetRadius();
	const float2& Hit = HitSphere.GetPos();

	if ( Axis==1 && Hit.y >= Min.y-Rad && Hit.y <= Max.y+Rad)
		return TRUE;
	if ( Axis==2 &&  Hit.x >= Min.x-Rad && Hit.x <= Max.x+Rad) 
		return TRUE;
	return FALSE;
}


//-------------------------------------------------
//	test to see if this line intersects our box
//-------------------------------------------------
Bool TLMaths::TBox2D::GetIntersection(const TCapsule2D& Capsule) const
{
	const TLMaths::TLine2D& CapsuleLine = Capsule.GetLine();

	//	quick is-inside test
	if ( GetIntersection( CapsuleLine.GetStart() ) || GetIntersection( CapsuleLine.GetEnd() ) )
		return TRUE;

	//	do our "last chance" inside box test first as it's quite a good general test...
	float2 BoxCenter = GetCenter();
	float2 NearestPoint = CapsuleLine.GetNearestPoint( BoxCenter );
	if ( (NearestPoint - BoxCenter).LengthSq() <= Capsule.GetRadiusSq() )
		return TRUE;

	//	do line intersection tests
	float2 TopLeft(		m_Min.x, m_Min.y );
	float2 TopRight(	m_Max.x, m_Min.y );
	float2 BottomRight(	m_Max.x, m_Max.y );
	float2 BottomLeft(	m_Min.x, m_Max.y );

	float EdgeDistSq;

	EdgeDistSq = CapsuleLine.GetDistanceSq( TLMaths::TLine2D( TopLeft, TopRight ) );
	if ( EdgeDistSq <= Capsule.GetRadiusSq() )
		return TRUE;

	EdgeDistSq = CapsuleLine.GetDistanceSq( TLMaths::TLine2D( TopRight, BottomRight ) );
	if ( EdgeDistSq <= Capsule.GetRadiusSq() )
		return TRUE;

	EdgeDistSq = CapsuleLine.GetDistanceSq( TLMaths::TLine2D( BottomRight, BottomLeft ) );
	if ( EdgeDistSq <= Capsule.GetRadiusSq() )
		return TRUE;

	EdgeDistSq = CapsuleLine.GetDistanceSq( TLMaths::TLine2D( BottomLeft, TopLeft ) );
	if ( EdgeDistSq <= Capsule.GetRadiusSq() )
		return TRUE;

	return FALSE;
}


//-------------------------------------------------
//	grow/shrink this box around its center
//-------------------------------------------------
void TLMaths::TBox2D::GrowBox(float Scale)
{
	//	get current center
	float2 OldCenter = GetCenter();

	//	center box around 0,0
	m_Min -= OldCenter;
	m_Max -= OldCenter;

	//	scale down
	m_Min *= Scale;
	m_Max *= Scale;

	//	move back to old center
	m_Min += OldCenter;
	m_Max += OldCenter;
}


//-------------------------------------------------
//	merge two boxes, box will only get bigger
//-------------------------------------------------
void TLMaths::TBox2D::Accumulate(const TBox2D& Box)			
{
	//	source box is not valid!
	if ( !Box.IsValid() )
	{
		if ( !TLDebug_Break("Accumulating invalid box") )
			return;
	}

	//	if we're not valid, just copy the box
	if ( !IsValid() )
	{
		Set( Box.GetMin(), Box.GetMax() );
	}
	else
	{
		Accumulate( Box.GetMin() );	
		Accumulate( Box.GetMax() );	
	}
}	



//-------------------------------------------------
//	transform box - this transforms each corner, then gets
//	a new/min max and hence a new box
//-------------------------------------------------
void TLMaths::TBox2D::Transform(const TLMaths::TTransform& Transform)
{
	//	not valid, or not going to make any changes... skip
	if ( !IsValid() || !Transform.HasAnyTransform() )
		return;

	//	gr: I'm pretty sure transforming the min & max wont work quite right, so transform each point
	//		and make a new bounds box
	TFixedArray<float2,4> BoundsPoints(0);
	GetBoxCorners( BoundsPoints );

	//	transform the points
	for ( u32 i=0;	i<BoundsPoints.GetSize();	i++ )
	{
		Transform.TransformVector( BoundsPoints[i] );
	}

	//	make new bounds
	SetInvalid();
	Accumulate( BoundsPoints );
}
