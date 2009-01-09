/*------------------------------------------------------
	
	3D and 2D box shapes

-------------------------------------------------------*/
#pragma once
#include <TootleCore/TLMaths.h>
#include <TootleCore/TLTypes.h>


namespace TLMaths
{
	class TBox;		//	3D box shape
	class TBox2D;	//	2D box shape

	class TSphere;
	class TSphere2D;
	class TCapsule;
	class TMatrix;
	class TTransform;

};



//-------------------------------------------------------------------
//	3D box shape
//-------------------------------------------------------------------
class TLMaths::TBox
{
public:
	TBox();
	TBox(const float3& Min,const float3& Max);

//	const float*	GetData() const		{	return m_MinMax[0].GetData();	}
	float3&			GetMin() 			{	return m_Min;	}
	float3&			GetMax() 			{	return m_Max;	}
	const float3&	GetMin() const		{	return m_Min;	}
	const float3&	GetMax() const		{	return m_Max;	}
	float3			GetCenter() const;	//	get the center of the box
	float3			GetSize() const		{	return (m_Max - m_Min);	}
	void			GetBoxCorners(TArray<float3>& CornerPositions) const;	//	get the 8 corners of the box
	
	void			Set(const float3& Min,const float3& Max)	{	m_Min = Min;	m_Max = Max;	m_IsValid = TRUE;	}
	void			Set(const float3& MinMax)					{	m_Min = MinMax;	m_Max = MinMax;	m_IsValid = TRUE;	}
	void			SetMin(const float3& Min)					{	m_Min = Min;	m_IsValid = TRUE;	}
	void			SetMax(const float3& Max)					{	m_Max = Max;	m_IsValid = TRUE;	}
	void			SetInvalid()								{	m_IsValid = FALSE;	}
	void			SetValid(Bool Valid=TRUE)					{	m_IsValid = Valid;	}
	Bool			IsValid() const								{	return m_IsValid;	}

	void			Accumulate(const TBox& Box);				//	accumulate other box. copies other box if this is invalid
	void			Accumulate(const TSphere& Sphere);			//	accumulate sphere
	void			Accumulate(const TCapsule& Capsule);		//	accumulate capsule
	void			Accumulate(const float3& Point);			//	grow the box to these extents
	void			Accumulate(const TArray<float3>& Points);	//	get the extents of all these points

	void			Transform(const TLMaths::TMatrix& Matrix,const float3& Scale);	//	transform this shape by this matrix
	void			Transform(const TLMaths::TTransform& Transform);	//	transform this shape
	void			Transform(const float3& Move)						{	m_Min += Move;	m_Max += Move;	}
	void			Untransform(const TLMaths::TTransform& Transform);	//	untransform box

	//	"intersection" is just a bool version of the distance check. (negative distance is an intersection)
	Bool			GetIntersection(const TLine& Line) const;
	Bool			GetIntersection(const float3& Pos) const;
	Bool			GetIntersection(const TSphere& Sphere) const;
	Bool			GetIntersection(const TCapsule& Capsule) const;
	Bool			GetIntersection(const TBox& Box) const;

	//	if a distance returns negative then it's overlapping by that amount - otherwise it's the distance from the edge of each shape
	float			GetDistance(const TLine& Line) const		{	return TLMaths::Sqrtf( GetDistanceSq( Line ) );	}
	float			GetDistance(const float3& Pos) const		{	return TLMaths::Sqrtf( GetDistanceSq( Pos ) );	}
	float			GetDistanceSq(const TLine& Line) const;
	float			GetDistanceSq(const float3& Pos) const;
	
	void			operator+=(const float3& v)		{	m_Min += v;	m_Max += v;	}
	void			operator-=(const float3& v)		{	m_Min -= v;	m_Max -= v;	}
	void			operator*=(const float3& v)		{	m_Min *= v;	m_Max *= v;	}
	void			operator/=(const float3& v)		{	m_Min /= v;	m_Max /= v;	}

protected:
	float3		m_Min;
	float3		m_Max;
	Bool		m_IsValid;			//	validity of bounding box is stored on the box... much easier for us
};





//-------------------------------------------------------------------
//	2D box shape
//-------------------------------------------------------------------
class TLMaths::TBox2D
{
public:
	TBox2D();
	TBox2D(const float2& Min,const float2& Max);

//	const float*	GetData() const		{	return m_MinMax[0].GetData();	}
	float2&			GetMin() 			{	return m_Min;	}
	float2&			GetMax() 			{	return m_Max;	}
	const float2&	GetMin() const		{	return m_Min;	}
	const float2&	GetMax() const		{	return m_Max;	}
	float2			GetCenter() const				{	return TLMaths::Interp( m_Min, m_Max, 0.5f );	}
	float3			GetCenter3(float z=0.f) const;	//	get the center of the box
	void			GetBoxCorners(TArray<float2>& CornerPositions) const;	//	get the 8 corners of the box

	void			Set(const float2& Min,const float2& Max)	{	m_Min = Min;	m_Max = Max;	m_IsValid = TRUE;	}
	void			Set(const float2& MinMax)					{	m_Min = MinMax;	m_Max = MinMax;	m_IsValid = TRUE;	}
	void			SetMin(const float2& Min)					{	m_Min = Min;	m_IsValid = TRUE;	}
	void			SetMax(const float2& Max)					{	m_Max = Max;	m_IsValid = TRUE;	}
	void			SetInvalid()								{	m_IsValid = FALSE;	}
	Bool			IsValid() const								{	return m_IsValid;	}

//	void			Accumulate(const TBox& Box);				//	accumulate other box. copies other box if this is invalid
//	void			Accumulate(const TSphere& Sphere);			//	accumulate sphere
	void			Accumulate(const float2& Point);			//	grow the box to these extents
	void			Accumulate(const TArray<float2>& Points);	//	get the extents of all these points

	void			Transform(const TLMaths::TTransform& Transform);	//	transform this shape
	void			Transform(const float2& Move)						{	m_Min += Move;	m_Max += Move;	}
	void			Untransform(const TLMaths::TTransform& Transform);	//	untransform box

	//	"intersection" is just a bool version of the distance check. (negative distance is an intersection)
	Bool			GetIntersection(const TLine& Line) const;
	Bool			GetIntersection(const float2& Pos) const;
	Bool			GetIntersection(const float3& Pos) const;
	Bool			GetIntersection(const TSphere& Sphere) const;
	Bool			GetIntersection(const TSphere2D& Sphere) const;
	Bool			GetIntersection(const TCapsule& Capsule) const;
	Bool			GetIntersection(const TBox& Box) const;

	//	if a distance returns negative then it's overlapping by that amount - otherwise it's the distance from the edge of each shape
	float			GetDistance(const TLine& Line) const		{	return TLMaths::Sqrtf( GetDistanceSq( Line ) );	}
	float			GetDistance(const float2& Pos) const		{	return TLMaths::Sqrtf( GetDistanceSq( Pos ) );	}
	float			GetDistanceSq(const TLine& Line) const;
	float			GetDistanceSq(const float2& Pos) const;
	
	void			operator+=(const float2& v)		{	m_Min += v;	m_Max += v;	}
	void			operator-=(const float2& v)		{	m_Min -= v;	m_Max -= v;	}
	void			operator*=(const float2& v)		{	m_Min *= v;	m_Max *= v;	}
	void			operator/=(const float2& v)		{	m_Min /= v;	m_Max /= v;	}

protected:
	float2		m_Min;
	float2		m_Max;
	Bool		m_IsValid;			//	validity of bounding box is stored on the box... much easier for us
};
