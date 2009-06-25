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
	TShapeCapsule2D(const TLMaths::TCapsule2D& Capsule) : m_Shape ( Capsule )	{}
	TShapeCapsule2D(const TLMaths::TBox2D& Box);

	static TRef						GetShapeType_Static()						{	return TLMaths_ShapeRef(TCapsule2D);	}
	virtual TRef					GetShapeType() const						{	return TLMaths_ShapeRef(TCapsule2D);	}
	virtual Bool					IsValid() const								{	return m_Shape.IsValid();	}
	virtual void					SetInvalid()								{	m_Shape.SetInvalid();	}
	virtual float3					GetCenter() const							{	return m_Shape.GetCenter();	}
	
	virtual Bool					GetIntersection(TShapeCapsule2D& OtherShape,TIntersection& NodeAIntersection,TIntersection& NodeBIntersection);
	
	const TLMaths::TCapsule2D&		GetCapsule() const								{	return m_Shape;	}
	void							SetCapsule(const TLMaths::TCapsule2D& Capsule)	{	m_Shape = Capsule;	}
	

protected:
	virtual Bool					ImportData(TBinaryTree& Data);
	virtual Bool					ExportData(TBinaryTree& Data) const;
	virtual TPtr<TShape>			Transform(const TLMaths::TTransform& Transform,TPtr<TShape>& pOldShape) const;

public:
	TLMaths::TCapsule2D				m_Shape;
};

