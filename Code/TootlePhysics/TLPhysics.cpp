#include "TLPhysics.h"
#include "TPhysicsNode.h"
#include <TootleMaths/TShape.h>
#include <TootleMaths/TShapeBox.h>
#include <TootleMaths/TShapeSphere.h>
#include <TootleMaths/TShapePolygon.h>
#include <TootleMaths/TShapeOblong.h>

//	namespace Box2D
#include <box2d/include/box2d.h>


//------------------------------------------------------
//	create a transformed shape from a body shape
//------------------------------------------------------
TPtr<TLMaths::TShape> TLPhysics::GetShapeFromBodyShape(b2Shape& BodyShape,const TLMaths::TTransform& Transform)
{
	b2Body& Body = *BodyShape.GetBody();

	if ( BodyShape.GetType() == e_polygonShape )
	{
		b2PolygonShape& PolyShape = (b2PolygonShape&)BodyShape;
		const b2XForm& Bodyxf = Body.GetXForm();

		//	get a list of the points
		TFixedArray<float2,100> Points;
		const b2Vec2* pShapeVerts = PolyShape.GetVertices();
		for ( s32 p=0;	p<PolyShape.GetVertexCount();	p++ )
		{
			//	transform by bodys transform
			b2Vec2 WorldPos = b2Mul( Bodyxf, pShapeVerts[p] );

			//	gr: transform by ourtransform for scale? or instead of the box2d one? as box2d lacks scale
			float2 WorldPos2( WorldPos.x, WorldPos.y );

			Points.Add( WorldPos2 );
		}

		//	create shape
		return new TLMaths::TShapePolygon2D( Points );
	}
	else if ( BodyShape.GetType() == e_circleShape )
	{
		TLDebug_Break("todo");
	}

	TLDebug_Break("Invalid body shape");
	return NULL;
}



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

		case TLMaths_ShapeRef(TOblong2D):
		{
			const TLMaths::TOblong2D& ShapeBox = static_cast<const TLMaths::TShapeOblong2D&>( Shape ).GetOblong();
			
			//	turn into polygon shape
			const TFixedArray<float2,4>& Corners = ShapeBox.GetBoxCorners();
		
			//	set shape vertexes
			PolygonDef.vertexCount = Corners.GetSize();
			for ( u32 i=0;	i<Corners.GetSize();	i++ )
				PolygonDef.vertices[i].Set( Corners[i].x, Corners[i].y );

			return TRUE;
		}

		case TLMaths_ShapeRef(Polygon2D):
		{
			const TLMaths::TShapePolygon2D& ShapePolygon = static_cast<const TLMaths::TShapePolygon2D&>( Shape );
			const TArray<float2>& Outline = ShapePolygon.GetOutline();

			//	if is not clockwise then create a new reversed (clockwise) shape to generate from
			if ( !ShapePolygon.IsClockwise() )
			{
				TLMaths::TShapePolygon2D ReversedShapePolygon( Outline );
				
				ReversedShapePolygon.ReverseContour();

				return GetPolygonDefFromShape( PolygonDef, ReversedShapePolygon );
			}

			//	check validity of polygon for use in box2d first
			if ( !ShapePolygon.Debug_CheckIsConvex() )
			{
				if ( !TLDebug_Break("Polygon shape is concave and needs to be convex for box2d. If your shape cannot be fixed then we can implement a tesselation system that then creates multiple shapes on a body...") )
					return FALSE;
			}

			//	gr: limit this at the shape level?
			if ( Outline.GetSize() > b2_maxPolygonVertices )
			{
				TLDebug_Break("Polygon shape has too many outline points for box2d");
				return FALSE;
			}

			//	set shape vertexes
			PolygonDef.vertexCount = Outline.GetSize();
			for ( u32 i=0;	i<Outline.GetSize();	i++ )
				PolygonDef.vertices[i].Set( Outline[i].x, Outline[i].y );

			return TRUE;
		}
		break;

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


