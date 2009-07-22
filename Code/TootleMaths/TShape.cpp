#include "TShape.h"
#include "TShapeSphere.h"
#include "TShapeBox.h"
#include "TShapeCapsule.h"
#include "TShapePolygon.h"
#include <TootleCore/TString.h>
#include <TootleCore/TBinaryTree.h>



Bool TLMaths::TShape::GetIntersection(TShapeSphere& OtherShape,TIntersection& NodeAIntersection,TIntersection& NodeBIntersection)		{	Debug_BreakOverloadThis("GetIntersection", static_cast<TShape&>(OtherShape) );	return FALSE;	}
Bool TLMaths::TShape::GetIntersection(TShapeSphere2D& OtherShape,TIntersection& NodeAIntersection,TIntersection& NodeBIntersection)		{	Debug_BreakOverloadThis("GetIntersection", static_cast<TShape&>(OtherShape) );	return FALSE;	}
Bool TLMaths::TShape::GetIntersection(TShapeBox& OtherShape,TIntersection& NodeAIntersection,TIntersection& NodeBIntersection)			{	Debug_BreakOverloadThis("GetIntersection", static_cast<TShape&>(OtherShape) );	return FALSE;	}
Bool TLMaths::TShape::GetIntersection(TShapeBox2D& OtherShape,TIntersection& NodeAIntersection,TIntersection& NodeBIntersection)		{	Debug_BreakOverloadThis("GetIntersection", static_cast<TShape&>(OtherShape) );	return FALSE;	}
Bool TLMaths::TShape::GetIntersection(TShapePolygon2D& OtherShape,TIntersection& NodeAIntersection,TIntersection& NodeBIntersection)		{	Debug_BreakOverloadThis("GetIntersection", static_cast<TShape&>(OtherShape) );	return FALSE;	}
Bool TLMaths::TShape::GetIntersection(TShapeCapsule2D& OtherShape,TIntersection& NodeAIntersection,TIntersection& NodeBIntersection)	{	Debug_BreakOverloadThis("GetIntersection", static_cast<TShape&>(OtherShape) );	return FALSE;	}
//Bool TLMaths::TShape::GetIntersection(TShapeMesh& OtherShape,TIntersection& NodeAIntersection,TIntersection& NodeBIntersection)		{	Debug_BreakOverloadThis("GetIntersection", static_cast<TShape&>(OtherShape) );	return FALSE;	}


Bool TLMaths::TShape::GetIntersection(TLMaths::TLine& OtherShape,TIntersection& NodeAIntersection,TIntersection& NodeBIntersection) const
{
	TRef ThisShapeType = this->GetShapeType();
	TRef OtherShapeType = "line";

	TTempString Debug_String;
	Debug_String.Appendf("Need to overload TShape; ");
	ThisShapeType.GetString( Debug_String );
	Debug_String.Append("get intersection");
	Debug_String.Append("( ");
	OtherShapeType.GetString( Debug_String );
	Debug_String.Append(")");

	TLDebug_Break( Debug_String );
	return FALSE;
}




//---------------------------------------------------
//	create a shape type from TBinaryData
//---------------------------------------------------
TPtr<TLMaths::TShape> TLMaths::ImportShapeData(TBinaryTree& Data)
{
	//	read type
	TRef ShapeType;

	//	gr: new method, import - for backwards compatibility do a read if there is no "type" child
	if ( !Data.ImportData("Type", ShapeType ) )
	{
		if ( !Data.Read( ShapeType ) )
			return NULL;
	}

	//	create type
	TPtr<TLMaths::TShape> pShape = CreateShapeType( ShapeType );
	if ( !pShape )
		return NULL;

	//	import data into shape
	if ( !pShape->ImportData( Data ) )
	{
		#ifdef _DEBUG

		//	print out the data so we might be able to see what's wrong with it.
		Data.Debug_PrintTree();

		//	break
		TTempString Debug_String("Failed to import shape type ");
		ShapeType.GetString( Debug_String );
		Debug_String.Append(" from data ");
		Data.GetDataRef().GetString( Debug_String );
		TLDebug_Break( Debug_String );
		#endif
		return NULL;
	}

	return pShape;
}


//---------------------------------------------------
//	create a shape type from TBinaryData
//---------------------------------------------------
Bool TLMaths::ImportShapeData(TBinaryTree& Data,TLMaths::TShape& Shape)
{
	//	read type
	TRef ShapeType;

	//	gr: new method, import - for backwards compatibility do a read if there is no "type" child
	if ( !Data.ImportData("Type", ShapeType ) )
	{
		if ( !Data.Read( ShapeType ) )
			return NULL;
	}

	//	make sure shape type matches
	if ( ShapeType != Shape.GetShapeType() )
	{
		#ifdef _DEBUG
		TTempString Debug_String("Tried to import a ");
		ShapeType.GetString( Debug_String );
		Debug_String.Append(" shape from data \"");
		Data.GetDataRef().GetString( Debug_String );
		Debug_String.Append("\" into a ");
		Shape.GetShapeType().GetString( Debug_String );
		Debug_String.Append(" shape.");
		TLDebug_Break( Debug_String );
		#endif
		return FALSE;
	}

	//	import data into shape
	if ( !Shape.ImportData( Data ) )
		return FALSE;

	return TRUE;
}

//---------------------------------------------------
//	export shape to data
//---------------------------------------------------
Bool TLMaths::ExportShapeData(TBinaryTree& Data,const TLMaths::TShape& Shape,Bool WriteIfInvalid)
{
	//	dont write if shape is invalid
	if ( !WriteIfInvalid && !Shape.IsValid() )
		return FALSE;

	//	write type
	Data.ExportData("type", Shape.GetShapeType() );
	//Data.Write(  );

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
		CASE_CREATE_SHAPE( Sphere );
		CASE_CREATE_SHAPE( Sphere2D );
#undef CASE_CREATE_SHAPE

		//	gr: slight exception as the polygon shape doesn't begin with T as it's not a real primitive type
		case TLMaths_ShapeRef(Polygon2D):	return new TLMaths::TShapePolygon2D();	break;
	};

#ifdef _DEBUG
	TTempString Debug_String("Unhandled shape type ");
	ShapeType.GetString( Debug_String );
	TLDebug_Break( Debug_String );
#endif
	return NULL;
}





//-----------------------------------------------------
//	undo a transform applied to node A when we did the intersection test
//-----------------------------------------------------
void TLMaths::TIntersection::Transform(const TLMaths::TTransform& Transform)
{
	if ( Transform.HasAnyTransform() )
		Transform.Transform( m_Intersection );
}


//-----------------------------------------------------
//	undo a transform applied to node A when we did the intersection test	
//-----------------------------------------------------
void TLMaths::TIntersection::Untransform(const TLMaths::TTransform& Transform)
{
	if ( Transform.HasAnyTransform() )
		Transform.Untransform( m_Intersection );
}



Bool TLMaths::TShape::HasIntersection(TShape& OtherShape)
{
	TRef OtherShapeType = OtherShape.GetShapeType();

	//	do shape specific functions
	switch ( OtherShapeType.GetData() )
	{
		case TLMaths_ShapeRef(TSphere):		return HasIntersection( static_cast<TShapeSphere&>(OtherShape) );
		case TLMaths_ShapeRef(TSphere2D):	return HasIntersection( static_cast<TShapeSphere2D&>(OtherShape) );
		case TLMaths_ShapeRef(TBox):		return HasIntersection( static_cast<TShapeBox&>(OtherShape) );
		case TLMaths_ShapeRef(TBox2D):		return HasIntersection( static_cast<TShapeBox2D&>(OtherShape) );
		case TLMaths_ShapeRef(TCapsule2D):	return HasIntersection( static_cast<TShapeCapsule2D&>(OtherShape) );
//		case TLMaths_ShapeRef(Mesh):		return HasIntersection( static_cast<TShapeMesh&>(OtherShape) );
		case TLMaths_ShapeRef(Polygon2D):	return HasIntersection( static_cast<TShapePolygon2D&>(OtherShape) );
	};

#ifdef _DEBUG
	TTempString Debug_String("HasIntersection: Unhandled shape type ");
	OtherShapeType.GetString( Debug_String );
	TLDebug_Break( Debug_String );
#endif

	return FALSE;
}


Bool TLMaths::TShape::GetIntersection(TShape& OtherShape,TIntersection& NodeAIntersection,TIntersection& NodeBIntersection)
{
	TRef OtherShapeType = OtherShape.GetShapeType();

	//	do shape specific functions
	switch ( OtherShapeType.GetData() )
	{
		case TLMaths_ShapeRef(TSphere):		return GetIntersection( static_cast<TShapeSphere&>(OtherShape), NodeAIntersection, NodeBIntersection );
		case TLMaths_ShapeRef(TSphere2D):	return GetIntersection( static_cast<TShapeSphere2D&>(OtherShape), NodeAIntersection, NodeBIntersection );
		case TLMaths_ShapeRef(TBox):		return GetIntersection( static_cast<TShapeBox&>(OtherShape), NodeAIntersection, NodeBIntersection );
		case TLMaths_ShapeRef(TBox2D):		return GetIntersection( static_cast<TShapeBox2D&>(OtherShape), NodeAIntersection, NodeBIntersection );
		case TLMaths_ShapeRef(TCapsule2D):	return GetIntersection( static_cast<TShapeCapsule2D&>(OtherShape), NodeAIntersection, NodeBIntersection );
//		case TLMaths_ShapeRef(Mesh):		return GetIntersection( static_cast<TShapeMesh&>(OtherShape), NodeAIntersection, NodeBIntersection );
		case TLMaths_ShapeRef(Polygon2D):	return GetIntersection( static_cast<TShapePolygon2D&>(OtherShape), NodeAIntersection, NodeBIntersection );
	};

#ifdef _DEBUG
	TTempString Debug_String("GetIntersection: Unhandled shape type ");
	OtherShapeType.GetString( Debug_String );
	TLDebug_Break( Debug_String );
#endif

	return FALSE;
}


void TLMaths::TShape::Debug_BreakOverloadThis(const char* pTestType,TShape& OtherShape)
{
	TRef ThisShapeType = this->GetShapeType();
	TRef OtherShapeType = OtherShape.GetShapeType();

	TTempString Debug_String;
	Debug_String.Appendf("Need to overload TShape; ");
	ThisShapeType.GetString( Debug_String );
	Debug_String.Append( pTestType );
	Debug_String.Append("( ");
	OtherShapeType.GetString( Debug_String );
	Debug_String.Append(")");

	TLDebug_Break( Debug_String );
}










TPtr<TLMaths::TShape> TLMaths::TShapeLine2D::Transform(const TLMaths::TTransform& Transform,TPtr<TShape>& pOldShape,Bool KeepShape) const
{
	if ( !IsValid() )
		return NULL;

	//	copy and transform box
	TLMaths::TLine2D NewLine( m_Shape );
	NewLine.Transform( Transform );

	//	re-use old shape
	if ( pOldShape && pOldShape.GetObject() != this && pOldShape->GetShapeType() == TLMaths::TBox::GetTypeRef() )
	{
		pOldShape.GetObject<TShapeLine2D>()->SetLine( NewLine );
		return pOldShape;
	}

	return new TShapeLine2D( NewLine );
}




Bool TLMaths::TShapeLine2D::ImportData(TBinaryTree& Data)
{
	if ( !Data.ImportData("Start", m_Shape.GetStart() ) )	return FALSE;
	if ( !Data.ImportData("End", m_Shape.GetEnd() ) )		return FALSE;

	return TRUE;
}

Bool TLMaths::TShapeLine2D::ExportData(TBinaryTree& Data) const
{
	Data.ExportData("Start", m_Shape.GetStart() );
	Data.ExportData("End", m_Shape.GetEnd() );
	return TRUE;
}



