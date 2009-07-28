/*------------------------------------------------------
	
	3D and 2D box shapes

-------------------------------------------------------*/
#pragma once
#include <TootleCore/TLMaths.h>
#include <TootleCore/TLTypes.h>
#include <TootleCore/TArray.h>
#include <TootleCore/TFixedArray.h>
#include "TLine.h"


namespace TLMaths
{
	class TBox;		//	3D box shape (AA)
	class TBox2D;	//	2D box shape (AA)

	class TSphere;
	class TSphere2D;
	class TCapsule;
	class TCapsule2D;
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

	static TRef		GetTypeRef()		{	return TLMaths_ShapeRef(TBox);	}

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
	Bool&			IsValid()									{	return m_IsValid;	}
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
	TBox2D() :
		m_IsValid	( FALSE )
	{
	}

	TBox2D(const float2& Min,const float2& Max) : 
		m_Min		( Min ), 
		m_Max		( Max ),
		m_IsValid	( TRUE )
	{
	}

	TBox2D(const float2& Center,float Size) : 
		m_Min		( Center - float2(Size,Size) ), 
		m_Max		( Center + float2(Size,Size) ),
		m_IsValid	( TRUE )
	{
	}

	TBox2D(float MinX,float MinY,float MaxX,float MaxY) :
		m_Min		( MinX, MinY ), 
		m_Max		( MaxX, MaxY ),
		m_IsValid	( TRUE )
	{
	}

	static TRef		GetTypeRef()		{	return TLMaths_ShapeRef(TBox2D);	}

//	const float*	GetData() const		{	return m_MinMax[0].GetData();	}
	float2&			GetMin() 			{	return m_Min;	}
	float2&			GetMax() 			{	return m_Max;	}
	const float2&	GetMin() const		{	return m_Min;	}
	const float2&	GetMax() const		{	return m_Max;	}
	float2			GetSize() const		{	return (m_Max - m_Min);	}
	float2			GetCenter() const				{	return TLMaths::Interp( m_Min, m_Max, 0.5f );	}
	float3			GetCenter3(float z=0.f) const;	//	get the center of the box
	void			GetBoxCorners(TArray<float2>& CornerPositions) const;	//	get the 4 corners of the box - in outline/contour/quad order
	void			GetBoxCorners(TArray<float3>& CornerPositions,float z) const;	//	get the 4 corners of the box - in outline/contour/quad order
	const float&	GetLeft() const		{	return m_Min.x;	}
	float&			GetLeft()			{	return m_Min.x;	}
	const float&	GetTop() const		{	return m_Min.y;	}
	float&			GetTop()			{	return m_Min.y;	}
	const float&	GetRight() const	{	return m_Max.x;	}
	float&			GetRight()			{	return m_Max.x;	}
	const float&	GetBottom() const	{	return m_Max.y;	}
	float&			GetBottom()			{	return m_Max.y;	}
	const float		GetWidth() const	{	return (m_Max.x - m_Min.x);	}
	const float		GetHeight() const	{	return (m_Max.y - m_Min.y);	}
	float			GetHalfWidth() const	{	return (m_Max.x - m_Min.x) * 0.5f;	}
	float			GetHalfHeight() const	{	return (m_Max.y - m_Min.y) * 0.5f;	}
	void			SetWidth(float Width)	{	m_Max.x = m_Min.x + Width;	}	//	set the width by changing the max
	void			SetHeight(float Height)	{	m_Max.y = m_Min.y + Height;	}	//	set the height by changing the max

	void			Set(const float2& Min,const float2& Max)	{	m_Min = Min;	m_Max = Max;	m_IsValid = TRUE;	}
	void			Set(const float2& MinMax)					{	m_Min = MinMax;	m_Max = MinMax;	m_IsValid = TRUE;	}
	void			Set(const TBox& Box)						{	m_Min = Box.GetMin().xy();	m_Max = Box.GetMax().xy();	m_IsValid = Box.IsValid();	}
	void			SetMin(const float2& Min)					{	m_Min = Min;	m_IsValid = TRUE;	}
	void			SetMax(const float2& Max)					{	m_Max = Max;	m_IsValid = TRUE;	}
	void			SetInvalid()								{	m_IsValid = FALSE;	}
	void			SetValid(Bool Valid=TRUE)					{	m_IsValid = Valid;	}
	Bool&			IsValid()									{	return m_IsValid;	}
	Bool			IsValid() const								{	return m_IsValid;	}

	void			Accumulate(const TBox2D& Box);				//	accumulate other box. copies other box if this is invalid
	void			Accumulate(const TBox& Box);				//	accumulate other box. copies other box if this is invalid
	void			Accumulate(const TSphere2D& Sphere);		//	accumulate sphere
	void			Accumulate(const float2& Point);			//	grow the box to these extents
	void			Accumulate(const float3& Point)				{	Accumulate( Point.xy() );	}
	void			Accumulate(const TArray<float2>& Points);	//	get the extents of all these points
	void			Accumulate(const TArray<float3>& Points);	//	get the extents of all these points

	inline void		CenterBox()									{	float2 Center = GetCenter();	m_Min -= Center;	m_Max -= Center;	}	//	this will center the box around 0,0
	void			GrowBox(float Scale);						//	grow/shrink this box around its center

	void			Transform(const TLMaths::TTransform& Transform);	//	transform this shape
	void			Transform(const float2& Move)						{	m_Min += Move;	m_Max += Move;	}
	void			Transform(const float3& Move)						{	m_Min += Move.xy();	m_Max += Move.xy();	}
	void			Untransform(const TLMaths::TTransform& Transform);	//	untransform box

	//	"intersection" is just a bool version of the distance check. (negative distance is an intersection)
	Bool			GetIntersection(const TLine& Line) const			{	return GetIntersection( Line.GetStart().xy(), Line.GetStart().xy() );	}
	Bool			GetIntersection(const TLine2D& Line) const			{	return GetIntersection( Line.GetStart(), Line.GetStart() );	}
	Bool			GetIntersection(const float2& Pos) const;
	Bool			GetIntersection(const float3& Pos) const;
	Bool			GetIntersection(const TSphere& Sphere) const;
	Bool			GetIntersection(const TSphere2D& Sphere) const;
	Bool			GetIntersection(const TCapsule& Capsule) const;
	Bool			GetIntersection(const TCapsule2D& Capsule) const;
	Bool			GetIntersection(const TBox2D& Box) const;
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
	void			operator*=(float v)				{	m_Min *= v;	m_Max *= v;	}
	void			operator/=(float v)				{	m_Min /= v;	m_Max /= v;	}

protected:
	Bool			GetIntersection(const float2& LineStart,const float2& LineEnd) const;

protected:
	float2		m_Min;
	float2		m_Max;
	Bool		m_IsValid;			//	validity of bounding box is stored on the box... much easier for us
};


