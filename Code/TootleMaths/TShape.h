/*------------------------------------------------------

	Overloadable shape type

-------------------------------------------------------*/
#pragma once

#include "TSphere.h"
#include "TBox.h"


namespace TLMaths
{
	class TShape;			//	base shape type
	class TShapeSphere2D;	//	sphere shape
	class TShapeSphere;		//	sphere shape

	class TIntersection;	//	resulting intersection information of two shapes
}




class TLMaths::TIntersection
{
public:
	TIntersection(const float3& IntersectionPos) :
		m_Position	( IntersectionPos )
	{
	}

	const float3&	GetPosition() const		{	return m_Position;	}

protected:
	float3			m_Position;		//	intersecting position
};




class TLMaths::TShape
{
public:
	TShape()															{	}
	virtual ~TShape()													{	}

	virtual TRef					GetShapeType() const = 0;					//	return a shape type
	virtual Bool					IsValid() const = 0;						//	check if the shape is valid
	virtual float3					GetCenter() const = 0;						//	get the center of the shape

	Bool							HasIntersection(TShape& OtherShape);
	Bool							GetIntersection(TShape& OtherShape,TIntersection& Intersection);

protected:
	void							Debug_BreakOverloadThis(const char* pTestType,TShape& OtherShape);

protected:
	//	simple fast intersection tests which don't need intersection information
//	virtual Bool					HasIntersection_Sphere(TShapeSphere& OtherShape);
	virtual Bool					HasIntersection_Sphere2D(TShapeSphere2D& OtherShape);
//	virtual Bool					HasIntersection_Box(TShapeBox& OtherShape);
//	virtual Bool					HasIntersection_Box2D(TShapeBox2D& OtherShape);

//	virtual Bool					GetIntersection_Sphere(TShapeSphere& OtherShape,TIntersection& Intersection);
	virtual Bool					GetIntersection_Sphere2D(TShapeSphere2D& OtherShape,TIntersection& Intersection);
//	virtual Bool					GetIntersection_Box(TShapeBox& OtherShape,TIntersection& Intersection);
//	virtual Bool					GetIntersection_Box2D(TShapeBox2D& OtherShape,TIntersection& Intersection);
};




class TLMaths::TShapeSphere2D : public TLMaths::TShape
{
public:
	TShapeSphere2D()															{}
	TShapeSphere2D(const TLMaths::TSphere2D& Sphere) : m_Sphere ( Sphere )		{}

	virtual TRef					GetShapeType() const						{	return TLMaths::TSphere2D::GetTypeRef();	}
	virtual Bool					IsValid() const								{	return GetSphere().IsValid();	}
	virtual float3					GetCenter() const							{	return GetSphere().GetPos();	}
	
	void							SetSphere(const TLMaths::TSphere2D& Sphere)	{	m_Sphere = Sphere;	}
	const TLMaths::TSphere2D&		GetSphere() const							{	return m_Sphere;	}

protected:
	TLMaths::TSphere2D				m_Sphere;			//	sphere collision object
};




class TLMaths::TShapeSphere : public TLMaths::TShape
{
public:
	TShapeSphere()															{}
	TShapeSphere(const TLMaths::TSphere& Sphere) : m_Sphere ( Sphere )		{}

	virtual TRef					GetShapeType() const						{	return TLMaths::TSphere::GetTypeRef();	}
	virtual Bool					IsValid() const								{	return GetSphere().IsValid();	}
	virtual float3					GetCenter() const							{	return GetSphere().GetPos();	}
	
	void							SetSphere(const TLMaths::TSphere& Sphere)	{	m_Sphere = Sphere;	}
	const TLMaths::TSphere&			GetSphere() const							{	return m_Sphere;	}

protected:
	TLMaths::TSphere				m_Sphere;			//	sphere collision object
};

