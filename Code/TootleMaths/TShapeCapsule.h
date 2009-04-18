/*------------------------------------------------------

	kind of TShape

-------------------------------------------------------*/
#pragma once

#include "TCapsule.h"
#include "TShape.h"



class TLMaths::TShapeCapsule2D : public TLMaths::TShape
{
public:
	TShapeCapsule2D()															{}
	TShapeCapsule2D(const TLMaths::TCapsule2D& Capsule) : m_Capsule ( Capsule )	{}
	TShapeCapsule2D(const TLMaths::TBox2D& Box);

	static TRef						GetShapeType_Static()						{	return TLMaths::TCapsule2D::GetTypeRef();	}
	virtual TRef					GetShapeType() const						{	return GetShapeType_Static();	}
	virtual Bool					IsValid() const								{	return GetCapsule().IsValid();	}
	virtual float3					GetCenter() const							{	return GetCapsule().GetCenter();	}
	
	virtual Bool					GetIntersection(TShapeCapsule2D& OtherShape,TIntersection& NodeAIntersection,TIntersection& NodeBIntersection);
	
	const TLMaths::TCapsule2D&		GetCapsule() const							{	return m_Capsule;	}
	void							SetCapsule(const TLMaths::TCapsule2D& Capsule)	{	m_Capsule = Capsule;	}
	

protected:
	virtual Bool					ImportData(TBinaryTree& Data);
	virtual Bool					ExportData(TBinaryTree& Data) const;
	virtual TPtr<TShape>			Transform(const TLMaths::TTransform& Transform,TPtr<TShape>& pThis,TPtr<TShape>& pOldShape);

protected:
	TLMaths::TCapsule2D				m_Capsule;
};

