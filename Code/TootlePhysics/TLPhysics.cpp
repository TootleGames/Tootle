#include "TLPhysics.h"
#include "TPhysicsNode.h"
#include <TootleMaths/TShape.h>
#include <TootleMaths/TShapeBox.h>
#include <TootleMaths/TShapeSphere.h>
#include <TootleMaths/TShapePolygon.h>

//	namespace Box2D
#include <box2d/include/box2d.h>


//------------------------------------------------------
//	create a transformed shape from a body shape
//------------------------------------------------------
TPtr<TLMaths::TShape> TLPhysics::GetShapeFromBodyShape(b2Fixture& BodyShape,const TLMaths::TTransform& Transform)
{
	//	gr: I figured the accurate/fast verison would transform by box2d. 
	//		but if we use the box2D transform, then it's out of date if the
	//		node is disabled(body frozen) and the transform[on the node] is changed as the
	//		body's transform cannot be changed until it's enabled (body is unfrozen). 

	//	gr: change this to use box2D when ENABLED and use our node transform when DISABLED.
//#define TRANSFORM_BY_BOX2D

	b2Body& Body = *BodyShape.GetBody();
	b2Shape* pBodyShape = BodyShape.GetShape();
	
	if ( !pBodyShape )
	{
		TLDebug_Break("Fixture missing shape");
		return NULL;
	}

	if ( BodyShape.GetType() == b2_polygonShape )
	{
		b2PolygonShape& PolyShape = static_cast<b2PolygonShape&>( *pBodyShape );

#ifdef TRANSFORM_BY_BOX2D
		const b2XForm& Bodyxf = Body.GetXForm();
#endif

		//	get a list of the points
		TFixedArray<float2,100> Points;
		for ( s32 p=0;	p<PolyShape.GetVertexCount();	p++ )
		{
			#ifdef TRANSFORM_BY_BOX2D
				//	transform by bodys transform
				b2Vec2 WorldPos = b2Mul( Bodyxf, PolyShape.GetVertex(p) );

				//	gr: transform by ourtransform for scale? or instead of the box2d one? as box2d lacks scale
				float2 WorldPos2( WorldPos.x, WorldPos.y );
			#else
				//	transform by OUR transform
				float2 WorldPos2( PolyShape.GetVertex(p).x, PolyShape.GetVertex(p).y );
				Transform.Transform( WorldPos2 );
			#endif

			Points.Add( WorldPos2 );
		}

		//	create shape
		return new TLMaths::TShapePolygon2D( Points );
	}
	else if ( BodyShape.GetType() == b2_circleShape )
	{
		b2CircleShape& CircleShape = static_cast<b2CircleShape&>( *pBodyShape );

		#ifdef TRANSFORM_BY_BOX2D
			//	transform by bodys transform to put shape in world space
			const b2XForm& Bodyxf = Body.GetXForm();
			b2Vec2 WorldPos = b2Mul( Bodyxf, CircleShape.m_p );
		#else
			//	transform by OUR transform
			float2 WorldPos( CircleShape.m_p.x, CircleShape.m_p.y );
			Transform.Transform( WorldPos );
		#endif

		//	make circle
		TLMaths::TSphere2D Circle( float2( WorldPos.x, WorldPos.y ), CircleShape.m_radius );
		return new TLMaths::TShapeSphere2D( Circle );
	}
	else if ( BodyShape.GetType() == b2_edgeShape )
	{
		b2EdgeShape& EdgeShape = static_cast<b2EdgeShape&>( *pBodyShape );
	
		//	make line
		#ifdef TRANSFORM_BY_BOX2D
			//	transform by bodys transform to put shape in world space
			const b2XForm& Bodyxf = Body.GetXForm();
			b2Vec2 v1 = b2Mul( Bodyxf, EdgeShape.GetVertex1() );
			b2Vec2 v2 = b2Mul( Bodyxf, EdgeShape.GetVertex2() );
			TLMaths::TLine2D Line( float2( v1.x, v1.y ), float2( v2.x, v2.y ) );
		#else
			//	transform by OUR transform
			const b2Vec2& v1 = EdgeShape.GetVertex1();
			const b2Vec2& v2 = EdgeShape.GetVertex2();
			TLMaths::TLine2D Line( float2( v1.x, v1.y ), float2( v2.x, v2.y ) );
			Line.Transform( Transform );
		#endif
			
		return new TLMaths::TShapeLine2D( Line );
	}
	else
	{
		TLDebug_Break("Invalid body shape");
	}

	return NULL;
}


//------------------------------------------------------
//	get a box2D shape definition from a shape. need some temporaries to decide which to return
//------------------------------------------------------
b2FixtureDef* TLPhysics::GetShapeDefFromShape(b2CircleDef& TempCircleDef,b2PolygonDef& TempPolygonDef,const TLMaths::TShape& Shape)
{
	if ( !Shape.IsValid() )
	{
		TLDebug_Break("Cannot create shapedef - shape is invalid");
		return NULL;
	}

	//	try a circle shape first
	if ( GetCircleDefFromShape( TempCircleDef, Shape ) )
		return &TempCircleDef;

	//	try polygon def
	if ( GetPolygonDefFromShape( TempPolygonDef, Shape ) )
		return &TempPolygonDef;

	//	shape is unsupported...
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
			TLDebug_Break("should not longer have oblong TShapes. TShapePolygon2D is its replacement");
			return FALSE;
		}

		case TLMaths_ShapeRef(Polygon2D):
		{
			const TLMaths::TShapePolygon2D& ShapePolygon = static_cast<const TLMaths::TShapePolygon2D&>( Shape );
			const TArray<float2>& Outline = ShapePolygon.GetOutline();

			//	gr: limit this at the shape level?
			if ( Outline.GetSize() > b2_maxPolygonVertices )
			{
				TLDebug_Break("Polygon shape has too many outline points for box2d - use edge shape instead");
				return FALSE;
			}

			//	if is not clockwise then create a new reversed (clockwise) shape to generate from
			if ( !ShapePolygon.IsClockwise() )
			{
				//	gr: throw up an error, as Polygon2D's should now always be clockwise
				if ( !TLDebug_Break("Polygon shape should always be clockwise. Possibly old datum/shape from mesh, fix with re-parse/output. Continue to reverse contour") )
					return FALSE;

				TLMaths::TShapePolygon2D ReversedShapePolygon( Outline );
				
				ReversedShapePolygon.ReverseContour();

				//	gr: need some anti-recursive check code here
				return GetPolygonDefFromShape( PolygonDef, ReversedShapePolygon );
			}

			//	check validity of polygon for use in box2d first
			if ( !ShapePolygon.Debug_CheckIsConvex() )
			{
				if ( !TLDebug_Break("Polygon shape is concave and needs to be convex for box2d. If your shape cannot be fixed then we can implement a tesselation system that then creates multiple shapes on a body...") )
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
//	returns TRUE if this shape must be created as a edge chain
//------------------------------------------------------
Bool TLPhysics::IsEdgeChainShape(const TLMaths::TShape& Shape)
{
	if ( Shape.GetShapeType() != TLMaths_ShapeRef(Polygon2D) )
		return FALSE;

	//	if number of verts is more than box2d can cope with for a polygon then yes it needs to be an edge chain
	const TLMaths::TShapePolygon2D& ShapePolygon = static_cast<const TLMaths::TShapePolygon2D&>( Shape );
	const TArray<float2>& Outline = ShapePolygon.GetOutline();

	if ( Outline.GetSize() > b2_maxPolygonVertices )
		return TRUE;

	return FALSE;
}


//------------------------------------------------------
//	fails if wrong type of shape
//------------------------------------------------------
Bool TLPhysics::GetEdgeChainVertexes(TArray<b2Vec2>& VertexBuffer,const TLMaths::TShape& Shape)
{
	if ( Shape.GetShapeType() != TLMaths_ShapeRef(Polygon2D) )
		return FALSE;

	//	if number of verts is more than box2d can cope with for a polygon then yes it needs to be an edge chain
	const TLMaths::TShapePolygon2D& ShapePolygon = static_cast<const TLMaths::TShapePolygon2D&>( Shape );
	const TArray<float2>& Outline = ShapePolygon.GetOutline();

	for ( u32 i=0;	i<Outline.GetSize();	i++ )
		VertexBuffer.Add( b2Vec2( Outline[i].x, Outline[i].y ) );

	return TRUE;
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
	
	SetIsNewCollision( TRUE );
}


//------------------------------------------------------
//	set up end-of-collision with this node
//------------------------------------------------------
void TLPhysics::TCollisionInfo::SetIsEndOfCollision(TRefRef ShapeRef,const TLPhysics::TPhysicsNode& OtherNode,TRefRef OtherShapeRef)
{
	m_OtherNode = OtherNode.GetNodeRef();
	m_OtherNodeOwner = OtherNode.GetOwnerSceneNodeRef();
	m_OtherNodeStatic = OtherNode.IsStatic();
	m_Shape = ShapeRef;
	m_OtherShape = OtherShapeRef;

	SetIsNewCollision( FALSE );

	//	invalidate other bits
	m_Intersection.Set( 0.f, 0.f, 0.f );
	m_OtherIntersection.Set( 0.f, 0.f, 0.f );
	m_IntersectionNormal.Set( 0.f, 0.f );
}



//------------------------------------------------------
//	export this collision info into a BinaryData
//------------------------------------------------------
void TLPhysics::TCollisionInfo::ExportData(TBinaryTree& Data)
{
	Data.Write( m_IsNewCollision );

	Data.ExportData("Node", m_OtherNode );
	Data.ExportData("Owner", m_OtherNodeOwner );
	Data.ExportData("OthStatic", m_OtherNodeStatic );
	Data.ExportData("Shape", m_Shape );
	Data.ExportData("OthShape", m_OtherShape );

	if ( m_IsNewCollision )
	{
		Data.ExportData("Intersection", m_Intersection );
		Data.ExportData("OthIntersection", m_OtherIntersection );
		Data.ExportData("Normal", m_IntersectionNormal);
	}
}


//------------------------------------------------------
//	get collision info from a BinaryData
//------------------------------------------------------
Bool TLPhysics::TCollisionInfo::ImportData(TBinaryTree& Data)
{
	Data.ResetReadPos();

	//	required data
	if ( !Data.Read( m_IsNewCollision ) )
		return FALSE;

	if ( !Data.ImportData("Node", m_OtherNode ) )				return FALSE;
	if ( !Data.ImportData("Owner", m_OtherNodeOwner ) )			return FALSE;
	if ( !Data.ImportData("OthStatic", m_OtherNodeStatic ) )	return FALSE;
	if ( !Data.ImportData("Shape", m_Shape ) )					return FALSE;
	if ( !Data.ImportData("OthShape", m_OtherShape ) )			return FALSE;

	//	optional data, reset if doesn't exist
	if ( !Data.ImportData("Intersection", m_Intersection ) )
		m_Intersection.Set( 0.f, 0.f, 0.f );

	if ( !Data.ImportData("OthIntersection", m_OtherIntersection ) )
		m_OtherIntersection.Set( 0.f, 0.f, 0.f );

	if ( !Data.ImportData("Normal", m_IntersectionNormal ) )
		m_IntersectionNormal.Set( 0.f, 0.f );

	return TRUE;
}


