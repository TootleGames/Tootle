/*------------------------------------------------------

	kind of TShape

-------------------------------------------------------*/
#pragma once

#include "TBox.h"
#include "TShape.h"




class TLMaths::TShapeBox : public TLMaths::TShape
{
public:
	TShapeBox()															{}
	TShapeBox(const TLMaths::TBox& Box) : m_Shape ( Box )					{}

	static TRef						GetShapeType_Static()						{	return TLMaths_ShapeRef(TBox);	}
	virtual TRef					GetShapeType() const						{	return TLMaths_ShapeRef(TBox);	}
	virtual Bool					IsValid() const								{	return m_Shape.IsValid();	}
	virtual void					SetInvalid()								{	return m_Shape.SetInvalid();	}
	virtual float3					GetCenter() const							{	return m_Shape.GetCenter();	}

	virtual void					Transform(const TLMaths::TTransform& Transform)	{	m_Shape.Transform( Transform );	}
	virtual TPtr<TShape>			Transform(const TLMaths::TTransform& Transform,TPtr<TShape>& pThis,TPtr<TShape>& pOldShape);
	
	virtual Bool					HasIntersection(TShapeSphere& OtherShape);

	const TLMaths::TBox&			GetBox() const								{	return m_Shape;	}
	void							SetBox(const TLMaths::TBox& Box)			{	m_Shape = Box;	}


protected:
	virtual Bool					ImportData(TBinaryTree& Data);
	virtual Bool					ExportData(TBinaryTree& Data) const;

public:
	TLMaths::TBox					m_Shape;
};


class TLMaths::TShapeBox2D : public TLMaths::TShape
{
public:
	TShapeBox2D()															{}
	TShapeBox2D(const TLMaths::TBox2D& Box) : m_Shape ( Box )				{}

	static TRef						GetShapeType_Static()						{	return TLMaths_ShapeRef(TBox2D);	}
	virtual TRef					GetShapeType() const						{	return TLMaths_ShapeRef(TBox2D);	}
	virtual Bool					IsValid() const								{	return m_Shape.IsValid();	}
	virtual void					SetInvalid()								{	return m_Shape.SetInvalid();	}
	virtual float3					GetCenter() const							{	return m_Shape.GetCenter();	}

	virtual void					Transform(const TLMaths::TTransform& Transform)	{	m_Shape.Transform( Transform );	}
	virtual TPtr<TShape>			Transform(const TLMaths::TTransform& Transform,TPtr<TShape>& pThis,TPtr<TShape>& pOldShape);

	virtual Bool					HasIntersection(TShapeSphere& OtherShape);
	virtual Bool					HasIntersection(TShapeSphere2D& OtherShape);
	virtual Bool					HasIntersection(TShapeBox2D& OtherShape);
	virtual Bool					HasIntersection(TShapeCapsule2D& OtherShape);

	const TLMaths::TBox2D&			GetBox() const								{	return m_Shape;	}
	void							SetBox(const TLMaths::TBox2D& Box)			{	m_Shape = Box;	}

protected:
	virtual Bool					ImportData(TBinaryTree& Data);
	virtual Bool					ExportData(TBinaryTree& Data) const;
	
public:
	TLMaths::TBox2D					m_Shape;
};

