/*------------------------------------------------------

	sphere kind of TShape

-------------------------------------------------------*/
#pragma once

#include "TSphere.h"
#include "TShape.h"



class TLMaths::TShapeSphere2D : public TLMaths::TShape
{
public:
	TShapeSphere2D()															{}
	TShapeSphere2D(const TLMaths::TSphere2D& Sphere) : m_Shape ( Sphere )		{}
	TShapeSphere2D(const TLMaths::TBox2D& Box);									//	create sphere 2D from box

	static TRef						GetShapeType_Static()						{	return TLMaths_ShapeRef(TSphere2D);	}
	virtual TRef					GetShapeType() const						{	return TLMaths_ShapeRef(TSphere2D);	}
	virtual Bool					IsValid() const								{	return m_Shape.IsValid();	}
	virtual void					SetInvalid()								{	return m_Shape.SetInvalid();	}
	virtual float3					GetCenter() const							{	return m_Shape.GetPos();	}

	virtual void					Transform(const TLMaths::TTransform& Transform)	{	m_Shape.Transform( Transform );	}
	virtual TPtr<TShape>			Transform(const TLMaths::TTransform& Transform,TPtr<TShape>& pThis,TPtr<TShape>& pOldShape);

	virtual Bool					HasIntersection(TShapeBox2D& OtherShape);
	virtual Bool					GetIntersection(TShapeSphere2D& OtherShape,TIntersection& NodeAIntersection,TIntersection& NodeBIntersection);

	void							SetSphere(const TLMaths::TSphere2D& Sphere)	{	m_Shape = Sphere;	}
	const TLMaths::TSphere2D&		GetSphere() const							{	return m_Shape;	}

protected:
	virtual Bool					ImportData(TBinaryTree& Data);
	virtual Bool					ExportData(TBinaryTree& Data) const;

public:
	TLMaths::TSphere2D				m_Shape;
};




class TLMaths::TShapeSphere : public TLMaths::TShape
{
public:
	TShapeSphere()															{}
	TShapeSphere(const TLMaths::TSphere& Sphere) : m_Shape ( Sphere )		{}
	TShapeSphere(const TLMaths::TBox& Box);

	static TRef						GetShapeType_Static() 						{	return TLMaths_ShapeRef(TSphere);	}
	virtual TRef					GetShapeType() const						{	return TLMaths_ShapeRef(TSphere);	}
	virtual Bool					IsValid() const								{	return m_Shape.IsValid();	}
	virtual void					SetInvalid()								{	return m_Shape.SetInvalid();	}
	virtual float3					GetCenter() const							{	return m_Shape.GetPos();	}

	virtual void					Transform(const TLMaths::TTransform& Transform)	{	m_Shape.Transform( Transform );	}
	virtual TPtr<TShape>			Transform(const TLMaths::TTransform& Transform,TPtr<TShape>& pThis,TPtr<TShape>& pOldShape);

	virtual Bool					GetIntersection(TShapeSphere& OtherShape,TIntersection& NodeAIntersection,TIntersection& NodeBIntersection);
	virtual Bool					HasIntersection(TShapeBox2D& OtherShape);

	void							SetSphere(const TLMaths::TSphere& Sphere)	{	m_Shape = Sphere;	}
	const TLMaths::TSphere&			GetSphere() const							{	return m_Shape;	}

protected:
	virtual Bool					ImportData(TBinaryTree& Data);
	virtual Bool					ExportData(TBinaryTree& Data) const;

public:
	TLMaths::TSphere				m_Shape;
};



