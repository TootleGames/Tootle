/*------------------------------------------------------
	
	3D and 2D sphere shapes (yeah, 2D sphere is a circle....
	but I'm sticking to a naming convention :)

-------------------------------------------------------*/
#pragma once
#include <TootleCore/TLMaths.h>
#include <TootleCore/TLTypes.h>
#include <TootleCore/TArray.h>

namespace TLMaths
{
	class TSphere;		//	3D sphere shape
	class TSphere2D;	//	2D sphere shape(circle)

	class TBox;
	class TBox2D;
	class TCapsule;
	class TLine2D;
};




//---------------------------------------------------------
//	sphere shape
//---------------------------------------------------------
class TLMaths::TSphere
{
public:
	TSphere();
	TSphere(const float3& Pos,float Radius);

	static TRef		GetTypeRef()								{	return "Sph";	}

	void			Set(const float3& Pos,float Radius)			{	m_Pos = Pos;	m_Radius = Radius;	}
	void			Set(const TSphere& Sphere)					{	m_Pos = Sphere.GetPos();	m_Radius = Sphere.GetRadius();	}
	void			SetInvalid()								{	m_Radius = -1.f;	}
	Bool			IsValid() const								{	return m_Radius >= 0.f;	}

	float3&			GetPos()									{	return m_Pos;	}
	const float3&	GetPos() const								{	return m_Pos;	}
	const float3&	GetPosNearestTo(const float3& Pos) const	{	return GetPos();	}
	float&			GetRadius() 								{	return m_Radius;	}
	const float&	GetRadius() const							{	return m_Radius;	}
	float			GetRadiusSq() const							{	return m_Radius*m_Radius;	}
	void			SetRadius(float Radius)						{	m_Radius = Radius;	}

	void			Accumulate(const TCapsule& Capsule);			//	accumulate other capsule. copies other if this is invalid
	void			Accumulate(const TSphere& Sphere);				//	grow the sphere to these extents
	void			Accumulate(const TBox& Box);					//	grow the sphere to these extents
	void			Accumulate(const TBox2D& Box);					//	grow the sphere to these extents
	void			Accumulate(const float3& Point);				//	grow the sphere to these extents
	void			Accumulate(const float2& Point,float z=0.f)		{	return Accumulate( float3( Point, z ) );	}
	void			Accumulate(const TArray<float3>& Points);		//	get the extents of all these points

	void			Transform(const TLMaths::TMatrix& Matrix,const float3& Scale);	//	transform this shape by this matrix
	void			Transform(const float3& Move)					{	m_Pos += Move;	}
	void			Transform(const TLMaths::TTransform& Transform);	//	transform sphere
	void			Untransform(const TLMaths::TTransform& Transform);	//	untransform sphere

	//	"intersection" is just a bool version of the distance check. (negative distance is an intersection)
	Bool			GetIntersection(const TCapsule& Capsule) const	{	return GetDistanceSq( Capsule ) < 0.f;	}
	Bool			GetIntersection(const TSphere& Sphere) const;
	Bool			GetIntersection(const float3& Pos) const		{	return GetDistanceSq( Pos ) < 0.f;	}
	Bool			GetIntersection(const TBox& Box) const;
	Bool			GetIntersection(const TLine& Line) const;		//	get ray intersection

	//	if a distance returns negative then it's overlapping by that amount - otherwise it's the distance from the edge of each shape
	float			GetDistance(const TCapsule& Capsule) const		{	return TLMaths::Sqrtf( GetDistanceSq( Capsule ) );	}
	float			GetDistance(const TSphere& Sphere) const		{	return TLMaths::Sqrtf( GetDistanceSq( Sphere ) );	}
	float			GetDistance(const float3& Pos) const			{	return TLMaths::Sqrtf( GetDistanceSq( Pos ) );	}
	float			GetDistance(const TBox& Box) const				{	return TLMaths::Sqrtf( GetDistanceSq( Box ) );	}
	float			GetDistanceSq(const TCapsule& Capsule) const;
	float			GetDistanceSq(const TSphere& Sphere) const;
	float			GetDistanceSq(const float3& Pos) const;
	float			GetDistanceSq(const TBox& Box) const;

private:
	float3			m_Pos;				//	center of sphere
	float			m_Radius;			//	radius of sphere
};




//---------------------------------------------------------
//	2D sphere shape
//---------------------------------------------------------
class TLMaths::TSphere2D
{
public:
	TSphere2D();
	TSphere2D(const float2& Pos,float Radius);

	static TRef		GetTypeRef()								{	return "Sph2";	}

	void			Set(const float2& Pos,float Radius)			{	m_Pos = Pos;	m_Radius = Radius;	}
	void			Set(const TSphere2D& Sphere)				{	m_Pos = Sphere.GetPos();	m_Radius = Sphere.GetRadius();	}
	void			Set(const TSphere& Sphere)					{	m_Pos = Sphere.GetPos();	m_Radius = Sphere.GetRadius();	}
	void			Set(const TBox2D& Box);						//	use the box's extents to create the biggest sphere that will fit in the box
	void			SetInvalid()								{	m_Radius = -1.f;	}
	Bool			IsValid() const								{	return m_Radius >= 0.f;	}

	const float2&	GetPos() const								{	return m_Pos;	}
	const float2&	GetPosNearestTo(const float2& Pos) const	{	return GetPos();	}
	const float&	GetRadius() const							{	return m_Radius;	}
	float			GetRadiusSq() const							{	return m_Radius*m_Radius;	}
	void			SetRadius(float Radius)						{	m_Radius = Radius;	}

//	void			Accumulate(const TCapsule& Capsule);			//	accumulate other capsule. copies other if this is invalid
//	void			Accumulate(const TSphere& Sphere);				//	grow the sphere to these extents
//	void			Accumulate(const TBox& Box);					//	grow the sphere to these extents
	void			Accumulate(const TBox2D& Box);					//	grow the sphere to these extents
	void			Accumulate(const float3& Point)					{	Accumulate( float2( Point.x, Point.y ) );	}
	void			Accumulate(const float2& Point);				//	grow the sphere to these extents
	void			Accumulate(const TArray<float2>& Points);		//	get the extents of all these points
	void			Accumulate(const TArray<float3>& Points);		//	get the extents of all these points

//	void			Transform(const TLMaths::TMatrix& Matrix,const float3& Scale);	//	transform this shape by this matrix
	void			Transform(const float2& Move)					{	m_Pos += Move;	}
	void			Transform(const TLMaths::TTransform& Transform);	//	transform sphere
	void			Untransform(const TLMaths::TTransform& Transform);	//	untransform sphere

	//	"intersection" is just a bool version of the distance check. (negative distance is an intersection)
//	Bool			GetIntersection(const TCapsule& Capsule) const	{	return GetDistanceSq( Capsule ) < 0.f;	}
//	Bool			GetIntersection(const TSphere& Sphere) const;
	Bool			GetIntersection(const TSphere2D& Sphere) const;
	Bool			GetIntersection(const TLine2D& Line) const;
	Bool			GetIntersection(const float2& Pos) const;
	Bool			GetIntersection(const float3& Pos) const		{	return GetIntersection( Pos.xy() );	}
//	Bool			GetIntersection(const TBox& Box) const;

	//	if a distance returns negative then it's overlapping by that amount - otherwise it's the distance from the edge of each shape
//	float			GetDistance(const TCapsule& Capsule) const		{	return TLMaths::Sqrtf( GetDistanceSq( Capsule ) );	}
//	float			GetDistance(const TSphere& Sphere) const		{	return TLMaths::Sqrtf( GetDistanceSq( Sphere ) );	}
//	float			GetDistance(const float3& Pos) const			{	return TLMaths::Sqrtf( GetDistanceSq( Pos ) );	}
//	float			GetDistance(const TBox& Box) const				{	return TLMaths::Sqrtf( GetDistanceSq( Box ) );	}
//	float			GetDistanceSq(const TCapsule& Capsule) const;
//	float			GetDistanceSq(const TSphere& Sphere) const;
//	float			GetDistanceSq(const float3& Pos) const;
//	float			GetDistanceSq(const TBox& Box) const;

private:
	float2			m_Pos;				//	center of sphere
	float			m_Radius;			//	radius of sphere
};


