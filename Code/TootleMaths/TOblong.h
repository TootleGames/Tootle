/*------------------------------------------------------
	
	3D and 2D oblong (non axis-aligned box) shapes

-------------------------------------------------------*/
#pragma once
#include <TootleCore/TLMaths.h>
#include <TootleCore/TLTypes.h>
#include <TootleCore/TArray.h>
#include <TootleCore/TFixedArray.h>


namespace TLMaths
{
	class TOblong;		//	3D orientated box - this is an oblong (though perhaps not square). 8 points in space
	class TOblong2D;	//	2D orientated box - this is essentially just a quad. 4 points in space

	class TSphere;
	class TSphere2D;
	class TCapsule;
	class TMatrix;
	class TTransform;

};


//-------------------------------------------------------------------
//	3D orientated box
//-------------------------------------------------------------------
class TLMaths::TOblong
{
public:
	TOblong();

	static TRef		GetTypeRef()		{	return "Obl";	}
/*
	const float*	GetData() const		{	return m_MinMax[0].GetData();	}
	float3&			GetMin() 			{	return m_Min;	}
	float3&			GetMax() 			{	return m_Max;	}
	const float3&	GetMin() const		{	return m_Min;	}
	const float3&	GetMax() const		{	return m_Max;	}
	float3			GetCenter() const;	//	get the center of the box
	float3			GetSize() const		{	return (m_Max - m_Min);	}
*/	
	TFixedArray<float3,8>&			GetBoxCorners()				{	return m_Corners;	}
	const TFixedArray<float3,8>&	GetBoxCorners() const		{	return m_Corners;	}
/*	
	void			Set(const float3& Min,const float3& Max)	{	m_Min = Min;	m_Max = Max;	m_IsValid = TRUE;	}
	void			Set(const float3& MinMax)					{	m_Min = MinMax;	m_Max = MinMax;	m_IsValid = TRUE;	}
	void			SetMin(const float3& Min)					{	m_Min = Min;	m_IsValid = TRUE;	}
	void			SetMax(const float3& Max)					{	m_Max = Max;	m_IsValid = TRUE;	}
*/	void			SetInvalid()								{	m_IsValid = FALSE;	}
	void			SetValid(Bool Valid=TRUE)					{	m_IsValid = Valid;	}
	Bool			IsValid() const								{	return m_IsValid;	}
/*
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
*/
protected:
	TFixedArray<float3,8>	m_Corners;
	Bool					m_IsValid;			//	validity of bounding box is stored on the box... much easier for us
};






//-------------------------------------------------------------------
//	2D orientated box
//-------------------------------------------------------------------
class TLMaths::TOblong2D
{
public:
	TOblong2D();

	static TRef		GetTypeRef()		{	return "Obl2";	}

	float2			GetCenter() const;	//	get the center of the box

	TFixedArray<float2,4>&			GetBoxCorners()				{	return m_Corners;	}
	const TFixedArray<float2,4>&	GetBoxCorners() const		{	return m_Corners;	}
/*	
	void			Set(const float3& Min,const float3& Max)	{	m_Min = Min;	m_Max = Max;	m_IsValid = TRUE;	}
	void			Set(const float3& MinMax)					{	m_Min = MinMax;	m_Max = MinMax;	m_IsValid = TRUE;	}
	void			SetMin(const float3& Min)					{	m_Min = Min;	m_IsValid = TRUE;	}
	void			SetMax(const float3& Max)					{	m_Max = Max;	m_IsValid = TRUE;	}
*/	void			SetInvalid()								{	m_IsValid = FALSE;	}
	void			SetValid(Bool Valid=TRUE)					{	m_IsValid = Valid;	}
	Bool			IsValid() const								{	return m_IsValid;	}
/*
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
*/
protected:
	TFixedArray<float2,4>	m_Corners;			//	the corners are in quad/outline-format. TopLeft,TopRight,BottomRight,BottomLeft.
	Bool					m_IsValid;			//	validity of bounding box is stored on the box... much easier for us
};

