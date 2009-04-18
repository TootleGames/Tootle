#include "TShapeBox.h"
#include <TootleCore/TBinaryTree.h>
#include "TShapeSphere.h"
#include "TShapeCapsule.h"



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


//----------------------------------------------------------
//	
//----------------------------------------------------------
TPtr<TLMaths::TShape> TLMaths::TShapeBox::Transform(const TLMaths::TTransform& Transform,TPtr<TLMaths::TShape>& pThis,TPtr<TLMaths::TShape>& pOldShape)
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
	if ( pOldShape && pOldShape->GetShapeType() == TLMaths::TBox::GetTypeRef() )
	{
		pOldShape.GetObject<TShapeBox>()->SetBox( NewBox );
		return pOldShape;
	}

	return new TShapeBox( NewBox );
}


//----------------------------------------------------------
//	fast box <-> sphere intersection
//----------------------------------------------------------
Bool TLMaths::TShapeBox::HasIntersection(TLMaths::TShapeSphere& CollisionShape)
{
	return m_Box.GetIntersection( CollisionShape.GetSphere() );
}



/*
//----------------------------------------------------------
//	fast box <-> mesh intersection
//----------------------------------------------------------
Bool TLMaths::TShapeBox::HasIntersection(TLMaths::TShapeMesh& CollisionMesh)
{
	//	get asset
	TPtr<TLAsset::TMesh>& pMesh = CollisionMesh.GetMeshAsset();
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
*/


//----------------------------------------------------------
//	
//----------------------------------------------------------
TPtr<TLMaths::TShape> TLMaths::TShapeBox2D::Transform(const TLMaths::TTransform& Transform,TPtr<TLMaths::TShape>& pThis,TPtr<TLMaths::TShape>& pOldShape)
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
	if ( pOldShape && pOldShape->GetShapeType() == TLMaths::TBox2D::GetTypeRef() )
	{
		pOldShape.GetObject<TShapeBox2D>()->SetBox( NewBox );
		return pOldShape;
	}

	return new TShapeBox2D( NewBox );
}


Bool TLMaths::TShapeBox2D::HasIntersection(TShapeSphere& CollisionShape)
{
	return m_Box.GetIntersection( CollisionShape.GetSphere() );
}

Bool TLMaths::TShapeBox2D::HasIntersection(TShapeSphere2D& CollisionShape)
{
	return m_Box.GetIntersection( CollisionShape.GetSphere() );
}


Bool TLMaths::TShapeBox2D::HasIntersection(TShapeBox2D& CollisionShape)
{
	return m_Box.GetIntersection( CollisionShape.GetBox() );
}


//----------------------------------------------------------
//	fast box <-> capsule intersection
//----------------------------------------------------------
Bool TLMaths::TShapeBox2D::HasIntersection(TShapeCapsule2D& CollisionShape)
{
	return m_Box.GetIntersection( CollisionShape.GetCapsule() );
}


/*
//----------------------------------------------------------
//	fast box <-> mesh intersection
//----------------------------------------------------------
Bool TLMaths::TShapeBox2D::HasIntersection(TLMaths::TShapeMesh& CollisionMesh)
{
	//	get asset
	TPtr<TLAsset::TMesh>& pMesh = CollisionMesh.GetMeshAsset();
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


	return FALSE;
}
*/
