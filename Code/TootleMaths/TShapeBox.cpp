#include "TShapeBox.h"
#include <TootleCore/TBinaryTree.h>
#include "TShapeSphere.h"
#include "TShapeCapsule.h"
#include "TShapePolygon.h"



//---------------------------------------
//	
//---------------------------------------
Bool TLMaths::TShapeBox2D::ImportData(TBinaryTree& Data)
{
	if ( !Data.ImportData("Min", m_Shape.GetMin() ) )		return FALSE;
	if ( !Data.ImportData("Max", m_Shape.GetMax() ) )		return FALSE;
	if ( !Data.ImportData("Valid", m_Shape.IsValid() ) )	return FALSE;

	return TRUE;
}

//---------------------------------------
//	
//---------------------------------------
Bool TLMaths::TShapeBox2D::ExportData(TBinaryTree& Data) const
{
	Data.ExportData("Min", m_Shape.GetMin() );
	Data.ExportData("Max", m_Shape.GetMax() );
	Data.ExportData("Valid", m_Shape.IsValid() );

	return TRUE;
}


//---------------------------------------
//	
//---------------------------------------
Bool TLMaths::TShapeBox::ImportData(TBinaryTree& Data)
{
	if ( !Data.ImportData("Min", m_Shape.GetMin() ) )		return FALSE;
	if ( !Data.ImportData("Max", m_Shape.GetMax() ) )		return FALSE;
	if ( !Data.ImportData("Valid", m_Shape.IsValid() ) )	return FALSE;

	return TRUE;
}

//---------------------------------------
//	
//---------------------------------------
Bool TLMaths::TShapeBox::ExportData(TBinaryTree& Data) const
{
	Data.ExportData("Min", m_Shape.GetMin() );
	Data.ExportData("Max", m_Shape.GetMax() );
	Data.ExportData("Valid", m_Shape.IsValid() );

	return TRUE;
}


//----------------------------------------------------------
//	
//----------------------------------------------------------
TPtr<TLMaths::TShape> TLMaths::TShapeBox::Transform(const TLMaths::TTransform& Transform,TPtr<TLMaths::TShape>& pOldShape,Bool KeepShape) const
{
	if ( !m_Shape.IsValid() )
		return NULL;

	if ( !KeepShape && Transform.HasRotation() )
	{
		TLDebug_Break("todo: need to transform into a 3D oblong (non AA box)");
		return NULL;
	}

	//	copy and transform box
	TLMaths::TBox NewBox( m_Shape );
	NewBox.Transform( Transform );

	//	re-use old shape
	if ( pOldShape && pOldShape.GetObjectPointer() != this && pOldShape->GetShapeType() == TLMaths::TBox::GetTypeRef() )
	{
		pOldShape.GetObjectPointer<TShapeBox>()->SetBox( NewBox );
		return pOldShape;
	}

	return new TShapeBox( NewBox );
}


//----------------------------------------------------------
//	fast box <-> sphere intersection
//----------------------------------------------------------
Bool TLMaths::TShapeBox::HasIntersection(TLMaths::TShapeSphere& CollisionShape)
{
	return m_Shape.GetIntersection( CollisionShape.GetSphere() );
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
TPtr<TLMaths::TShape> TLMaths::TShapeBox2D::Transform(const TLMaths::TTransform& Transform,TPtr<TLMaths::TShape>& pOldShape,Bool KeepShape) const
{
	if ( !m_Shape.IsValid() )
		return NULL;

	//	if the transform contains a rotation then it's a complex transform into an oblong
	if ( !KeepShape && Transform.HasRotation() )
	{
		//	create an oblong shape
		TLMaths::TOblong2D NewOblong( m_Shape, Transform );

		//	re-use old shape
		if ( pOldShape && pOldShape.GetObjectPointer() != this && pOldShape->GetShapeType() == TLMaths::TShapePolygon2D::GetShapeType_Static() )
		{
			pOldShape.GetObjectPointer<TShapePolygon2D>()->SetOutline( NewOblong.GetBoxCorners() );
			return pOldShape;
		}

		return new TShapePolygon2D( NewOblong.GetBoxCorners() );
	}
	else
	{
		//	simple transform
		TLMaths::TBox2D NewBox( m_Shape );
		NewBox.Transform( Transform );

		//	re-use old shape
		if ( pOldShape && pOldShape.GetObjectPointer() != this && pOldShape->GetShapeType() == TLMaths::TBox2D::GetTypeRef() )
		{
			pOldShape.GetObjectPointer<TShapeBox2D>()->SetBox( NewBox );
			return pOldShape;
		}

		return new TShapeBox2D( NewBox );
	}
}


Bool TLMaths::TShapeBox2D::HasIntersection(TShapeSphere& CollisionShape)
{
	return m_Shape.GetIntersection( CollisionShape.GetSphere() );
}

Bool TLMaths::TShapeBox2D::HasIntersection(TShapeSphere2D& CollisionShape)
{
	return m_Shape.GetIntersection( CollisionShape.GetSphere() );
}


Bool TLMaths::TShapeBox2D::HasIntersection(TShapeBox2D& CollisionShape)
{
	return m_Shape.GetIntersection( CollisionShape.GetBox() );
}


//----------------------------------------------------------
//	fast box <-> capsule intersection
//----------------------------------------------------------
Bool TLMaths::TShapeBox2D::HasIntersection(TShapeCapsule2D& CollisionShape)
{
	return m_Shape.GetIntersection( CollisionShape.GetCapsule() );
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


//----------------------------------------------------------
//	return a random position inside the shape
//----------------------------------------------------------
float3 TLMaths::TShapeBox2D::GetRandomPosition() const
{
	float x = TLMaths::Randf( m_Shape.GetLeft(), m_Shape.GetRight() );
	float y = TLMaths::Randf( m_Shape.GetTop(), m_Shape.GetBottom() );

	return float3( x, y, 0.f );
}
