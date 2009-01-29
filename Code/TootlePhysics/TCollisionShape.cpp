#include "TCollisionShape.h"
#include "TPhysicsGraph.h"
#include <TootleAsset/TAsset.h>
#include <TootleAsset/TMesh.h>


//	just for first project
#define FORCE_2D_COLLISION

#define LINEWIDTH(x)	0.f
//#define LINEWIDTH(x)	x


namespace TLPhysics
{
	Bool	GetIntersection_SphereLine(const TLMaths::TSphere& Sphere,const TLMaths::TLine& Line,float LineWidth,TIntersection& NodeAIntersection,TIntersection& NodeBIntersection);
	Bool	GetIntersection_SphereLineStrip(const TLMaths::TSphere& Sphere,const TArray<float3>& LineStrip,float LineWidth,TIntersection& NodeAIntersection,TIntersection& NodeBIntersection);
	Bool	GetIntersection_SphereLineStrip(const TLMaths::TSphere& Sphere,const TArray<float2>& LineStrip,float LineWidth,TIntersection& NodeAIntersection,TIntersection& NodeBIntersection);
	Bool	GetIntersection_SphereTriangle(const TLMaths::TSphere& Sphere,const TFixedArray<float3,3>& Triangle,TIntersection& NodeAIntersection,TIntersection& NodeBIntersection);
}






//-----------------------------------------------------
//	undo a transform applied to node A when we did the intersection test
//-----------------------------------------------------
void TLPhysics::TIntersection::Transform(const TLMaths::TTransform& Transform)
{
	if ( Transform.HasAnyTransform() )
		Transform.TransformVector( m_Intersection );
}


//-----------------------------------------------------
//	undo a transform applied to node A when we did the intersection test	
//-----------------------------------------------------
void TLPhysics::TIntersection::Untransform(const TLMaths::TTransform& Transform)
{
	Transform.UntransformVector( m_Intersection );
}



//----------------------------------------------------------
//	get intersection between sphere and line
//----------------------------------------------------------
Bool TLPhysics::GetIntersection_SphereLine(const TLMaths::TSphere& Sphere,const TLMaths::TLine& Line,float LineWidth,TIntersection& NodeAIntersection,TIntersection& NodeBIntersection)
{
	//	get the intersection point on the line...
	float3 IntersectionOnLine = Line.GetNearestPoint( Sphere.GetPos() );
	float3 DeltaToLine( IntersectionOnLine - Sphere.GetPos() );

	//	too far away?
	float Distance = DeltaToLine.LengthSq();
	float Radius = Sphere.GetRadius() + LINEWIDTH(LineWidth);

	if ( Distance > (Radius*Radius) )
		return FALSE;

	float3 DeltaToLineNormal = DeltaToLine;
	DeltaToLineNormal.Normalise( Radius );

	NodeAIntersection.m_Intersection = Sphere.GetPos() + DeltaToLineNormal;
	NodeBIntersection.m_Intersection = IntersectionOnLine;
	TLDebug_CheckFloat( NodeAIntersection.m_Intersection );
	TLDebug_CheckFloat( NodeBIntersection.m_Intersection );

	//	get normal of line
	NodeAIntersection.m_Normal = Line.GetDirectionNormal().CrossProduct( float3(0,0,1) );
	NodeAIntersection.m_HasNormal = TLDebug_CheckFloat( NodeAIntersection.m_Normal );

	return TRUE;
}



//----------------------------------------------------------
//	get intersection between sphere and line strip
//----------------------------------------------------------
Bool TLPhysics::GetIntersection_SphereLineStrip(const TLMaths::TSphere& Sphere,const TArray<float3>& LineStrip,float LineWidth,TIntersection& NodeAIntersection,TIntersection& NodeBIntersection)
{
	//	too short
	if ( LineStrip.GetSize() < 2 )
		return FALSE;

	//	just a single line, quicker...
	if ( LineStrip.GetSize() == 2 )
	{
		TLMaths::TLine Line;
		Line.Set( LineStrip[0], LineStrip[1] );
		return GetIntersection_SphereLine( Sphere, Line, LineWidth, NodeAIntersection, NodeBIntersection );
	}

	u32 IntersectionCount = 0;

	for ( u32 i=1;	i<LineStrip.GetSize();	i++ )
	{
		TLMaths::TLine Line;
		Line.Set( LineStrip[i-1], LineStrip[i] );

		//	tiny line!
		if ( Line.GetDirection().LengthSq() < TLMaths::g_NearZero )
		{
			TLDebug_Break("Tiny line");
			continue;
		}

		//	get the intersection point on the line...
		float3 IntersectionOnLine = Line.GetNearestPoint( Sphere.GetPos() );
		float3 DeltaToLine( IntersectionOnLine - Sphere.GetPos() );

		//	too far away?
		float Distance = DeltaToLine.LengthSq();
		float Radius = Sphere.GetRadius() + LINEWIDTH(LineWidth);

		if ( Distance > (Radius*Radius) )
			continue;

		float3 DeltaToLineNormal = DeltaToLine;
		DeltaToLineNormal.Normalise( Radius );
		float3 IntersectionOnSphere = Sphere.GetPos() + DeltaToLineNormal;

		//	first intersection, just set values
		if ( IntersectionCount == 0 )
		{
			NodeAIntersection.m_Intersection = IntersectionOnSphere;
			NodeBIntersection.m_Intersection = IntersectionOnLine;
			TLDebug_CheckFloat( NodeAIntersection.m_Intersection );
			TLDebug_CheckFloat( NodeBIntersection.m_Intersection );

			//	get normal of line
			NodeAIntersection.m_Normal = Line.GetDirectionNormal().CrossProduct( float3(0,0,1) );
			NodeAIntersection.m_HasNormal = TLDebug_CheckFloat( NodeAIntersection.m_Normal );
		}
		else
		{
			//	already intersected... merge intersections
			//	todo: probably need something more intelligent
			//		will probably replace with multiple-collision/intersection OnCollision's
			NodeAIntersection.m_Intersection += IntersectionOnSphere;
			NodeAIntersection.m_Intersection *= 0.5f;

			NodeBIntersection.m_Intersection += IntersectionOnLine;
			NodeBIntersection.m_Intersection *= 0.5f;

			TLDebug_CheckFloat( NodeAIntersection.m_Intersection );
			TLDebug_CheckFloat( NodeBIntersection.m_Intersection );

			//	get normal of line
			NodeAIntersection.m_Normal += Line.GetDirectionNormal().CrossProduct( float3(0,0,1) );
			NodeAIntersection.m_HasNormal = TLDebug_CheckFloat( NodeAIntersection.m_Normal );
		}

		IntersectionCount++;
	}

	if ( NodeAIntersection.m_HasNormal )
	{
		NodeAIntersection.m_Normal.Normalise();
		NodeAIntersection.m_HasNormal = TLDebug_CheckFloat( NodeAIntersection.m_Normal );
	}
	

	//	no intersections
	if ( IntersectionCount == 0 )
		return FALSE;

	return TRUE;
}


//----------------------------------------------------------
//	get intersection between sphere and line strip
//----------------------------------------------------------
Bool TLPhysics::GetIntersection_SphereLineStrip(const TLMaths::TSphere& Sphere,const TArray<float2>& LineStrip,float LineWidth,TIntersection& NodeAIntersection,TIntersection& NodeBIntersection)
{
	//	too short
	if ( LineStrip.GetSize() < 2 )
		return FALSE;

	//	just a single line, quicker...
	if ( LineStrip.GetSize() == 2 )
	{
		TLMaths::TLine Line;
		Line.Set( LineStrip[0], LineStrip[1] );
		return GetIntersection_SphereLine( Sphere, Line, LineWidth, NodeAIntersection, NodeBIntersection );
	}

	u32 IntersectionCount = 0;
	float2 SpherePos2( Sphere.GetPos().x, Sphere.GetPos().y );

	for ( u32 i=1;	i<LineStrip.GetSize();	i++ )
	{
		TLMaths::TLine2D Line;
		Line.Set( LineStrip[i-1], LineStrip[i] );

		//	tiny line!
		if ( Line.GetDirection().LengthSq() < TLMaths::g_NearZero )
		{
			TLDebug_Print("Tiny line in linestrip");
			continue;
		}

		//	get the intersection point on the line...
		float2 IntersectionOnLine = Line.GetNearestPoint( SpherePos2 );
		float2 DeltaToLine( IntersectionOnLine - SpherePos2 );

		//	too far away?
		float Distance = DeltaToLine.LengthSq();
		float Radius = Sphere.GetRadius() + LINEWIDTH(LineWidth);

		if ( Distance > (Radius*Radius) )
			continue;

		float2 DeltaToLineNormal = DeltaToLine;
		DeltaToLineNormal.Normalise( Radius );
		float3 IntersectionOnSphere( SpherePos2.x + DeltaToLineNormal.x, SpherePos2.y + DeltaToLineNormal.y, 0.f );
		float3 LineDirNormal( Line.GetDirection(0.f).Normal() );

		//	first intersection, just set values
		if ( IntersectionCount == 0 )
		{
			NodeAIntersection.m_Intersection = IntersectionOnSphere;
			NodeBIntersection.m_Intersection = IntersectionOnLine;
			TLDebug_CheckFloat( NodeAIntersection.m_Intersection );
			TLDebug_CheckFloat( NodeBIntersection.m_Intersection );

			//	get normal of line
			NodeAIntersection.m_Normal = LineDirNormal.CrossProduct( float3(0,0,1) );
			NodeAIntersection.m_HasNormal = TLDebug_CheckFloat( NodeAIntersection.m_Normal );
		}
		else
		{
			//	already intersected... merge intersections
			//	todo: probably need something more intelligent
			//		will probably replace with multiple-collision/intersection OnCollision's
			NodeAIntersection.m_Intersection += IntersectionOnSphere;
			NodeAIntersection.m_Intersection *= 0.5f;

			NodeBIntersection.m_Intersection += IntersectionOnLine;
			NodeBIntersection.m_Intersection *= 0.5f;

			TLDebug_CheckFloat( NodeAIntersection.m_Intersection );
			TLDebug_CheckFloat( NodeBIntersection.m_Intersection );

			//	get normal of line
			NodeAIntersection.m_Normal += LineDirNormal.CrossProduct( float3(0,0,1) );
			NodeAIntersection.m_HasNormal = TLDebug_CheckFloat( NodeAIntersection.m_Normal );
		}

		IntersectionCount++;
	}

	if ( NodeAIntersection.m_HasNormal )
	{
		NodeAIntersection.m_Normal.Normalise();
		NodeAIntersection.m_HasNormal = TLDebug_CheckFloat( NodeAIntersection.m_Normal );
	}
	

	//	no intersections
	if ( IntersectionCount == 0 )
		return FALSE;

	return TRUE;
}



//----------------------------------------------------------
//	get intersection between sphere and triangle
//----------------------------------------------------------
Bool TLPhysics::GetIntersection_SphereTriangle(const TLMaths::TSphere& Sphere,const TFixedArray<float3,3>& Triangle,TIntersection& NodeAIntersection,TIntersection& NodeBIntersection)
{
	TLDebug_Break("todo: GetIntersection_SphereTriangle");
	return FALSE;
}

//----------------------------------------------------------
//	work out which fast intersection func to use
//----------------------------------------------------------
Bool TLPhysics::TCollisionShape::HasIntersection(TLPhysics::TCollisionShape* pCollisionShape)
{
	if ( !pCollisionShape )
	{
		TLDebug_Break("Collision Shape expected");
		return FALSE;
	}

	switch ( pCollisionShape->GetShapeType() )
	{
	case TLPhysics::Shape_Sphere:
		return HasIntersection_Sphere( static_cast<TCollisionSphere*>(pCollisionShape) );
		break;

	case TLPhysics::Shape_Box:
		return HasIntersection_Box( static_cast<TCollisionBox*>(pCollisionShape) );
		break;

	case TLPhysics::Shape_Mesh:
		return HasIntersection_Mesh( static_cast<TCollisionMesh*>(pCollisionShape) );
		break;

	case TLPhysics::Shape_MeshWithBounds:
		return HasIntersection_MeshWithBounds( static_cast<TCollisionMeshWithBounds*>(pCollisionShape) );
		break;

	default:
		TLDebug_Break("HasIntersection: Unhandled shape type");
		return FALSE;
		break;
	}
}


Bool TLPhysics::TCollisionShape::HasIntersection_Sphere(TLPhysics::TCollisionSphere* pCollisionShape)	
{	
	TIntersection IntA,IntB;	
	return GetIntersection_Sphere( pCollisionShape, IntA, IntB );	
}

Bool TLPhysics::TCollisionShape::HasIntersection_Box(TLPhysics::TCollisionBox* pCollisionShape)	
{	
	TIntersection IntA,IntB;
	return GetIntersection_Box( pCollisionShape, IntA, IntB );
}

Bool TLPhysics::TCollisionShape::HasIntersection_Box2D(TLPhysics::TCollisionBox2D* pCollisionShape)	
{	
	TIntersection IntA,IntB;
	return GetIntersection_Box2D( pCollisionShape, IntA, IntB );
}	

Bool TLPhysics::TCollisionShape::HasIntersection_Mesh(TLPhysics::TCollisionMesh* pCollisionMesh)		
{
	TIntersection IntA,IntB;
	return GetIntersection_Mesh( pCollisionMesh, IntA, IntB );
}	

Bool TLPhysics::TCollisionShape::HasIntersection_MeshWithBounds(TLPhysics::TCollisionMeshWithBounds* pCollisionMesh)	
{	
	TIntersection IntA,IntB;
	return GetIntersection_MeshWithBounds( pCollisionMesh, IntA, IntB );
}


//----------------------------------------------------------
//	work out which intersection func to use
//----------------------------------------------------------
Bool TLPhysics::TCollisionShape::GetIntersection(TLPhysics::TCollisionShape* pCollisionShape,TIntersection& NodeAIntersection,TIntersection& NodeBIntersection)
{
	if ( !pCollisionShape )
	{
		TLDebug_Break("Collision Shape expected");
		return FALSE;
	}

	switch ( pCollisionShape->GetShapeType() )
	{
	case TLPhysics::Shape_Sphere:
		return GetIntersection_Sphere( static_cast<TCollisionSphere*>(pCollisionShape), NodeAIntersection, NodeBIntersection );
		break;

	case TLPhysics::Shape_Box:
		return GetIntersection_Box( static_cast<TCollisionBox*>(pCollisionShape), NodeAIntersection, NodeBIntersection );
		break;

	case TLPhysics::Shape_Mesh:
		return GetIntersection_Mesh( static_cast<TCollisionMesh*>(pCollisionShape), NodeAIntersection, NodeBIntersection );
		break;

	case TLPhysics::Shape_MeshWithBounds:
		return GetIntersection_MeshWithBounds( static_cast<TCollisionMeshWithBounds*>(pCollisionShape), NodeAIntersection, NodeBIntersection );
		break;

	default:
		TLDebug_Break("GetIntersection: Unhandled shape type");
		return FALSE;
		break;
	}
}


Bool TLPhysics::TCollisionShape::GetIntersection_Sphere(TLPhysics::TCollisionSphere* pCollisionShape,TIntersection& NodeAIntersection,TIntersection& NodeBIntersection)		
{
	TLDebug_Break("This shape type has not implemented Sphere intersection test");
	return FALSE;	
}

Bool TLPhysics::TCollisionShape::GetIntersection_Box(TLPhysics::TCollisionBox* pCollisionShape,TIntersection& NodeAIntersection,TIntersection& NodeBIntersection)		
{
	TLDebug_Break("This shape type has not implemented Box intersection test");
	return FALSE;	
}


Bool TLPhysics::TCollisionShape::GetIntersection_Box2D(TLPhysics::TCollisionBox2D* pCollisionShape,TIntersection& NodeAIntersection,TIntersection& NodeBIntersection)		
{
	TLDebug_Break("This shape type has not implemented Box2D intersection test");
	return FALSE;	
}


Bool TLPhysics::TCollisionShape::GetIntersection_Mesh(TLPhysics::TCollisionMesh* pCollisionMesh,TIntersection& NodeAIntersection,TIntersection& NodeBIntersection)
{
	TLDebug_Break("This shape type has not implemented mesh intersection test");
	return FALSE;	
}


Bool TLPhysics::TCollisionShape::GetIntersection_MeshWithBounds(TLPhysics::TCollisionMeshWithBounds* pCollisionMesh,TIntersection& NodeAIntersection,TIntersection& NodeBIntersection)
{
	TLDebug_Break("This shape type has not implemented mesh intersection test");
	return FALSE;	
}


//----------------------------------------------------------
//	
//----------------------------------------------------------
TPtr<TLPhysics::TCollisionShape> TLPhysics::TCollisionSphere::Transform(const TLMaths::TTransform& Transform,TPtr<TLPhysics::TCollisionShape>& pThis,TPtr<TLPhysics::TCollisionShape>& pOldShape)
{
	if ( !m_Sphere.IsValid() )
		return NULL;

	//	no transform, so just use ourself
	if ( !Transform.HasAnyTransform() )
	{
		return pThis;
	}

	//	copy and transform sphere
	TLMaths::TSphere NewSphere( m_Sphere );
	NewSphere.Transform( Transform );

	TLDebug_CheckFloat( m_Sphere.GetPos() );

	//	re-use old shape
	if ( pOldShape && pOldShape->GetShapeType() == TLPhysics::Shape_Sphere )
	{
		pOldShape.GetObject<TCollisionSphere>()->SetSphere( NewSphere );
		return pOldShape;
	}

	return new TCollisionSphere( NewSphere );
}


//----------------------------------------------------------
//	sphere <-> sphere intersection
//----------------------------------------------------------
Bool TLPhysics::TCollisionSphere::GetIntersection_Sphere(TLPhysics::TCollisionSphere* pCollisionShape,TIntersection& NodeAIntersection,TIntersection& NodeBIntersection)
{
	const TLMaths::TSphere& a = this->GetSphere();
	const TLMaths::TSphere& b = pCollisionShape->GetSphere();
	
	//	get the vector between the spheres
	float3 Diff( b.GetPos() );
	Diff -= a.GetPos();

	//	get length
	float DiffLengthSq = Diff.LengthSq();

	//	too embedded to do anything with it 
	if ( DiffLengthSq < TLMaths::g_NearZero )     
		return FALSE;   

	float TotalRadSq = a.GetRadius() + b.GetRadius();

	//	too far away to intersect
	if ( DiffLengthSq > TotalRadSq*TotalRadSq )
		return FALSE;

	float DiffLength = TLMaths::Sqrtf( DiffLengthSq );

	//	intersected, work out the intersection points
	float NormalMultA = a.GetRadius() / DiffLength;
	NodeAIntersection.m_Intersection = a.GetPos() + (Diff * NormalMultA);
	TLDebug_CheckFloat( NodeAIntersection.m_Intersection );

	float NormalMultB = b.GetRadius() / DiffLength;
	NodeBIntersection.m_Intersection = b.GetPos() - (Diff * NormalMultB);
	TLDebug_CheckFloat( NodeBIntersection.m_Intersection );

	return TRUE;
}



//----------------------------------------------------------
//
//----------------------------------------------------------
Bool TLPhysics::TCollisionSphere::GetIntersection_Mesh(TLPhysics::TCollisionMesh* pCollisionMesh,TIntersection& NodeAIntersection,TIntersection& NodeBIntersection)
{
	//	get asset
	TPtr<TLAsset::TMesh>& pMeshAsset = pCollisionMesh->GetMeshAsset();
	TLAsset::TMesh* pMesh = pMeshAsset.GetObject();
	if ( !pMesh )
	{
		//	gr: this asserts here in snowman code when you delete a line because the mesh is deleted, but
		//	the physics node is delayed for a frame. The solution will eventually be a message from the asset manager
		//	to let objects/nodes/etc know when an asset has changed or has been deleted
		TLDebug_Break("Mesh asset expected... need to catch this earlier? with isvalid?");
		return FALSE;
	}

	//	transform our sphere, it's cheaper to un-transform a sphere than moving the mesh
	TLMaths::TSphere TempSphere = GetSphere();
	TLMaths::TTransform MeshTransform;
	if ( pCollisionMesh->GetMeshTransform( MeshTransform ) )
		TempSphere.Untransform( MeshTransform );

	//	test against geometry
	u32 p=0;
	const TArray<float3>& Verts = pMesh->GetVertexes();

	if ( pMesh->GetLines().GetSize() )
	{
	//	TLMaths::TCapsule TempCapsule;
	//	TempCapsule.SetRadius( pMesh->GetLineWidth() );

		for ( p=0;	p<pMesh->GetLines().GetSize();	p++ )
		{
			const TLAsset::TMesh::Line& Line = pMesh->GetLines().ElementAt(p);
			if ( Line.GetSize() < 2 )
				continue;

			u32 LineSize = Line.GetSize();
			if ( LineSize > TLAsset::g_MaxLineStripSize )
			{
				TLDebug_Break("Asset with line strip that's too long!");
				LineSize = TLAsset::g_MaxLineStripSize;
			}

			//	get all the verts for the line
	#ifdef FORCE_2D_COLLISION
			TFixedArray<float2,TLAsset::g_MaxLineStripSize> LineStripBuffer( LineSize );
	#else
			TFixedArray<float3,TLAsset::g_MaxLineStripSize> LineStripBuffer( LineSize );
	#endif
			for ( u32 i=0;	i<LineStripBuffer.GetSize();	i++ )
				LineStripBuffer[i] = Verts[ Line[i] ];

			//	do linestrip intersection test
			if ( GetIntersection_SphereLineStrip( TempSphere, LineStripBuffer, pMesh->GetLineWidth(), NodeAIntersection, NodeBIntersection ) )
			{
				if ( MeshTransform.HasAnyTransform() )
				{
					//	transform the intersection to match where the mesh would be
					NodeAIntersection.Transform( MeshTransform );
					NodeBIntersection.Transform( MeshTransform );
				}
				return TRUE;
			}
		}
	}
/*
	for ( p=0;	p<pMesh->GetTriangles().GetSize();	p++ )
	{
		const TLAsset::TMesh::Triangle& Triangle = pMesh->GetTriangles()[p];
		TFixedArray<float3,3> TrianglePoints;
		TrianglePoints[0] = Verts[ Triangle[0] ];
		TrianglePoints[1] = Verts[ Triangle[1] ];
		TrianglePoints[2] = Verts[ Triangle[2] ];

		if ( GetIntersection_SphereTriangle( TempSphere, TrianglePoints, NodeAIntersection, NodeBIntersection ) )
		{
			//	transform the intersection to match where the mesh would be
			if ( MeshTransform.HasAnyTransform() )
			{
				NodeAIntersection.Transform( MeshTransform );
				NodeBIntersection.Transform( MeshTransform );
			}
			return TRUE;
		}
	}

	for ( p=0;	p<pMesh->GetTristrips().GetSize();	p++ )
	{
		TLDebug_Break("Todo: sphere-tristrip intersection");
		break;
	}

	for ( p=0;	p<pMesh->GetTrifans().GetSize();	p++ )
	{
		TLDebug_Break("Todo: sphere-trifan intersection");
		break;
	}
*/
	return FALSE;
}



//----------------------------------------------------------
//
//----------------------------------------------------------
Bool TLPhysics::TCollisionSphere::GetIntersection_MeshWithBounds(TLPhysics::TCollisionMeshWithBounds* pCollisionMesh,TIntersection& NodeAIntersection,TIntersection& NodeBIntersection)
{
	//	fast intersection checks before we do complex geometry checks
	const TLMaths::TSphere& CollisionSphere = pCollisionMesh->GetSphere();
	if ( CollisionSphere.IsValid() )
	{
		if ( !GetSphere().GetIntersection( CollisionSphere ) )
			return FALSE;
	}

	const TLMaths::TBox& CollisionBox = pCollisionMesh->GetBox();
	if ( CollisionBox.IsValid() )
	{
		if ( !GetSphere().GetIntersection( CollisionBox ) )
			return FALSE;
	}

	//	will probbaly intersect with geometry so do a complex intersection test now
	return GetIntersection_Mesh( pCollisionMesh, NodeAIntersection, NodeBIntersection );
}



//----------------------------------------------------------
//	
//----------------------------------------------------------
TPtr<TLPhysics::TCollisionShape> TLPhysics::TCollisionBox::Transform(const TLMaths::TTransform& Transform,TPtr<TLPhysics::TCollisionShape>& pThis,TPtr<TLPhysics::TCollisionShape>& pOldShape)
{
	if ( !m_Box.IsValid() )
		return NULL;

	//	no transform, so just use ourself
	if ( !Transform.HasAnyTransform() )
	{
		return pThis;
	}

	//	copy and transform box
	TLMaths::TBox NewBox( m_Box );
	NewBox.Transform( Transform );

	//	re-use old shape
	if ( pOldShape && pOldShape->GetShapeType() == TLPhysics::Shape_Box )
	{
		pOldShape.GetObject<TCollisionBox>()->SetBox( NewBox );
		return pOldShape;
	}

	return new TCollisionBox( NewBox );
}


//----------------------------------------------------------
//	fast box <-> sphere intersection
//----------------------------------------------------------
Bool TLPhysics::TCollisionBox::HasIntersection_Sphere(TLPhysics::TCollisionSphere* pCollisionShape)
{
	//	fast rejection test
	if ( !m_BoundsSphere.GetIntersection( pCollisionShape->GetSphere() ) )
		return FALSE;

	return m_Box.GetIntersection( pCollisionShape->GetSphere() );
}


//----------------------------------------------------------
//	fast box <-> mesh intersection
//----------------------------------------------------------
Bool TLPhysics::TCollisionBox::HasIntersection_Mesh(TLPhysics::TCollisionMesh* pCollisionMesh)
{
	//	get asset
	TPtr<TLAsset::TMesh>& pMesh = pCollisionMesh->GetMeshAsset();
	if ( !pMesh )
	{
		TLDebug_Break("Mesh asset expected... need to catch this earlier? with isvalid?");
		return FALSE;
	}

	//	transform our sphere, it's cheaper to un-transform a sphere than moving the mesh
	TLMaths::TBox TempBox = GetBox();
	TLMaths::TTransform MeshTransform;
	if ( pCollisionMesh->GetMeshTransform( MeshTransform ) )
		TempBox.Untransform( MeshTransform );

	//	test against geometry
	u32 p=0;
	const TArray<float3>& Verts = pMesh->GetVertexes();

	if ( pMesh->GetLines().GetSize() )
	{
		TLMaths::TCapsule TempCapsule;
		TempCapsule.SetRadius( pMesh->GetLineWidth() );

		for ( p=0;	p<pMesh->GetLines().GetSize();	p++ )
		{
			const TLAsset::TMesh::Line& Line = pMesh->GetLines().ElementAt(p);
			if ( Line.GetSize() < 2 )
				continue;

			TLMaths::TLine& LineShape = TempCapsule.GetLine();
			for ( u32 i=1;	i<Line.GetSize();	i++ )
			{
				LineShape.Set( Verts[ Line[i-1] ], Verts[ Line[i] ] );
				//if ( TempBox.GetIntersection( TempCapsule ) )
				if ( TempBox.GetIntersection( LineShape ) )
				{
					return TRUE;
				}
			}
		}
	}

	for ( p=0;	p<pMesh->GetTriangles().GetSize();	p++ )
	{
		TLDebug_Break("Todo: box-triangle intersection");
		break;
	}

	for ( p=0;	p<pMesh->GetTristrips().GetSize();	p++ )
	{
		TLDebug_Break("Todo: sphere-tristrip intersection");
		break;
	}

	for ( p=0;	p<pMesh->GetTrifans().GetSize();	p++ )
	{
		TLDebug_Break("Todo: sphere-trifan intersection");
		break;
	}

	return FALSE;
}


//----------------------------------------------------------
//	fast box <-> mesh intersection
//----------------------------------------------------------
Bool TLPhysics::TCollisionBox::HasIntersection_MeshWithBounds(TLPhysics::TCollisionMeshWithBounds* pCollisionMesh)
{
	TLDebug_Break("If this is used, it can be sped up");

	//	gr: dont need to do mesh intersection test?
	return HasIntersection_Mesh( pCollisionMesh );
}


//----------------------------------------------------------
//	get mesh asset and cache
//----------------------------------------------------------
TPtr<TLAsset::TMesh>& TLPhysics::TCollisionMesh::GetMeshAsset()
{
	if ( !m_pMeshCache && m_MeshRef.IsValid() )
	{
		m_pMeshCache = TLAsset::GetAsset( m_MeshRef, TRUE );
	}

	return m_pMeshCache;										 
}

//----------------------------------------------------------
//	get mesh asset, but cannot cache as const
//----------------------------------------------------------
TPtr<TLAsset::TMesh> TLPhysics::TCollisionMesh::GetMeshAsset() const
{
	if ( m_pMeshCache )
		return m_pMeshCache;

	TPtr<TLAsset::TMesh> pMesh = TLAsset::GetAsset( m_MeshRef, TRUE );

	return pMesh;										 
}


//----------------------------------------------------------
//	
//----------------------------------------------------------
float3 TLPhysics::TCollisionMesh::GetCenter() const
{
	TLDebug_Break("Todo: get mesh center");
	return float3(0,0,0);
}


//----------------------------------------------------------
//	
//----------------------------------------------------------
TPtr<TLPhysics::TCollisionShape> TLPhysics::TCollisionMesh::Transform(const TLMaths::TTransform& Transform,TPtr<TLPhysics::TCollisionShape>& pThis,TPtr<TLPhysics::TCollisionShape>& pOldShape)
{
	//	get asset
	TPtr<TLAsset::TMesh>& pMesh = GetMeshAsset();
	if ( !pMesh )
	{
		TTempString DebugString("Collision mesh ");
		m_MeshRef.GetString( DebugString );
		DebugString.Append(" missing");
		TLDebug_Print( DebugString );
		return NULL;
	}

	//	make up a bounds sphere and box, transform them and make a new mesh with those
	TLMaths::TSphere BoundsSphere = pMesh->CalcBoundsSphere();
	TLMaths::TBox BoundsBox = pMesh->CalcBoundsBox();

	BoundsSphere.Transform( Transform );
	BoundsBox.Transform( Transform );

	TPtr<TLPhysics::TCollisionMeshWithBounds> pNewCollisionMesh = NULL;

	//	re-use old shape
	if ( pOldShape && pOldShape->GetShapeType() == TLPhysics::Shape_MeshWithBounds )
	{
		pNewCollisionMesh = pOldShape;
	}
	else
	{
		pNewCollisionMesh = new TCollisionMeshWithBounds;
	}

	//	setup transformed collision mesh
	pNewCollisionMesh->SetMeshRef( m_MeshRef );
	pNewCollisionMesh->SetMeshTransform( Transform );
	pNewCollisionMesh->SetSphere( BoundsSphere );
	pNewCollisionMesh->SetBox( BoundsBox );

	return pNewCollisionMesh;
}


/* XXX

//----------------------------------------------------------
//
//----------------------------------------------------------
Bool TLPhysics::TCollisionMeshWithBounds::HasIntersection_Box(TLPhysics::TCollisionBox* pCollisionShape)
{
	//	see if collision shape intersects our bounds objects, if it doesn't it's too far from the mesh to collide
	//	note: bounds shapes are pre-transformed! no need to transform them!

	if ( m_BoundsSphere.IsValid() )
	{
		if ( !pCollisionShape->GetBox().GetIntersection( m_BoundsSphere ) )
			return FALSE;
	}

	if ( m_BoundsBox.IsValid() ) 
	{
		if ( !pCollisionShape->GetBox().GetIntersection( m_BoundsBox ) )
			return FALSE;
	}

	//	do normal/plain intersection test
	//	gr: this needs to untransform the shape?
	if ( m_MeshTransform.HasAnyTransform() )
	{
		TLDebug_Break("Check: code below needs to untransform sphere by mesh transform?");
	}

	return TCollisionMesh::HasIntersection_Box( pCollisionShape );
}
*/

//----------------------------------------------------------
//
//----------------------------------------------------------
Bool TLPhysics::TCollisionMeshWithBounds::GetIntersection_Sphere(TLPhysics::TCollisionSphere* pCollisionShape,TIntersection& NodeAIntersection,TIntersection& NodeBIntersection)
{
	//	see if collision shape intersects our bounds objects, if it doesn't it's too far from the mesh to collide
	//	note: bounds shapes are pre-transformed! no need to transform them!

	if ( m_BoundsSphere.IsValid() )
	{
		if ( !pCollisionShape->GetSphere().GetIntersection( m_BoundsSphere ) )
			return FALSE;
	}

	if ( m_BoundsBox.IsValid() ) 
	{
		if ( !pCollisionShape->GetSphere().GetIntersection( m_BoundsBox ) )
			return FALSE;
	}

	//	do normal/plain intersection test
	//	gr: this needs to untransform the shape?
	if ( m_MeshTransform.HasAnyTransform() )
	{
		TLDebug_Break("Check: code below needs to untransform sphere by mesh transform?");
	}

	return TCollisionMesh::GetIntersection_Sphere( pCollisionShape, NodeAIntersection, NodeBIntersection );
}


//----------------------------------------------------------
//
//----------------------------------------------------------
Bool TLPhysics::TCollisionMeshWithBounds::GetIntersection_Box(TLPhysics::TCollisionBox* pCollisionShape,TIntersection& NodeAIntersection,TIntersection& NodeBIntersection)
{
	//	see if collision shape intersects our bounds objects, if it doesn't it's too far from the mesh to collide
	//	note: bounds shapes are pre-transformed! no need to transform them!

	if ( m_BoundsSphere.IsValid() )
	{
		if ( !pCollisionShape->GetBox().GetIntersection( m_BoundsSphere ) )
			return FALSE;
	}

	if ( m_BoundsBox.IsValid() ) 
	{
		if ( !pCollisionShape->GetBox().GetIntersection( m_BoundsBox ) )
			return FALSE;
	}

	//	do normal/plain intersection test
	return TCollisionMesh::GetIntersection_Box( pCollisionShape, NodeAIntersection, NodeBIntersection );
}


//----------------------------------------------------------
//
//----------------------------------------------------------
Bool TLPhysics::TCollisionMeshWithBounds::GetIntersection_Box2D(TLPhysics::TCollisionBox2D* pCollisionShape,TIntersection& NodeAIntersection,TIntersection& NodeBIntersection)
{
	TLDebug_Break("todo");

	//	do normal/plain intersection test
	return TCollisionMesh::GetIntersection_Box2D( pCollisionShape, NodeAIntersection, NodeBIntersection );
}



//----------------------------------------------------------
//	
//----------------------------------------------------------
float3 TLPhysics::TCollisionMeshWithBounds::GetCenter() const
{
	//	take from sphere if possible
	if ( m_BoundsSphere.IsValid() )
		return m_BoundsSphere.GetPos();

	//	take from box
	if ( m_BoundsBox.IsValid() )
		return m_BoundsBox.GetCenter();

	//	take from mesh
	return TCollisionMesh::GetCenter();
}






//----------------------------------------------------------
//	
//----------------------------------------------------------
TPtr<TLPhysics::TCollisionShape> TLPhysics::TCollisionBox2D::Transform(const TLMaths::TTransform& Transform,TPtr<TLPhysics::TCollisionShape>& pThis,TPtr<TLPhysics::TCollisionShape>& pOldShape)
{
	if ( !m_Box.IsValid() )
		return NULL;

	//	no transform, so just use ourself
	if ( !Transform.HasAnyTransform() )
	{
		return pThis;
	}

	//	copy and transform box
	TLMaths::TBox2D NewBox( m_Box );
	NewBox.Transform( Transform );

	//	re-use old shape
	if ( pOldShape && pOldShape->GetShapeType() == TLPhysics::Shape_Box2D )
	{
		pOldShape.GetObject<TCollisionBox2D>()->SetBox( NewBox );
		return pOldShape;
	}

	return new TCollisionBox2D( NewBox );
}


Bool TLPhysics::TCollisionBox2D::HasIntersection_Sphere(TCollisionSphere* pCollisionShape)
{
	return m_Box.GetIntersection( pCollisionShape->GetSphere() );
}




//----------------------------------------------------------
//	fast box <-> mesh intersection
//----------------------------------------------------------
Bool TLPhysics::TCollisionBox2D::HasIntersection_Mesh(TLPhysics::TCollisionMesh* pCollisionMesh)
{
	//	get asset
	TPtr<TLAsset::TMesh>& pMesh = pCollisionMesh->GetMeshAsset();
	if ( !pMesh )
	{
//		TLDebug_Break("Mesh asset expected... need to catch this earlier? with isvalid?");
		return FALSE;
	}

	//	transform our sphere, it's cheaper to un-transform a sphere than moving the mesh
	TLMaths::TBox2D TempBox = GetBox();
	TLMaths::TTransform MeshTransform;
	if ( pCollisionMesh->GetMeshTransform( MeshTransform ) )
		TempBox.Untransform( MeshTransform );

	//	test against geometry
	u32 p=0;
	const TArray<float3>& Verts = pMesh->GetVertexes();

	if ( pMesh->GetLines().GetSize() )
	{
		TLMaths::TCapsule TempCapsule;
		TempCapsule.SetRadius( pMesh->GetLineWidth() );

		for ( p=0;	p<pMesh->GetLines().GetSize();	p++ )
		{
			const TLAsset::TMesh::Line& Line = pMesh->GetLines().ElementAt(p);
			if ( Line.GetSize() < 2 )
				continue;

			TLMaths::TLine& LineShape = TempCapsule.GetLine();
			for ( u32 i=1;	i<Line.GetSize();	i++ )
			{
				LineShape.Set( Verts[ Line[i-1] ], Verts[ Line[i] ] );
				//if ( TempBox.GetIntersection( TempCapsule ) )
				if ( TempBox.GetIntersection( LineShape ) )
				{
					return TRUE;
				}
			}
		}
	}
/*
	for ( p=0;	p<pMesh->GetTriangles().GetSize();	p++ )
	{
		TLDebug_Break("Todo: box-triangle intersection");
		break;
	}

	for ( p=0;	p<pMesh->GetTristrips().GetSize();	p++ )
	{
		TLDebug_Break("Todo: sphere-tristrip intersection");
		break;
	}

	for ( p=0;	p<pMesh->GetTrifans().GetSize();	p++ )
	{
		TLDebug_Break("Todo: sphere-trifan intersection");
		break;
	}
*/
	return FALSE;
}


//----------------------------------------------------------
//	fast box <-> mesh intersection
//----------------------------------------------------------
Bool TLPhysics::TCollisionBox2D::HasIntersection_MeshWithBounds(TLPhysics::TCollisionMeshWithBounds* pCollisionMesh)
{
	if ( !m_Box.GetIntersection( pCollisionMesh->GetBox() ) )
		return FALSE;

	if ( !m_Box.GetIntersection( pCollisionMesh->GetSphere() ) )
		return FALSE;

	//	gr: dont need to do mesh intersection test?
	return HasIntersection_Mesh( pCollisionMesh );
}
