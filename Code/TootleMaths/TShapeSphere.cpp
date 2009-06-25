#include "TShapeSphere.h"
#include "TShapeBox.h"
#include <TootleCore/TBinaryTree.h>




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
	m_Shape.Set( Box.GetCenter(), Radius );
}


//---------------------------------------
//	
//---------------------------------------
Bool TLMaths::TShapeSphere2D::ImportData(TBinaryTree& Data)
{
	if ( !Data.ImportData("Pos", m_Shape.GetPos() ) )		return FALSE;
	if ( !Data.ImportData("Radius", m_Shape.GetRadius() ) )	return FALSE;

	return TRUE;
}

//---------------------------------------
//	
//---------------------------------------
Bool TLMaths::TShapeSphere2D::ExportData(TBinaryTree& Data) const
{
	Data.ExportData("Pos", m_Shape.GetPos() );
	Data.ExportData("Radius", m_Shape.GetRadius() );

	return TRUE;
}



//----------------------------------------------------------
//	
//----------------------------------------------------------
TPtr<TLMaths::TShape> TLMaths::TShapeSphere2D::Transform(const TLMaths::TTransform& Transform,TPtr<TLMaths::TShape>& pOldShape) const
{
	if ( !m_Shape.IsValid() )
		return NULL;

	//	copy and transform sphere
	TLMaths::TSphere2D NewSphere( m_Shape );
	NewSphere.Transform( Transform );

	TLDebug_CheckFloat( m_Shape.GetPos() );

	//	re-use old shape
	if ( pOldShape && pOldShape.GetObject() != this && pOldShape->GetShapeType() == TLMaths::TSphere2D::GetTypeRef() )
	{
		pOldShape.GetObject<TLMaths::TShapeSphere2D>()->SetSphere( NewSphere );
		return pOldShape;
	}

	return new TLMaths::TShapeSphere2D( NewSphere );
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
	m_Shape.Set( Box.GetCenter(), Radius );
}


//---------------------------------------
//	
//---------------------------------------
Bool TLMaths::TShapeSphere::ImportData(TBinaryTree& Data)
{
	if ( !Data.ImportData("Pos", m_Shape.GetPos() ) )		return FALSE;
	if ( !Data.ImportData("Radius", m_Shape.GetRadius() ) )	return FALSE;

	return TRUE;
}

//---------------------------------------
//	
//---------------------------------------
Bool TLMaths::TShapeSphere::ExportData(TBinaryTree& Data) const
{
	Data.ExportData("Pos", m_Shape.GetPos() );
	Data.ExportData("Radius", m_Shape.GetRadius() );

	return TRUE;
}




//----------------------------------------------------------
//	
//----------------------------------------------------------
TPtr<TLMaths::TShape> TLMaths::TShapeSphere::Transform(const TLMaths::TTransform& Transform,TPtr<TShape>& pOldShape) const
{
	if ( !m_Shape.IsValid() )
		return NULL;

	//	copy and transform sphere
	TLMaths::TSphere NewSphere( m_Shape );
	NewSphere.Transform( Transform );

	TLDebug_CheckFloat( m_Sphere.GetPos() );

	//	re-use old shape
	if ( pOldShape && pOldShape.GetObject() != this && pOldShape->GetShapeType() == TLMaths::TSphere::GetTypeRef() )
	{
		pOldShape.GetObject<TShapeSphere>()->SetSphere( NewSphere );
		return pOldShape;
	}

	return new TShapeSphere( NewSphere );
}





//----------------------------------------------------------
//	sphere <-> sphere intersection
//----------------------------------------------------------
Bool TLMaths::TShapeSphere::GetIntersection(TLMaths::TShapeSphere& CollisionShape,TLMaths::TIntersection& NodeAIntersection,TLMaths::TIntersection& NodeBIntersection)
{
	const TLMaths::TSphere& a = this->GetSphere();
	const TLMaths::TSphere& b = CollisionShape.GetSphere();
	
	//	get the vector between the spheres
	float3 Diff( b.GetPos() );
	Diff -= a.GetPos();

	//	get length
	float DiffLengthSq = Diff.LengthSq();

	//	too embedded to do anything with it 
	if ( DiffLengthSq < TLMaths_NearZero )     
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


/*
//----------------------------------------------------------
//
//----------------------------------------------------------
Bool TLMaths::TShapeSphere::GetIntersection(TLMaths::TShapeMesh& CollisionMesh,TLMaths::TIntersection& NodeAIntersection,TLMaths::TIntersection& NodeBIntersection)
{
	//	get asset
	TPtr<TLAsset::TMesh>& pMeshAsset = pCollisionMesh.GetMeshAsset();
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

	return FALSE;
}
*/

//----------------------------------------------------------
//
//----------------------------------------------------------
Bool TLMaths::TShapeSphere::HasIntersection(TLMaths::TShapeBox2D& CollisionShape)						
{	
	return CollisionShape.GetBox().GetIntersection( this->GetSphere() );
}


//----------------------------------------------------------
//	sphere <-> sphere intersection
//----------------------------------------------------------
Bool TLMaths::TShapeSphere2D::GetIntersection(TLMaths::TShapeSphere2D& CollisionShape,TLMaths::TIntersection& NodeAIntersection,TLMaths::TIntersection& NodeBIntersection)
{
	const TLMaths::TSphere2D& a = this->GetSphere();
	const TLMaths::TSphere2D& b = CollisionShape.GetSphere();
	
	//	get the vector between the spheres
	float2 Diff( b.GetPos() );
	Diff -= a.GetPos();

	//	get length
	float DiffLengthSq = Diff.LengthSq();

	//	too embedded to do anything with it 
	if ( DiffLengthSq < TLMaths_NearZero )     
		return FALSE;   

	float TotalRadSq = a.GetRadius() + b.GetRadius();

	//	too far away to intersect
	if ( DiffLengthSq > TotalRadSq*TotalRadSq )
		return FALSE;

	float DiffLength = TLMaths::Sqrtf( DiffLengthSq );

	//	intersected, work out the intersection points
	float NormalMultA = a.GetRadius() / DiffLength;
	NodeAIntersection.m_Intersection.xy() = a.GetPos() + (Diff * NormalMultA);
	NodeAIntersection.m_Intersection.z = 0.f;
	TLDebug_CheckFloat( NodeAIntersection.m_Intersection );

	float NormalMultB = b.GetRadius() / DiffLength;
	NodeBIntersection.m_Intersection.xy() = b.GetPos() - (Diff * NormalMultB);
	NodeBIntersection.m_Intersection.z = 0.f;
	TLDebug_CheckFloat( NodeBIntersection.m_Intersection );

	return TRUE;
}


//----------------------------------------------------------
//
//----------------------------------------------------------
Bool TLMaths::TShapeSphere2D::HasIntersection(TLMaths::TShapeBox2D& CollisionShape)						
{	
	return CollisionShape.GetBox().GetIntersection( this->GetSphere() );
}
