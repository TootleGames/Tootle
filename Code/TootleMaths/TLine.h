/*------------------------------------------------------
	
	3D and 2D line shapes

-------------------------------------------------------*/
#pragma once
#include <TootleCore/TLMaths.h>
#include <TootleCore/TLTypes.h>
#include <TootleCore/TArray.h>


namespace TLMaths
{
	class TLine;		//	3D line shape
	class TLine2D;		//	2D line shape
	class TSphere2D;	//	

	void		ExpandLineStrip(const TArray<float3>& LineStrip,float Width,TArray<float3>& OutsideLineStrip,TArray<float3>& InsideLineStrip);	//	create an outside and inside linestrip for an existing linestrip
	float3		GetLineStripOutset(const float3& Start,const float3& Middle,const float3& End,float OutsetLength=1.f);	//	calculates the (normalised by default) outset for the Middle of a line strip (outset is relative)	//	gr: assumes line is clockwise. if not (and you have to detect that) then invert result
	float3		GetLineOutset(const float3& Start,const float3& End,float OutsetLength=1.f);	//	calculates the outset for a line (outset is relative)
	void		GetNearestLinePoints(const TLMaths::TLine2D& LineA,const TLMaths::TLine2D& LineB,Bool& LineANearStart,Bool& LineBNearStart,float& DistanceSq);	//	work out which two ends of these lines are closest. Set's a combination of bools to TRUE when it's a start, FALSE if its nearer the end, returns the distance sq of the nearest points too
};


class TLMaths::TLine
{
public:
	TLine()				{	}
	TLine(const float3& Start,const float3& End);
	TLine(const TLine2D& Line,float z=0.f);

	static TRef			GetTypeRef()		{	return "Line";	}

	void				Set(const float3& Start,const float3& End)		{	m_Start = Start;	m_End = End;	}
	void				Set(const TLine& Line)							{	m_Start = Line.GetStart();	m_End = Line.GetEnd();	}
	void				SetDir(const float3& Start,const float3& Dir)	{	Set( Start, Start + Dir );	}
	void				SetStart(const float3& Start)					{	m_Start = Start;	}
	void				SetEnd(const float3& End)						{	m_End = End;	}

	const float3&		GetStart() const								{	return m_Start;	}
	const float3&		GetEnd() const									{	return m_End;	}
	float3				GetCenter() const								{	return (m_Start + m_End) * 0.5f;	}
	float3				GetDirection() const							{	return m_End - m_Start;	}
	float3				GetDirectionNormal(float NormalLength=1.f) const	{	return (m_End - m_Start).Normal(NormalLength);	}
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
	TLine2D(const TLine& Line);

	static TRef			GetTypeRef()									{	return "Line2";	}

	void				Set(const float2& Start,const float2& End)		{	m_Start = Start;	m_End = End;	}
	void				SetStart(const float2& Start)					{	m_Start = Start;	}
	void				SetEnd(const float2& End)						{	m_End = End;	}
	void				Set(const TLine2D& Line)						{	m_Start = Line.GetStart();	m_End = Line.GetEnd();	}
	void				Set(const TLine& Line)							{	m_Start = Line.GetStart();	m_End = Line.GetEnd();	}
	void				SetDir(const float2& Start,const float2& Dir)	{	Set( Start, Start + Dir );	}

	float2&				GetStart()										{	return m_Start;	}
	float2&				GetEnd()										{	return m_End;	}
	const float2&		GetStart() const								{	return m_Start;	}
	const float2&		GetEnd() const									{	return m_End;	}
	float2				GetCenter() const								{	return (m_Start + m_End) * 0.5f;	}
	float2				GetDirection() const							{	return m_End - m_Start;	}
	float3				GetDirection(float z) const						{	return (m_End - m_Start).xyz(z);	}
	float2				GetDirectionNormal(float NormalLength=1.f) const	{	return (m_End - m_Start).Normal(NormalLength);	}
	float				GetLengthSq() const								{	return (m_End - m_Start).LengthSq();	}
	float				GetLength() const								{	return (m_End - m_Start).Length();	}
	TLMaths::TAngle		GetAngle() const;								//	calculate angle of line

	Bool				GetIntersectionPos(const TLine2D& Line,float2& IntersectionPos) const;	//	get the point where this line crosses the other
	Bool				GetIntersectionPos(const TLine2D& Line,float& IntersectionAlongThis,float& IntersectionAlongLine) const;	//	checks for intersection and returns where along the line on both cases where it intersects

	Bool				GetIntersection(const float2& Pos) const		{	return GetDistanceSq( Pos ) <= 0.f;	}
	Bool				GetIntersection(const TSphere2D& Sphere) const;
	Bool				GetIntersection(const TLine2D& Line) const;		//	simple intersection test
	SyncBool			GetIntersectionDistance(const TLine2D& Line,float& IntersectionAlongThis,float& IntersectionAlongLine) const;	//	like GetIntersectionPos... return WAIT if the lines intersect past their extents
	float				GetDistance(const float2& Pos) const			{	return TLMaths::Sqrtf( GetDistanceSq( Pos ) );	}
	float				GetDistanceSq(const float2& Pos) const;			//	get the distance from point to the line. returns ZERO if on the line (no mathematical way of doing a negative result)
	float2				GetNearestPoint(const float2& Pos) const		{	float pal;	return GetNearestPoint( Pos, pal );	}
	float2				GetNearestPoint(const float2& Pos,float& PointAlongLine) const;		//	get a point along the line nearest to this point
	float2				GetNearestPoint(const float3& Pos) const		{	return GetNearestPoint( Pos.xy() );	}
	float				GetDistanceSq(const TLine2D& Line) const;		//	get distance to another line
	Bool				GetIsPointNearestToStart(const float2& Pos,float& DistanceSq) const;	//	return whether this point is nearer to the start, or to the end. DistanceSq is set to the distance (commonly used afterwards)

	void				GetPointAlongLine(float2& PointAlongLine,float Factor) const;	//	get a point along the line from 0..1 (factor)
	void				MoveStart(float Distance);						//	move the start point along the direction by an amount (NOT a factor)
	void				MoveEnd(float Distance);						//	move the end point along the direction by an amount (NOT a factor)

	void				Transform(const float2& Move)					{	m_Start += Move;	m_End += Move;	}
	void				Transform(const float3& Move)					{	m_Start += Move;	m_End += Move;	}
	void				Transform(const TLMaths::TTransform& Transform);

public:
	float2				m_Start;	//	line start
	float2				m_End;		//	line end
};


