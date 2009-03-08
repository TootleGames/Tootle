/*------------------------------------------------------
	
	Frustum class

-------------------------------------------------------*/
#pragma once
#include <TootleCore/TLMaths.h>
#include <TootleCore/TFixedArray.h>
#include "TOblong.h"


namespace TLMaths
{
	class TFrustum;
	class TPlane;
};





//-----------------------------------------------
//	3D plane
//-----------------------------------------------
class TLMaths::TPlane
{
public:

	float&			x()					{	return m_Plane.x;	}
	const float&	x() const			{	return m_Plane.x;	}
	float&			y()					{	return m_Plane.y;	}
	const float&	y() const			{	return m_Plane.y;	}
	float&			z()					{	return m_Plane.z;	}
	const float&	z() const			{	return m_Plane.z;	}
	float&			w()					{	return m_Plane.w;	}
	const float&	w() const			{	return m_Plane.w;	}
	float4&			xyzw()				{	return m_Plane;	}
	const float4&	xyzw() const		{	return m_Plane;	}
	const float3&	GetNormal() const	{	return m_Plane.xyz();	}

protected:
	float4		m_Plane;
};


//-----------------------------------------------
//	Camera frustum, box of 6 planes really
//-----------------------------------------------
class TLMaths::TFrustum
{
public:
	TFrustum();

	void			SetNearFar(float Near,float Far)	{	m_Near = Near;	m_Far = Far;	}

	TPlane&			GetLeftPlane()			{	return m_Planes[0];	}
	const TPlane&	GetLeftPlane() const	{	return m_Planes[0];	}
	TPlane&			GetRightPlane()			{	return m_Planes[1];	}
	const TPlane&	GetRightPlane() const	{	return m_Planes[1];	}
	TPlane&			GetTopPlane()			{	return m_Planes[2];	}
	const TPlane&	GetTopPlane() const		{	return m_Planes[2];	}
	TPlane&			GetBottomPlane()		{	return m_Planes[3];	}
	const TPlane&	GetBottomPlane() const	{	return m_Planes[3];	}
	TPlane&			GetNearPlane()			{	return m_Planes[4];	}
	const TPlane&	GetNearPlane() const	{	return m_Planes[4];	}
	TPlane&			GetFarPlane()			{	return m_Planes[5];	}
	const TPlane&	GetFarPlane() const		{	return m_Planes[5];	}

	TOblong&		GetBox()				{	return m_Box;	}
	const TOblong&	GetBox() const			{	return m_Box;	}

	Bool			HasIntersection(const float3& Point) const;			//	check if this point is inside the frustum
	Bool			HasIntersection(const float2& Point) const;			//	check if this 2D point is inside the frustum
	Bool			HasIntersection(const TSphere& Sphere) const;		//	check if sphere is inside frustum
	Bool			HasIntersection(const TLMaths::TBox2D& Box) const;	//	check to see if this box is inside/clipping the frustum - 2D so only checks against top/left/right/bottom
	Bool			HasIntersection(const TLMaths::TBox& Box,Bool TestNearFarPlanes=TRUE) const;	//	check to see if this box is the frustum at all

protected:
	TFixedArray<TPlane,6>	m_Planes;
	float					m_Near;		//	near z - should be able to extract this from the plane really...
	float					m_Far;		//	far z - should be able to extract this from the plane really...

	TOblong					m_Box;		//	3D frustum box
};




