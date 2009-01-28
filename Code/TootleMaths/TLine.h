/*------------------------------------------------------
	
	3D and 2D line shapes

-------------------------------------------------------*/
#pragma once
#include <TootleCore/TLMaths.h>
#include <TootleCore/TLTypes.h>


namespace TLMaths
{
	class TLine;		//	3D line shape
	class TLine2D;		//	2D line shape
	class TSphere2D;	//	
};


class TLMaths::TLine
{
public:
	TLine()				{	}
	TLine(const float3& Start,const float3& End);
	void				Set(const float3& Start,const float3& End)		{	m_Start = Start;	m_End = End;	}
	void				Set(const TLine& Line)							{	m_Start = Line.GetStart();	m_End = Line.GetEnd();	}
	void				SetDir(const float3& Start,const float3& Dir)	{	Set( Start, Start + Dir );	}
	void				SetStart(const float3& Start)					{	m_Start = Start;	}
	void				SetEnd(const float3& End)						{	m_End = End;	}

	const float3&		GetStart() const								{	return m_Start;	}
	const float3&		GetEnd() const									{	return m_End;	}
	float3				GetCenter() const								{	return (m_Start + m_End) * 0.5f;	}
	float3				GetDirection() const							{	return m_End - m_Start;	}
	float3				GetDirectionNormal() const						{	return (m_End - m_Start).Normal();	}
	float				GetLengthSq() const								{	return GetDirection().LengthSq();	}
	float				GetLength() const								{	return GetDirection().Length();	}

	Bool				GetIntersection(const float3& Pos) const		{	return GetDistanceSq( Pos ) <= 0.f;	}
	float				GetDistance(const float3& Pos) const			{	return TLMaths::Sqrtf( GetDistanceSq( Pos ) );	}
	float				GetDistanceSq(const float3& Pos) const;			//	get the distance from point to the line. returns ZERO if on the line (no mathematical way of doing a negative result)
	float3				GetNearestPoint(const float3& Pos) const;		//	get a point along the line nearest to this point
	
	void				Transform(const float3& Move)					{	m_Start += Move;	m_End += Move;	}

public:
	float3				m_Start;	//	line start
	float3				m_End;		//	line end
};



class TLMaths::TLine2D
{
public:
	TLine2D()				{	}
	TLine2D(const float2& Start,const float2& End);
	TLine2D(const float3& Start,const float3& End);
	void				Set(const float2& Start,const float2& End)		{	m_Start = Start;	m_End = End;	}
	void				Set(const TLine2D& Line)						{	m_Start = Line.GetStart();	m_End = Line.GetEnd();	}
	void				Set(const TLine& Line)							{	m_Start = Line.GetStart();	m_End = Line.GetEnd();	}
	void				SetDir(const float2& Start,const float2& Dir)	{	Set( Start, Start + Dir );	}

	const float2&		GetStart() const								{	return m_Start;	}
	const float2&		GetEnd() const									{	return m_End;	}
	float2				GetCenter() const								{	return (m_Start + m_End) * 0.5f;	}
	float2				GetDirection() const							{	return m_End - m_Start;	}
	float3				GetDirection(float z) const						{	return (m_End - m_Start).xyz(z);	}
	float				GetLengthSq() const								{	return GetDirection().LengthSq();	}
	float				GetLength() const								{	return GetDirection().Length();	}

	Bool				GetIntersectionPos(const TLine2D& Line,float2& IntersectionPos) const;	//	get the point where this line crosses the other
	Bool				GetIntersectionPos(const TLine2D& Line,float& IntersectionAlongThis,float& IntersectionAlongLine) const;	//	checks for intersection and returns where along the line on both cases where it intersects

	Bool				GetIntersection(const float2& Pos) const		{	return GetDistanceSq( Pos ) <= 0.f;	}
	Bool				GetIntersection(const TSphere2D& Sphere) const;
	Bool				GetIntersection(const TLine2D& Line) const;		//	simple intersection test
	float				GetDistance(const float2& Pos) const			{	return TLMaths::Sqrtf( GetDistanceSq( Pos ) );	}
	float				GetDistanceSq(const float2& Pos) const;			//	get the distance from point to the line. returns ZERO if on the line (no mathematical way of doing a negative result)
	float2				GetNearestPoint(const float2& Pos) const;		//	get a point along the line nearest to this point
	float2				GetNearestPoint(const float3& Pos) const;		//	get a point along the line nearest to this point
	
	void				Transform(const float2& Move)					{	m_Start += Move;	m_End += Move;	}
	void				Transform(const float3& Move)					{	m_Start += Move;	m_End += Move;	}

public:
	float2				m_Start;	//	line start
	float2				m_End;		//	line end
};


