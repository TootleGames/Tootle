/*------------------------------------------------------
	
	3D and 2D capsule shapes

-------------------------------------------------------*/
#pragma once
#include <TootleCore/TLMaths.h>
#include <TootleCore/TLMathsMisc.h>
#include "TLine.h"


namespace TLMaths
{
	class TCapsule;		//	3D capsule shape
	class TCapsule2D;	//	2D capsule shape(circle)

	class TSphere;
};




//---------------------------------------------------------
//	capsule shape
//---------------------------------------------------------
class TLMaths::TCapsule
{
public:
	TCapsule() : m_Radius ( -1.f )													{	}	//	invalid
	TCapsule(const TLine2D& Line,float Radius) : m_Line ( Line ), m_Radius ( Radius )	{	}
	TCapsule(const float3& Start,const float3& End,float Radius) : m_Line ( Start, End ), m_Radius ( Radius )	{	}

	static TRef		GetTypeRef()											{	return TLMaths_ShapeRef(TCapsule);	}

	void			Set(const TLine& Line,float Radius)						{	m_Line = Line;	m_Radius = Radius;	}
	void			Set(const float3& Start,const float3& End,float Radius)	{	m_Line.Set( Start, End );	m_Radius = Radius;	}
	void			Set(const TCapsule& Capsule)							{	m_Line.Set( Capsule.GetLine() );	m_Radius = Capsule.GetRadius();	}
	void			SetInvalid()											{	m_Radius = -1.f;	}
	Bool			IsValid() const											{	return m_Radius >= 0.f;	}

	float3			GetCenter() const								{	return m_Line.GetCenter();	}
	float3			GetPos() const									{	return GetCenter();	}
	TLine&			GetLine()										{	return m_Line;	}
	const TLine&	GetLine() const									{	return m_Line;	}
	float3			GetPosNearestTo(const float3& Pos) const		{	return GetLine().GetNearestPoint( Pos );	}
	float&			GetRadius()										{	return m_Radius;	}
	const float&	GetRadius() const								{	return m_Radius;	}
	void			SetRadius(float Radius)							{	m_Radius = Radius;	}

	TSphere			GetSphere() const;								//	create sphere from capsule

	void			Accumulate(const TCapsule& Capsule);			//	accumulate other capsule. copies other if this is invalid
	void			Accumulate(const float3& Point);				//	grow the box to these extents
	void			Accumulate(const TArray<float3>& Points);		//	get the extents of all these points

	void			Transform(const TLMaths::TMatrix& Matrix,const float3& Scale);	//	transform this shape by this matrix
	void			Transform(const float3& Move)					{	m_Line.Transform( Move );	}
	void			Transform(const TLMaths::TTransform& Transform);	//	transform sphere
	void			Untransform(const TLMaths::TTransform& Transform);	//	untransform sphere

	//	"intersection" is just a bool version of the distance check. (negative distance is an intersection)
	Bool			GetIntersection(const TCapsule& Capsule) const	{	return GetDistanceSq( Capsule ) < 0.f;	}
	Bool			GetIntersection(const TSphere& Sphere) const	{	return GetDistanceSq( Sphere ) < 0.f;	}
	Bool			GetIntersection(const float3& Pos) const		{	return GetDistanceSq( Pos ) < 0.f;	}

	//	if a distance returns negative then it's overlapping by that amount - otherwise it's the distance from the edge of each shape
	float			GetDistance(const TCapsule& Capsule) const		{	return TLMaths::Sqrtf( GetDistanceSq( Capsule ) );	}
	float			GetDistance(const TSphere& Sphere) const		{	return TLMaths::Sqrtf( GetDistanceSq( Sphere ) );	}
	float			GetDistance(const float3& Pos) const			{	return TLMaths::Sqrtf( GetDistanceSq( Pos ) );	}
	float			GetDistanceSq(const TCapsule& Capsule) const;
	float			GetDistanceSq(const TSphere& Sphere) const;
	float			GetDistanceSq(const float3& Pos) const;

protected:
	TLine			m_Line;				//	start and end of capsule
	float			m_Radius;			//	radius of sphere/capsule
};





//---------------------------------------------------------
//	capsule shape
//---------------------------------------------------------
class TLMaths::TCapsule2D
{
public:
	TCapsule2D() : m_Radius ( -1.f )													{	}	//	invalid
	TCapsule2D(const TLine2D& Line,float Radius) : m_Line ( Line ), m_Radius ( Radius )	{	}
	TCapsule2D(const float2& Start,const float2& End,float Radius) : m_Line ( Start, End ), m_Radius ( Radius )	{	}

	static TRef		GetTypeRef()											{	return TLMaths_ShapeRef(TCapsule2D);	}

	void			Set(const TLine2D& Line,float Radius)					{	m_Line = Line;	m_Radius = Radius;	}
	void			Set(const float2& Start,const float2& End,float Radius)	{	m_Line.Set( Start, End );	m_Radius = Radius;	}
	void			Set(const TLMaths::TBox2D& Box);						//	create a capsule out of a box - gr: note this function doesn't envelope the box, it uses it's extents as dimensions. always use ACCUMULATE functions for bounding shapes
	void			SetInvalid()											{	m_Radius = -1.f;	}
	Bool			IsValid() const											{	return m_Radius >= 0.f;	}

	float2			GetCenter() const								{	return m_Line.GetCenter();	}
	float2			GetPos() const									{	return GetCenter();	}
	TLine2D&		GetLine()										{	return m_Line;	}
	const TLine2D&	GetLine() const									{	return m_Line;	}
	float2			GetPosNearestTo(const float2& Pos) const		{	return GetLine().GetNearestPoint( Pos );	}
	float&			GetRadius()										{	return m_Radius;	}
	float			GetRadiusSq() const								{	return m_Radius*m_Radius;	}
	const float&	GetRadius() const								{	return m_Radius;	}
	void			SetRadius(float Radius)							{	m_Radius = Radius;	}
	
//	Bool			GetIntersection(const TBox2D& Box) const;

	void			Transform(const TLMaths::TTransform& Transform);	//	transform capsule

protected:
	TLine2D			m_Line;				//	start and end of capsule
	float			m_Radius;			//	radius of capsule
};


