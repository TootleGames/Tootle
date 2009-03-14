#include "TSphere.h"
#include "TCapsule.h"
#include "TBox.h"
#include <TootleCore/TString.h>


TLMaths::TSphere::TSphere() :
	m_Pos				( 0.f, 0.f, 0.f ),
	m_Radius			( -1.f )	//	invalid
{
}


TLMaths::TSphere::TSphere(const float3& Pos,float Radius) :
	m_Pos				( Pos ),
	m_Radius			( Radius )
{
}


//-----------------------------------------------------------
//	accumulate other capsule. copies other if this is invalid
//-----------------------------------------------------------
void TLMaths::TSphere::Accumulate(const TCapsule& Capsule)
{
	if ( !IsValid() )
	{
		Set( Capsule.GetSphere() );
		return;
	}

	TLDebug_Break("todo: get two furthest points on capsule then accumulate both");
}


//-----------------------------------------------------------
//	grow the box to these extents
//-----------------------------------------------------------
void TLMaths::TSphere::Accumulate(const TSphere& Sphere)
{
	if ( !IsValid() )
	{
		Set( Sphere );
		return;
	}

	//	get furthest point on both spheres from each other
	float3 DirToSphere( Sphere.GetPos() - GetPos() );
	DirToSphere.Normalise();
	float3 FurthestPointOnSphere = Sphere.GetPos() + DirToSphere.Normal( Sphere.GetRadius() );
	float3 FurthestPointOnThis = GetPos() - DirToSphere.Normal( GetRadius() );

	//	new sphere center is midpoint between furthest points
	m_Pos = (FurthestPointOnSphere + FurthestPointOnThis) * 0.5f;

	//	new radius is half length from furthest point to furthest point
	m_Radius = (FurthestPointOnSphere - FurthestPointOnThis).Length() * 0.5f;
}



//-----------------------------------------------------------
//	grow the box to these extents
//-----------------------------------------------------------
void TLMaths::TSphere::Accumulate(const TBox& Box)
{
	Accumulate( Box.GetMin() );
	Accumulate( Box.GetMax() );
}


//-----------------------------------------------------------
//	grow the box to these extents
//-----------------------------------------------------------
void TLMaths::TSphere::Accumulate(const TBox2D& Box)
{
	Accumulate( Box.GetMin() );
	Accumulate( Box.GetMax() );
}

//-----------------------------------------------------------
//	grow the box to these extents
//-----------------------------------------------------------
void TLMaths::TSphere::Accumulate(const float3& Point)
{
	if ( !IsValid() )
	{
		Set( Point, 0.f );
		return;
	}

	//	get the furthest point on the sphere away from the point
	float3 FurthestPoint( GetPos() );
	if ( m_Radius > 0.f )
	{
		float3 DirToSphere( GetPos() - Point );
		//DirToSphere.Normalise( m_Radius );
		DirToSphere.Normalise();
		FurthestPoint += DirToSphere * m_Radius;
	}

	//	new sphere center is midpoint between furthest point and point
	m_Pos = (Point + FurthestPoint) * 0.5f;

	//	new radius is length from new pos to either point or furthest point
	m_Radius = (m_Pos - FurthestPoint).Length();
}


//-----------------------------------------------------------
//	get the extents of all these points
//-----------------------------------------------------------
void TLMaths::TSphere::Accumulate(const TArray<float3>& Points)
{
	if ( Points.GetSize() == 0 )
		return;

	if ( Points.GetSize() == 1 )
	{
		Accumulate( Points[0] );
		return;
	}

	//	gr: best result is getting a box from these points then accumulating that
	TBox PointsBox;
	PointsBox.Accumulate( Points );

	Accumulate( PointsBox );

	/*
	//	gr: new, slightly slower, but much more accuarate version
	
	//FIRST PASS: find 4 minima/maxima points
	float2 xmin = new float2(1e+10, 0);
	float2 ymin = new float2(0, 1e+10);
	float2 xmax = new float2(-1e+10, 0);
	float2 ymax = new float2(0, -1e+10);
	
	var i:uint, k:uint = Points.GetSize();	
	for (i = 0; i < k; i++)
	{
		p = points[i];
		if (p.x < xmin.x)
			xmin = p; //New xminimum point
		if (p.x > xmax.x)
			xmax = p;
		if (p.y < ymin.y)
			ymin = p;
		if (p.y > ymax.y)
			ymax = p;
	}
	//Set xspan = distance between the 2 points xmin & xmax (squared)
	var dx:int = xmax.x - xmin.x;
	var dy:int = xmax.y - xmin.y;
	var xspan:int = dx * dx + dy * dy;
	
	//same for y span
	dx = ymax.x - ymin.x;
	dy = ymax.y - ymin.y;
	var yspan:int = dx * dx + dy * dy;
	
	//Set points dia1 & dia2 to the maximally separated pair
	var dia1:Point = xmin; //assume xspan biggest
	var dia2:Point = xmax;
	var maxspan:Number = xspan;
	
	if (yspan > maxspan)
	{
		maxspan = yspan;
		dia1 = ymin;
		dia2 = ymax;
	}
	
	//dia1,dia2 is a diameter of initial circle
	//calc initial center
	var cx:int = (dia1.x + dia2.x) >> 1;
	var cy:int = (dia1.y + dia2.y) >> 1;
	
	//calculate initial radius**2 and radius 
	dx = dia2.x - cx; //x component of radius vector
	dy = dia2.y - cy; //y component of radius vector
	
	var rSq:Number = dx * dx + dy * dy;
	var r:Number = Math.sqrt(rSq);
	
	var t0:Number, t1:Number;
	
	var dSq:Number, d0:Number, d1:Number;
	
	//SECOND PASS: increment current circle 
	for (i = 0; i < k; i++)
	{	//this point is outside of current circle
		p = points[i];
		
		dx = p.x - cx;
		dy = p.y - cy;
		dSq = dx * dx + dy * dy;
		if (dSq > rSq) //do r**2 test first
		{ 
			d0 = Math.sqrt(dSq);
			
			//calc radius of new circle
			r = (r + d0) >> 1;
			rSq = r * r;  //for next r**2 compare
			d1 = d0 - r;
			
			// calc center of new sphere
			cx = (r * cx + d1 * p.x) / d0;
			cy = (r * cy + d1 * p.y) / d0;
		}
	}
	
	return new Circle(cx, cy, r);
	*/
}


Bool TLMaths::TSphere::GetIntersection(const TLMaths::TBox& Box) const
{
	if ( !this->IsValid() || !Box.IsValid() )
	{
		TLDebug_Break("Distance test between invalid shapes");
		return FALSE;
	}

	float dmin = 0.f;
	float3 PosMinusMin = m_Pos - Box.GetMin();
	float3 PosMinusMax = m_Pos - Box.GetMax();

	if( m_Pos.x < Box.GetMin().x ) 
		dmin += PosMinusMin.x * PosMinusMin.x;
	else if ( m_Pos.x > Box.GetMax().x ) 
		dmin += PosMinusMax.x * PosMinusMax.x;

	if ( dmin > m_Radius*m_Radius ) 
		return FALSE;

	if( m_Pos.y < Box.GetMin().y ) 
		dmin += PosMinusMin.y * PosMinusMin.y;
	else if ( m_Pos.y > Box.GetMax().y ) 
		dmin += PosMinusMax.y * PosMinusMax.y;

	if ( dmin > m_Radius*m_Radius ) 
		return FALSE;

	if( m_Pos.z < Box.GetMin().z ) 
		dmin += PosMinusMin.z * PosMinusMin.z;
	else if ( m_Pos.z > Box.GetMax().z ) 
		dmin += PosMinusMax.z * PosMinusMax.z;

	if ( dmin > m_Radius*m_Radius ) 
		return FALSE;

	return TRUE;
}


float TLMaths::TSphere::GetDistanceSq(const TLMaths::TBox& Box) const
{
	if ( !this->IsValid() || !Box.IsValid() )
	{
		TLDebug_Break("Distance test between invalid shapes");
		return 0.f;
	}

	//	get a line from the sphere to the center then do a line intersection test
	float3 BoxCenter = Box.GetCenter();
	float3 DirToBox = (BoxCenter - m_Pos);
	DirToBox.Normalise( m_Radius );
	TLine SphereLine( m_Pos, m_Pos + DirToBox );

	return Box.GetDistanceSq( SphereLine );
}



//--------------------------------------------------------
//	untransform sphere
//	note: must be opposite order to transform!
//--------------------------------------------------------
void TLMaths::TSphere::Untransform(const TLMaths::TTransform& Transform)
{
	if ( Transform.HasTranslate() )
	{
		m_Pos -= Transform.GetTranslate();
	}

	if ( Transform.HasMatrix() )
	{
		TLDebug_Break("todo: undo transform sphere");
		//Transform.m_Matrix.UnTransformVector( m_Pos );
	}

	if ( Transform.HasScale() )
	{
		const float3& Scale = Transform.GetScale();
		m_Pos /= Scale;

		//	scale radius by biggest value of the scale
		float BigScale = (Scale.x>Scale.y) ? Scale.x : Scale.y;
		BigScale = (Scale.z>BigScale) ? Scale.z : BigScale;

		m_Radius /= BigScale;
	}
}


//---------------------------------------------------------
//	
//---------------------------------------------------------
float TLMaths::TSphere::GetDistanceSq(const TCapsule& Capsule) const
{
	if ( !this->IsValid() || !Capsule.IsValid() )
	{
		TLDebug_Break("Distance test between invalid shapes");
		return 0.f;
	}

	//	gr: this hasn't been tested, just assuming it's right...
	float3 NearPosOnCapsule = Capsule.GetPosNearestTo( GetPos() );

	//	now treat as a sphere...

	//	get the vector between the spheres
	float3 Diff( GetPos() - NearPosOnCapsule );

	//	get the extents of the spheres (squared)
	float SphereRadSq = GetRadius() + Capsule.GetRadius();
	SphereRadSq *= SphereRadSq;

	//	return the gap
	return Diff.LengthSq() - SphereRadSq;
}


//--------------------------------------------------------
//	get ray intersection
//--------------------------------------------------------
Bool TLMaths::TSphere::GetIntersection(const TLine& Line) const
{
	float DistanceSq = Line.GetDistanceSq( m_Pos );

	if ( DistanceSq > GetRadiusSq() )
		return FALSE;

	return TRUE;
}


//--------------------------------------------------------
//	transform this shape by this matrix
//--------------------------------------------------------
void TLMaths::TSphere::Transform(const TLMaths::TMatrix& Matrix,const float3& Scale)
{
	if ( !IsValid() )
		return;

	//	transform center of sphere
	m_Pos *= Scale;
	Matrix.TransformVector( m_Pos );

	//	scale radius of sphere
	float3 SphereRadius3 = Scale * GetRadius();
	
	//	new radius is biggest of 3 axis' radius
	m_Radius = SphereRadius3.x;
	if ( SphereRadius3.y > m_Radius )	m_Radius = SphereRadius3.y;
	if ( SphereRadius3.z > m_Radius )	m_Radius = SphereRadius3.z;
}

	
//--------------------------------------------------------
//	transform sphere
//--------------------------------------------------------
void TLMaths::TSphere::Transform(const TLMaths::TTransform& Transform)
{
	if ( !IsValid() )
	{
		TLDebug_Break("Transforming invalid capsule");
		return;
	}

	if ( Transform.HasScale() )
	{
		const float3& Scale = Transform.GetScale();
		m_Pos *= Scale;

		//	scale radius by biggest value of the scale
		float BigScale = (Scale.x>Scale.y) ? Scale.x : Scale.y;
		BigScale = (Scale.z>BigScale) ? Scale.z : BigScale;

		m_Radius *= BigScale;
	}

	if ( Transform.HasRotation() )
	{
		Transform.GetRotation().RotateVector( m_Pos );
	}

	if ( Transform.HasMatrix() )
	{
		Transform.GetMatrix().TransformVector( m_Pos );
	}

	if ( Transform.HasTranslate() )
	{
		m_Pos += Transform.GetTranslate();
	}
}


Bool TLMaths::TSphere::GetIntersection(const TSphere& Sphere) const	
{
	if ( !this->IsValid() || !Sphere.IsValid() )
	{
		TLDebug_Break("Distance test between invalid shapes");
		return FALSE;
	}

	float SphereTotalRad = GetRadius() + Sphere.GetRadius();

	//	get the vector between the spheres
	float3 Diff( GetPos() - Sphere.GetPos() );
	if ( Diff.x > SphereTotalRad )	return FALSE;
	if ( Diff.y > SphereTotalRad )	return FALSE;
	if ( Diff.z > SphereTotalRad )	return FALSE;

	//	return if diff sq is less than total rad sq
	return Diff.LengthSq() < (SphereTotalRad*SphereTotalRad);
}


//---------------------------------------------------------
//	
//---------------------------------------------------------
float TLMaths::TSphere::GetDistanceSq(const TSphere& Sphere) const
{
	if ( !this->IsValid() || !Sphere.IsValid() )
	{
		TLDebug_Break("Distance test between invalid shapes");
		return 0.f;
	}

	//	get the vector between the spheres
	float3 Diff( GetPos() - Sphere.GetPos() );

	//	get the extents of the spheres (squared)
	float SphereRadSq = GetRadius() + Sphere.GetRadius();
	SphereRadSq *= SphereRadSq;

//	TLDebug_Print( TString("Dist from sphere to sphere: %3.f", Diff.LengthSq() - SphereRadSq ) );

	//	return the gap
	return Diff.LengthSq() - SphereRadSq;
}

//---------------------------------------------------------
//	
//---------------------------------------------------------
float TLMaths::TSphere::GetDistanceSq(const float3& Pos) const
{
	if ( !this->IsValid() )
	{
		TLDebug_Break("Distance test between invalid shapes");
		return 0.f;
	}

	//	get the vector between the points
	float3 Diff( GetPos() - Pos );

	//	get the extents of the spheres (squared)
	float SphereRadSq = GetRadius();
	SphereRadSq *= SphereRadSq;

	//	return the gap
	return Diff.LengthSq() - SphereRadSq;
}



TLMaths::TSphere2D::TSphere2D() :
	m_Pos				( 0.f, 0.f ),
	m_Radius			( -1.f )	//	invalid
{
}


TLMaths::TSphere2D::TSphere2D(const float2& Pos,float Radius) :
	m_Pos				( Pos ),
	m_Radius			( Radius )
{
}


//-----------------------------------------------------------
//	grow the box to these extents
//-----------------------------------------------------------
void TLMaths::TSphere2D::Accumulate(const TBox2D& Box)
{
	Accumulate( Box.GetMin() );
	Accumulate( Box.GetMax() );
}

//-----------------------------------------------------------
//	grow the box to these extents
//-----------------------------------------------------------
void TLMaths::TSphere2D::Accumulate(const float2& Point)
{
	if ( !IsValid() )
	{
		Set( Point, 0.f );
		return;
	}

	//	get the furthest point on the sphere away from the point
	float2 FurthestPoint( GetPos() );
	if ( m_Radius > 0.f )
	{
		float2 DirToSphere( GetPos() - Point );
		//DirToSphere.Normalise( m_Radius );
		DirToSphere.Normalise();
		FurthestPoint += DirToSphere * m_Radius;
	}

	//	new sphere center is midpoint between furthest point and point
	m_Pos = (Point + FurthestPoint) * 0.5f;

	//	new radius is length from new pos to either point or furthest point
	m_Radius = (m_Pos - FurthestPoint).Length();
}


//-----------------------------------------------------------
//	get the extents of all these points
//-----------------------------------------------------------
void TLMaths::TSphere2D::Accumulate(const TArray<float2>& Points)
{
	if ( Points.GetSize() == 0 )
		return;

	if ( Points.GetSize() == 1 )
	{
		Accumulate( Points[0] );
		return;
	}

	//	gr: best result is getting a box from these points then accumulating that
	TBox2D PointsBox;
	PointsBox.Accumulate( Points );

	Accumulate( PointsBox );
}


//-----------------------------------------------------------
//	get the extents of all these points
//-----------------------------------------------------------
void TLMaths::TSphere2D::Accumulate(const TArray<float3>& Points)
{
	if ( Points.GetSize() == 0 )
		return;

	if ( Points.GetSize() == 1 )
	{
		Accumulate( Points[0] );
		return;
	}

	//	gr: best result is getting a box from these points then accumulating that
	TBox2D PointsBox;
	PointsBox.Accumulate( Points );

	Accumulate( PointsBox );
}


//-----------------------------------------------------------
//	
//-----------------------------------------------------------
Bool TLMaths::TSphere2D::GetIntersection(const TLMaths::TSphere2D& Sphere) const	
{
	if ( !this->IsValid() || !Sphere.IsValid() )
	{
		TLDebug_Break("Distance test between invalid shapes");
		return FALSE;
	}

	float SphereTotalRad = GetRadius() + Sphere.GetRadius();

	//	get the vector between the spheres
	float2 Diff( GetPos() - Sphere.GetPos() );
	if ( Diff.x > SphereTotalRad )	return FALSE;
	if ( Diff.y > SphereTotalRad )	return FALSE;

	//	return if diff sq is less than total rad sq
	return Diff.LengthSq() < (SphereTotalRad*SphereTotalRad);
}



//-----------------------------------------------------------
//	
//-----------------------------------------------------------
Bool TLMaths::TSphere2D::GetIntersection(const float2& Pos) const	
{
	if ( !this->IsValid() )
	{
		TLDebug_Break("Distance test between invalid shapes");
		return FALSE;
	}

	float SphereRad = GetRadius();

	//	get the vector between the spheres
	float2 Diff( GetPos() - Pos );
	if ( Diff.x > SphereRad )	return FALSE;
	if ( Diff.y > SphereRad )	return FALSE;

	//	return if diff sq is less than total rad sq
	return Diff.LengthSq() < (SphereRad*SphereRad);
}


//-----------------------------------------------------------
//	
//-----------------------------------------------------------
Bool TLMaths::TSphere2D::GetIntersection(const TLMaths::TLine2D& Line) const
{
	float2 NearestPointOnLine = Line.GetNearestPoint( GetPos() );

	float DistSq = ( NearestPointOnLine - GetPos() ).LengthSq();

	return (DistSq <= GetRadiusSq());
}


//-----------------------------------------------------------
//	use the box's extents to create the biggest sphere that will fit in the box
//-----------------------------------------------------------
void TLMaths::TSphere2D::Set(const TBox2D& Box)
{
	//	set center to center of the box
	m_Pos = Box.GetCenter();

	//	use the smallest extent as the radius
	float2 BoxSize = Box.GetSize();
	m_Radius = BoxSize.x < BoxSize.y ? BoxSize.x : BoxSize.y;

	//	half the radius (box extent is diameter)
	m_Radius *= 0.5f;
}




//-----------------------------------------------------------
//	grow the box to these extents
//-----------------------------------------------------------
void TLMaths::TSphere2D::Accumulate(const TSphere2D& Sphere)
{
	if ( !IsValid() )
	{
		Set( Sphere );
		return;
	}

	//	get furthest point on both spheres from each other
	float2 DirToSphere( Sphere.GetPos() - GetPos() );
	DirToSphere.Normalise();
	float2 FurthestPointOnSphere = Sphere.GetPos() + DirToSphere.Normal( Sphere.GetRadius() );
	float2 FurthestPointOnThis = GetPos() - DirToSphere.Normal( GetRadius() );

	//	new sphere center is midpoint between furthest points
	m_Pos = (FurthestPointOnSphere + FurthestPointOnThis) * 0.5f;

	//	new radius is half length from furthest point to furthest point
	m_Radius = (FurthestPointOnSphere - FurthestPointOnThis).Length() * 0.5f;
}


//-----------------------------------------------------------
//	grow the box to these extents
//-----------------------------------------------------------
void TLMaths::TSphere2D::Accumulate(const TSphere& Sphere)
{
	if ( !IsValid() )
	{
		Set( Sphere );
		return;
	}

	//	get furthest point on both spheres from each other
	float2 DirToSphere( Sphere.GetPos().xy() - GetPos() );
	DirToSphere.Normalise();
	float2 FurthestPointOnSphere = Sphere.GetPos().xy() + DirToSphere.Normal( Sphere.GetRadius() );
	float2 FurthestPointOnThis = GetPos() - DirToSphere.Normal( GetRadius() );

	//	new sphere center is midpoint between furthest points
	m_Pos = (FurthestPointOnSphere + FurthestPointOnThis) * 0.5f;

	//	new radius is half length from furthest point to furthest point
	m_Radius = (FurthestPointOnSphere - FurthestPointOnThis).Length() * 0.5f;
}



//--------------------------------------------------------
//	transform sphere
//--------------------------------------------------------
void TLMaths::TSphere2D::Transform(const TLMaths::TTransform& Transform)
{
	if ( !IsValid() )
	{
		TLDebug_Break("Transforming invalid capsule");
		return;
	}

	if ( Transform.HasScale() )
	{
		const float2& Scale = Transform.GetScale().xy();
		m_Pos *= Scale;

		//	scale radius by biggest value of the scale
		float BigScale = (Scale.x>Scale.y) ? Scale.x : Scale.y;

		m_Radius *= BigScale;
	}

	if ( Transform.HasRotation() )
	{
		Transform.GetRotation().RotateVector( m_Pos );
	}

	if ( Transform.HasMatrix() )
	{
		Transform.GetMatrix().TransformVector( m_Pos );
	}

	if ( Transform.HasTranslate() )
	{
		m_Pos += Transform.GetTranslate();
	}
}

