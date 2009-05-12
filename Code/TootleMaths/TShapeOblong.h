/*------------------------------------------------------

	kind of TShape

-------------------------------------------------------*/
#pragma once

#include "TOblong.h"
#include "TShape.h"



class TLMaths::TShapeOblong2D : public TLMaths::TShape
{
public:
	TShapeOblong2D()															{}
	TShapeOblong2D(const TLMaths::TOblong2D& Oblong) : m_Shape ( Oblong )		{}

	static TRef						GetShapeType_Static()						{	return TLMaths_ShapeRef(TOblong2D);	}
	virtual TRef					GetShapeType() const						{	return TLMaths_ShapeRef(TOblong2D);	}
	virtual Bool					IsValid() const								{	return m_Shape.IsValid();	}
	virtual void					SetInvalid()								{	return m_Shape.SetInvalid();	}
	virtual float3					GetCenter() const							{	return m_Shape.GetCenter();	}
	
	void							SetOblong(const TLMaths::TOblong2D& Oblong)	{	m_Shape = Oblong;	}
	const TLMaths::TOblong2D&		GetOblong() const							{	return m_Shape;	}

	virtual Bool					HasIntersection(TShapeBox2D& OtherShape);

protected:
	virtual Bool					ImportData(TBinaryTree& Data);
	virtual Bool					ExportData(TBinaryTree& Data) const;

public:
	TLMaths::TOblong2D				m_Shape;
};

