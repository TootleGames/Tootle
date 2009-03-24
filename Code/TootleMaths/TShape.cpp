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



//---------------------------------------
//	create sphere 2D from box
//---------------------------------------
TLMaths::TShapeSphere2D::TShapeSphere2D(const TLMaths::TBox2D& Box)
{
	//	work out radius (NOT the diagonal!)
	float HalfWidth = Box.GetHalfWidth();
	float HalfHeight = Box.GetHalfHeight();
	float Radius = (HalfWidth>HalfHeight) ? HalfWidth : HalfHeight;

	//	make up sphere and shape
	m_Sphere.Set( Box.GetCenter(), Radius );
}


//---------------------------------------
//	create sphere from box
//---------------------------------------
TLMaths::TShapeSphere::TShapeSphere(const TLMaths::TBox& Box)
{
	//	work out radius (NOT the diagonal!)
	float3 HalfSize = Box.GetSize() * 0.5f;
	float Radius = HalfSize.x;
	if ( HalfSize.y > Radius )	Radius = HalfSize.y;
	if ( HalfSize.z > Radius )	Radius = HalfSize.z;

	//	make up sphere and shape
	m_Sphere.Set( Box.GetCenter(), Radius );
}


//---------------------------------------
//	create sphere from box
//---------------------------------------
TLMaths::TShapeCapsule2D::TShapeCapsule2D(const TLMaths::TBox2D& Box)
{
	m_Capsule.Set( Box );
}
