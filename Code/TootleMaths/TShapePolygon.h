/*------------------------------------------------------

	kind of TShape

-------------------------------------------------------*/
#pragma once

#include "TShape.h"




//-----------------------------------------------------
//	outline of a polygon. Outline is not closed and is always clockwise (CleanContour ensures these things)
//	todo; move contour related code and outline stuff above into a simple math class (out of the TShape)
//-----------------------------------------------------
class TLMaths::TShapePolygon2D : public TLMaths::TShape
{
public:
	TShapePolygon2D()																{}
	TShapePolygon2D(const TArray<float2>& Outline)									{	m_Outline.Copy( Outline );	CleanContour();	}
	TShapePolygon2D(const TArray<float3>& Outline);

	static TRef						GetShapeType_Static()							{	return TLMaths_ShapeRef(Polygon2D);	}
	virtual TRef					GetShapeType() const							{	return TLMaths_ShapeRef(Polygon2D);	}
	virtual Bool					IsValid() const									{	return (m_Outline.GetSize() >= 3);	}
	virtual void					SetInvalid()									{	m_Outline.SetSize(0);	}
	virtual float3					GetCenter() const;
	
	const TArray<float2>&			GetOutline() const								{	return m_Outline;	}
	void							SetOutline(const TArray<float2>& Outline,Bool DoCleanContour=TRUE)		{	m_Outline = Outline;	if ( DoCleanContour )	CleanContour();	}

	Bool							IsClockwise() const;							//	check that the shape is/isn't clockwise
	FORCEINLINE Bool				SetClockwise(Bool Clockwise=TRUE)				{	if ( Clockwise == IsClockwise() )	return FALSE;	ReverseContour();	return TRUE;	}
	void							ReverseContour();								//	reverse contour order
	void							CleanContour();									//	checks for overlapping points, tiny edges, and reverses outline if it's anti clockwise

	virtual void					Transform(const TLMaths::TTransform& Transform)	{	Transform.Transform( m_Outline );	CleanContour();	}
	virtual TPtr<TShape>			Transform(const TLMaths::TTransform& Transform,TPtr<TShape>& pOldShape,Bool KeepShape=FALSE) const;

	Bool							Debug_CheckIsConvex() const;					//	debug check that matches box2d's convex polygon check

protected:
	virtual Bool					ImportData(TBinaryTree& Data);
	virtual Bool					ExportData(TBinaryTree& Data) const;
	
protected:
	TArray<float2>					m_Outline;
};

