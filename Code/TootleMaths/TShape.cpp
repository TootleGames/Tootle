#include "TShape.h"
#include <TootleCore/TString.h>
#include <TootleCore/TBinaryTree.h>



//---------------------------------------------------
//	create a shape type from TBinaryData
//---------------------------------------------------
TPtr<TLMaths::TShape> TLMaths::ImportShapeData(TBinaryTree& Data)
{
	//	read type
	TRef ShapeType;
	if ( !Data.Read( ShapeType ) )
		return NULL;

	//	create type
	TPtr<TLMaths::TShape> pShape = CreateShapeType( ShapeType );
	if ( !pShape )
		return NULL;

	//	import data into shape
	if ( !pShape->ImportData( Data ) )
		return NULL;

	return pShape;
}


//---------------------------------------------------
//	export shape to data
//---------------------------------------------------
Bool TLMaths::ExportShapeData(TBinaryTree& Data,const TLMaths::TShape& Shape)
{
	//	write type
	Data.Write( Shape.GetShapeType() );

	//	write data
	if ( !Shape.ExportData( Data ) )
		return FALSE;
	
	return TRUE;
}


//---------------------------------------------------
//	create shape instance from ref
//---------------------------------------------------
TPtr<TLMaths::TShape> TLMaths::CreateShapeType(TRefRef ShapeType)
{
	switch ( ShapeType.GetData() )
	{
#define CASE_CREATE_SHAPE(TYPE)	case TLMaths_ShapeRef(T##TYPE):	return new TLMaths::TShape##TYPE();	break
		CASE_CREATE_SHAPE( Box );
		CASE_CREATE_SHAPE( Box2D );
//		CASE_CREATE_SHAPE( Capsule );
		CASE_CREATE_SHAPE( Capsule2D );
//		CASE_CREATE_SHAPE( Line );
//		CASE_CREATE_SHAPE( Line2D );
//		CASE_CREATE_SHAPE( Oblong );
		CASE_CREATE_SHAPE( Oblong2D );
		CASE_CREATE_SHAPE( Sphere );
		CASE_CREATE_SHAPE( Sphere2D );
#undef CASE_CREATE_SHAPE
	};

#ifdef _DEBUG
	TTempString Debug_String("Unhandled shape type ");
	ShapeType.GetString( Debug_String );
	TLDebug_Break( Debug_String );
#endif
	return NULL;
}




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
//	
//---------------------------------------
Bool TLMaths::TShapeSphere2D::ImportData(TBinaryTree& Data)
{
	if ( !Data.ImportData("Pos", m_Sphere.GetPos() ) )		return FALSE;
	if ( !Data.ImportData("Radius", m_Sphere.GetRadius() ) )	return FALSE;

	return TRUE;
}

//---------------------------------------
//	
//---------------------------------------
Bool TLMaths::TShapeSphere2D::ExportData(TBinaryTree& Data) const
{
	Data.ExportData("Pos", m_Sphere.GetPos() );
	Data.ExportData("Radius", m_Sphere.GetRadius() );

	return TRUE;
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
//	
//---------------------------------------
Bool TLMaths::TShapeSphere::ImportData(TBinaryTree& Data)
{
	if ( !Data.ImportData("Pos", m_Sphere.GetPos() ) )		return FALSE;
	if ( !Data.ImportData("Radius", m_Sphere.GetRadius() ) )	return FALSE;

	return TRUE;
}

//---------------------------------------
//	
//---------------------------------------
Bool TLMaths::TShapeSphere::ExportData(TBinaryTree& Data) const
{
	Data.ExportData("Pos", m_Sphere.GetPos() );
	Data.ExportData("Radius", m_Sphere.GetRadius() );

	return TRUE;
}


//---------------------------------------
//	create sphere from box
//---------------------------------------
TLMaths::TShapeCapsule2D::TShapeCapsule2D(const TLMaths::TBox2D& Box)
{
	m_Capsule.Set( Box );
}


//---------------------------------------
//	
//---------------------------------------
Bool TLMaths::TShapeCapsule2D::ImportData(TBinaryTree& Data)
{
	if ( !Data.ImportData("Start", m_Capsule.GetLine().GetStart() ) )	return FALSE;
	if ( !Data.ImportData("End", m_Capsule.GetLine().GetEnd() ) )		return FALSE;
	if ( !Data.ImportData("Radius", m_Capsule.GetRadius() ) )	return FALSE;

	return TRUE;
}

//---------------------------------------
//	
//---------------------------------------
Bool TLMaths::TShapeCapsule2D::ExportData(TBinaryTree& Data) const
{
	Data.ExportData("Start", m_Capsule.GetLine().GetStart() );
	Data.ExportData("End", m_Capsule.GetLine().GetEnd() );
	Data.ExportData("Radius", m_Capsule.GetRadius() );

	return TRUE;
}


//---------------------------------------
//	
//---------------------------------------
Bool TLMaths::TShapeBox2D::ImportData(TBinaryTree& Data)
{
	if ( !Data.ImportData("Min", m_Box.GetMin() ) )		return FALSE;
	if ( !Data.ImportData("Max", m_Box.GetMax() ) )		return FALSE;
	if ( !Data.ImportData("Valid", m_Box.IsValid() ) )	return FALSE;

	return TRUE;
}

//---------------------------------------
//	
//---------------------------------------
Bool TLMaths::TShapeBox2D::ExportData(TBinaryTree& Data) const
{
	Data.ExportData("Min", m_Box.GetMin() );
	Data.ExportData("Max", m_Box.GetMax() );
	Data.ExportData("Valid", m_Box.IsValid() );

	return TRUE;
}


//---------------------------------------
//	
//---------------------------------------
Bool TLMaths::TShapeBox::ImportData(TBinaryTree& Data)
{
	if ( !Data.ImportData("Min", m_Box.GetMin() ) )		return FALSE;
	if ( !Data.ImportData("Max", m_Box.GetMax() ) )		return FALSE;
	if ( !Data.ImportData("Valid", m_Box.IsValid() ) )	return FALSE;

	return TRUE;
}

//---------------------------------------
//	
//---------------------------------------
Bool TLMaths::TShapeBox::ExportData(TBinaryTree& Data) const
{
	Data.ExportData("Min", m_Box.GetMin() );
	Data.ExportData("Max", m_Box.GetMax() );
	Data.ExportData("Valid", m_Box.IsValid() );

	return TRUE;
}


//---------------------------------------
//	
//---------------------------------------
Bool TLMaths::TShapeOblong2D::ImportData(TBinaryTree& Data)
{
	if ( !Data.ImportArrays("Corners", m_Oblong.GetBoxCorners() ) )		return FALSE;
	if ( !Data.ImportData("Valid", m_Oblong.IsValid() ) )					return FALSE;

	return TRUE;
}

//---------------------------------------
//	
//---------------------------------------
Bool TLMaths::TShapeOblong2D::ExportData(TBinaryTree& Data) const
{
	Data.ExportArray("Corners", m_Oblong.GetBoxCorners() );
	Data.ExportData("Valid", m_Oblong.IsValid() );

	return TRUE;
}
