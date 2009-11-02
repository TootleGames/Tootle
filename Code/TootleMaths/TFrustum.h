/*------------------------------------------------------
	
	Frustum class

-------------------------------------------------------*/
#pragma once
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

	TOblong&		GetBox()				{	return m_Box;	}
	const TOblong&	GetBox() const			{	return m_Box;	}

protected:
	float					m_Near;		//	near z - should be able to extract this from the plane really...
	float					m_Far;		//	far z - should be able to extract this from the plane really...

	TOblong					m_Box;		//	3D frustum box
};




