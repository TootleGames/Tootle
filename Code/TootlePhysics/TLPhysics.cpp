#include "TLPhysics.h"
#include "TPhysicsNode.h"
#include <TootleMaths/TShape.h>
#include <TootleMaths/TShapeBox.h>
#include <TootleMaths/TShapeSphere.h>

//	namespace Box2D
#include <box2d/include/box2d.h>


//------------------------------------------------------
//	get a box2D polygon [definition] shape from a tootle shape
//------------------------------------------------------
Bool TLPhysics::GetPolygonDefFromShape(b2PolygonDef& PolygonDef,const TLMaths::TShape& Shape)
{
	switch ( Shape.GetShapeType().GetData() )
	{
		case TLMaths_ShapeRef(TBox2D):
		{
			const TLMaths::TBox2D& ShapeBox = static_cast<const TLMaths::TShapeBox2D&>( Shape ).GetBox();
			float2 ShapeBoxCenter = ShapeBox.GetCenter();
			PolygonDef.SetAsBox( ShapeBox.GetHalfWidth(), ShapeBox.GetHalfHeight(), b2Vec2( ShapeBoxCenter.x, ShapeBoxCenter.y ), 0.f );
			return TRUE;
		}

	};

	return FALSE;
}


//------------------------------------------------------
//	get a box2D polygon [definition] shape from a tootle shape
//------------------------------------------------------
Bool TLPhysics::GetCircleDefFromShape(b2CircleDef& PolygonDef,const TLMaths::TShape& Shape)
{
	switch ( Shape.GetShapeType().GetData() )
	{
		case TLMaths_ShapeRef(TSphere2D):
		{
			const TLMaths::TSphere2D& ShapeSphere = static_cast<const TLMaths::TShapeSphere2D&>( Shape ).GetSphere();
			PolygonDef.localPosition = b2Vec2( ShapeSphere.GetPos().x, ShapeSphere.GetPos().y );
			PolygonDef.radius = ShapeSphere.GetRadius();
			return TRUE;
		}

		case TLMaths_ShapeRef(TSphere):
		{
			const TLMaths::TSphere& ShapeSphere = static_cast<const TLMaths::TShapeSphere&>( Shape ).GetSphere();
			PolygonDef.localPosition = b2Vec2( ShapeSphere.GetPos().x, ShapeSphere.GetPos().y );
			PolygonDef.radius = ShapeSphere.GetRadius();
			return TRUE;
		}

	};

	return FALSE;
}

//------------------------------------------------------
//	set collision info against this node
//------------------------------------------------------
void TLPhysics::TCollisionInfo::Set(const TLPhysics::TPhysicsNode& OtherNode,const TLMaths::TIntersection& Intersection)
{
	m_OtherNode = OtherNode.GetNodeRef();
	m_OtherNodeOwner = OtherNode.GetOwnerSceneNodeRef();
	m_OtherNodeStatic = OtherNode.IsStatic();

	m_Intersection = Intersection.m_Intersection;
}



//------------------------------------------------------
//	export this collision info into a BinaryData
//------------------------------------------------------
void TLPhysics::TCollisionInfo::ExportData(TBinaryTree& Data)
{
	Data.Write( m_OtherNode );
	Data.Write( m_OtherNodeOwner );
	Data.Write( m_OtherNodeStatic );
	Data.Write( m_Intersection );
}


//------------------------------------------------------
//	get collision info from a BinaryData
//------------------------------------------------------
Bool TLPhysics::TCollisionInfo::ImportData(TBinaryTree& Data)
{
	Data.ResetReadPos();

	if ( !Data.Read( m_OtherNode ) )		return FALSE;
	if ( !Data.Read( m_OtherNodeOwner ) )	return FALSE;
	if ( !Data.Read( m_OtherNodeStatic ) )	return FALSE;
	if ( !Data.Read( m_Intersection ) )		return FALSE;

	return TRUE;
}


