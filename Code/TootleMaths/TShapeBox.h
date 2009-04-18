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
	TShapeBox(const TLMaths::TBox& Box) : m_Box ( Box )					{}

	static TRef						GetShapeType_Static()						{	return TLMaths::TBox::GetTypeRef();	}
	virtual TRef					GetShapeType() const						{	return GetShapeType_Static();	}
	virtual Bool					IsValid() const								{	return GetBox().IsValid();	}
	virtual float3					GetCenter() const							{	return GetBox().GetCenter();	}
	
	const TLMaths::TBox&			GetBox() const								{	return m_Box;	}
	void							SetBox(const TLMaths::TBox& Box)			{	m_Box = Box;	}

	virtual Bool					HasIntersection(TShapeSphere& OtherShape);

protected:
	virtual Bool					ImportData(TBinaryTree& Data);
	virtual Bool					ExportData(TBinaryTree& Data) const;
	virtual TPtr<TShape>			Transform(const TLMaths::TTransform& Transform,TPtr<TShape>& pThis,TPtr<TShape>& pOldShape);

protected:
	TLMaths::TBox					m_Box;
};


class TLMaths::TShapeBox2D : public TLMaths::TShape
{
public:
	TShapeBox2D()															{}
	TShapeBox2D(const TLMaths::TBox2D& Box) : m_Box ( Box )					{}

	static TRef						GetShapeType_Static()						{	return TLMaths::TBox2D::GetTypeRef();	}
	virtual TRef					GetShapeType() const						{	return GetShapeType_Static();	}
	virtual Bool					IsValid() const								{	return GetBox().IsValid();	}
	virtual float3					GetCenter() const							{	return GetBox().GetCenter();	}
	
	virtual Bool					HasIntersection(TShapeSphere& OtherShape);
	virtual Bool					HasIntersection(TShapeSphere2D& OtherShape);
	virtual Bool					HasIntersection(TShapeBox2D& OtherShape);
	virtual Bool					HasIntersection(TShapeCapsule2D& OtherShape);

	const TLMaths::TBox2D&			GetBox() const								{	return m_Box;	}
	void							SetBox(const TLMaths::TBox2D& Box)			{	m_Box = Box;	}

protected:
	virtual Bool					ImportData(TBinaryTree& Data);
	virtual Bool					ExportData(TBinaryTree& Data) const;
	virtual TPtr<TShape>			Transform(const TLMaths::TTransform& Transform,TPtr<TShape>& pThis,TPtr<TShape>& pOldShape);
	
protected:
	TLMaths::TBox2D					m_Box;
};

