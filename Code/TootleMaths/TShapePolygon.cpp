#include "TShapePolygon.h"
#include <TootleCore/TBinaryTree.h>

//	for quick copy&paste of the box2d checks
#include <box2d/include/Box2D.h>



float3 TLMaths::TShapePolygon2D::GetCenter() const							
{
	TLMaths::TBox2D Bounds;
	Bounds.Accumulate( m_Outline );

	return Bounds.GetCenter().xyz(0.f);
}


Bool TLMaths::TShapePolygon2D::ImportData(TBinaryTree& Data)
{
	if ( !Data.ImportArrays("Outline", m_Outline ) )		
		return FALSE;

	return TRUE;
}


Bool TLMaths::TShapePolygon2D::ExportData(TBinaryTree& Data) const
{
	Data.ExportArray("Outline", m_Outline );

	return TRUE;
}


TPtr<TLMaths::TShape> TLMaths::TShapePolygon2D::Transform(const TLMaths::TTransform& Transform,TPtr<TShape>& pThis,TPtr<TShape>& pOldShape)
{
	if ( !IsValid() )
		return NULL;

	//	no transform, so just use ourself
	if ( !Transform.HasAnyTransform() )
		return pThis;

	//	re-use old shape if possible
	TPtr<TLMaths::TShape> pNewShape;
	Bool ReUseShape = FALSE;
	if ( pOldShape && pOldShape.GetObject() != this && pOldShape->GetShapeType() == GetShapeType() )
	{
	}
	else
	{
		//	otherwise create new shape 
		pNewShape = new TShapePolygon2D();
	}

	//	get the shape to modify
	TPtr<TLMaths::TShape>& pResultShape = pNewShape ? pNewShape : pOldShape;

	//	copy current outline
	TArray<float2>& NewOutline = pResultShape.GetObject<TShapePolygon2D>()->GetOutline();
	NewOutline.Copy( GetOutline() );
	pResultShape->Transform( Transform );

	return pResultShape;
}



//-----------------------------------------------------
//	debug check that matches box2d's convex polygon check
//-----------------------------------------------------
Bool TLMaths::TShapePolygon2D::Debug_CheckIsConvex() const
{
	// Compute normals. Ensure the edges have non-zero length.
	b2Vec2 m_normals[b2_maxPolygonVertices];
	for (int32 i = 0; i < m_Outline.GetSize(); ++i)
	{
		int32 i1 = i;
		int32 i2 = i + 1 < m_Outline.GetSize() ? i + 1 : 0;
		float2 edge2 = m_Outline[i2] - m_Outline[i1];
		b2Vec2 edge( edge2.x, edge2.y );
		b2Assert(edge.LengthSquared() > B2_FLT_EPSILON * B2_FLT_EPSILON);
		m_normals[i] = b2Cross(edge, 1.0f);
		m_normals[i].Normalize();
	}

	// Ensure the polygon is convex.
	for (int32 i = 0; i < m_Outline.GetSize(); ++i)
	{
		for (int32 j = 0; j < m_Outline.GetSize(); ++j)
		{
			// Don't check vertices on the current edge.
			if (j == i || j == (i + 1) % m_Outline.GetSize())
			{
				continue;
			}
			
			// Your polygon is non-convex (it has an indentation).
			// Or your polygon is too skinny.
			float2 edge2 = m_Outline[j] - m_Outline[i];
			b2Vec2 edge( edge2.x, edge2.y );
			float32 s = b2Dot(m_normals[i], edge);

			Bool IsConcave = (s < -b2_linearSlop);

			//	gr: our concave detection (angle of incident) is the opposite from box2d's because of different right/left hand coordinate system
			IsConcave = !IsConcave;

			if ( IsConcave )
				return FALSE;
		}
	}

	return TRUE;
}


//-----------------------------------------------------
//	debug check that matches box2d's counter clockwise check
//-----------------------------------------------------
Bool TLMaths::TShapePolygon2D::IsClockwise() const
{
	// Compute normals. Ensure the edges have non-zero length.
	b2Vec2 m_normals[b2_maxPolygonVertices];
	for (int32 i = 0; i < m_Outline.GetSize(); ++i)
	{
		int32 i1 = i;
		int32 i2 = i + 1 < m_Outline.GetSize() ? i + 1 : 0;
		float2 edge2 = m_Outline[i2] - m_Outline[i1];
		b2Vec2 edge( edge2.x, edge2.y );
		b2Assert(edge.LengthSquared() > B2_FLT_EPSILON * B2_FLT_EPSILON);
		m_normals[i] = b2Cross(edge, 1.0f);
		m_normals[i].Normalize();
	}

	// Ensure the polygon is counter-clockwise.
	for (int32 i = 1; i < m_Outline.GetSize(); ++i)
	{
		float32 cross = b2Cross(m_normals[i-1], m_normals[i]);

		// Keep asinf happy.
		cross = b2Clamp(cross, -1.0f, 1.0f);

		// You have consecutive edges that are almost parallel on your polygon.
		float32 angle = asinf(cross);
		if ( angle > b2_angularSlop )
			return TRUE;
	}

	return FALSE;
}


//-----------------------------------------------------
//	reverse contour order to switch between clockwise and counter clockwise
//-----------------------------------------------------
void TLMaths::TShapePolygon2D::ReverseContour()
{
	//	save old outline
	TFixedArray<float2,100> OldOutline;
	OldOutline.Copy( m_Outline );

	//	re-add in reverse order
	m_Outline.Empty();
	for ( s32 i=OldOutline.GetLastIndex();	i>=0;	i-- )
		m_Outline.Add( OldOutline[i] );
}


