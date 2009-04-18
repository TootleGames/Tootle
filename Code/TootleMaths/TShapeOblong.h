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
	TShapeOblong2D(const TLMaths::TOblong2D& Oblong) : m_Oblong ( Oblong )		{}

	static TRef						GetShapeType_Static()						{	return TLMaths::TOblong2D::GetTypeRef();	}
	virtual TRef					GetShapeType() const						{	return GetShapeType_Static();	}
	virtual Bool					IsValid() const								{	return GetOblong().IsValid();	}
	virtual float3					GetCenter() const							{	return GetOblong().GetCenter();	}
	
	void							SetOblong(const TLMaths::TOblong2D& Oblong)	{	m_Oblong = Oblong;	}
	const TLMaths::TOblong2D&		GetOblong() const							{	return m_Oblong;	}

protected:
	virtual Bool					ImportData(TBinaryTree& Data);
	virtual Bool					ExportData(TBinaryTree& Data) const;

protected:
	TLMaths::TOblong2D				m_Oblong;
};

