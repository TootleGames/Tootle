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
	TShapeSphere2D(const TLMaths::TSphere2D& Sphere) : m_Sphere ( Sphere )		{}
	TShapeSphere2D(const TLMaths::TBox2D& Box);									//	create sphere 2D from box

	static TRef						GetShapeType_Static()						{	return GetShapeType_Static();	}
	virtual TRef					GetShapeType() const						{	return TLMaths::TSphere2D::GetTypeRef();	}
	virtual Bool					IsValid() const								{	return GetSphere().IsValid();	}
	virtual float3					GetCenter() const							{	return GetSphere().GetPos();	}
	
	virtual TPtr<TShape>			Transform(const TLMaths::TTransform& Transform,TPtr<TShape>& pThis,TPtr<TShape>& pOldShape);

	void							SetSphere(const TLMaths::TSphere2D& Sphere)	{	m_Sphere = Sphere;	}
	const TLMaths::TSphere2D&		GetSphere() const							{	return m_Sphere;	}

	virtual Bool					HasIntersection(TShapeBox2D& OtherShape);
	virtual Bool					GetIntersection(TShapeSphere2D& OtherShape,TIntersection& NodeAIntersection,TIntersection& NodeBIntersection);

protected:
	virtual Bool					ImportData(TBinaryTree& Data);
	virtual Bool					ExportData(TBinaryTree& Data) const;

protected:
	TLMaths::TSphere2D				m_Sphere;			//	sphere collision object
};




class TLMaths::TShapeSphere : public TLMaths::TShape
{
public:
	TShapeSphere()															{}
	TShapeSphere(const TLMaths::TSphere& Sphere) : m_Sphere ( Sphere )		{}
	TShapeSphere(const TLMaths::TBox& Box);

	static TRef						GetShapeType_Static() 						{	return TLMaths::TSphere::GetTypeRef();	}
	virtual TRef					GetShapeType() const						{	return GetShapeType_Static();	}
	virtual Bool					IsValid() const								{	return GetSphere().IsValid();	}
	virtual float3					GetCenter() const							{	return GetSphere().GetPos();	}

	virtual TPtr<TShape>			Transform(const TLMaths::TTransform& Transform,TPtr<TShape>& pThis,TPtr<TShape>& pOldShape);
	virtual Bool					GetIntersection(TShapeSphere& OtherShape,TIntersection& NodeAIntersection,TIntersection& NodeBIntersection);
	
	void							SetSphere(const TLMaths::TSphere& Sphere)	{	m_Sphere = Sphere;	}
	const TLMaths::TSphere&			GetSphere() const							{	return m_Sphere;	}

	virtual Bool					HasIntersection(TShapeBox2D& OtherShape);

protected:
	virtual Bool					ImportData(TBinaryTree& Data);
	virtual Bool					ExportData(TBinaryTree& Data) const;

protected:
	TLMaths::TSphere				m_Sphere;			//	sphere collision object
};



