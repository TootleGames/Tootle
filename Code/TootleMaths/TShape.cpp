#include "TShape.h"
#include <TootleCore/TString.h>


Bool TLMaths::TShape::HasIntersection(TShape& OtherShape)
{
	TRefRef OtherShapeType = OtherShape.GetShapeType();

	if ( OtherShapeType == TLMaths::TSphere2D::GetTypeRef() )
		return HasIntersection( static_cast<TShapeSphere2D&>(OtherShape) );
	
	if ( OtherShapeType == TLMaths::TSphere::GetTypeRef() )
		return HasIntersection( static_cast<TShapeSphere&>(OtherShape) );

	TLDebug_Break("unknown shape type");
	return FALSE;
}


Bool TLMaths::TShape::GetIntersection(TShape& OtherShape,TIntersection& Intersection)
{
	TRefRef OtherShapeType = OtherShape.GetShapeType();

	if ( OtherShapeType == TLMaths::TSphere2D::GetTypeRef() )
		return GetIntersection( static_cast<TShapeSphere2D&>(OtherShape), Intersection );

	if ( OtherShapeType == TLMaths::TSphere::GetTypeRef() )
		return GetIntersection( static_cast<TShapeSphere&>(OtherShape), Intersection );

	TLDebug_Break("unknown shape type");
	return FALSE;
}


Bool TLMaths::TShape::HasIntersection_Sphere2D(TShapeSphere2D& OtherShape)
{	
	Debug_BreakOverloadThis("HasIntersection", static_cast<TShape&>(OtherShape) );	
	return FALSE;	
}


Bool TLMaths::TShape::GetIntersection_Sphere2D(TShapeSphere2D& OtherShape,TIntersection& Intersection)	
{	
	Debug_BreakOverloadThis("GetIntersection", static_cast<TShape&>(OtherShape) );	
	return FALSE;	
}


void TLMaths::TShape::Debug_BreakOverloadThis(const char* pTestType,TShape& OtherShape)
{
	TRef ThisShapeType = this->GetShapeType();
	TRef OtherShapeType = OtherShape.GetShapeType();

	TTempString Debug_String;
	Debug_String.Appendf("TShape ");
	ThisShapeType.GetString( Debug_String );
	Debug_String.Append( pTestType );
	Debug_String.Append(" handling needs overloading for ");
	OtherShapeType.GetString( Debug_String );

	TLDebug_Break( Debug_String );
}

