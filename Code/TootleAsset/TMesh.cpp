#include "TMesh.h"
#include <TootleCore/TBinaryTree.h>


#define LINE_PADDING_HALF	(1.f)
#define GENERATE_QUADS_AS_TRIANGLES

namespace TLAsset
{
	const TColour	g_DefaultVertexColour;
}


TLAsset::TMesh::TMesh(const TRef& AssetRef) :
	TAsset		( "Mesh", AssetRef ),
	m_LineWidth	( 0.f )
{
}



//-------------------------------------------------------
//	clear mesh
//-------------------------------------------------------
void TLAsset::TMesh::Empty()
{
	m_Vertexes.Empty();
	m_Colours.Empty();

	m_Triangles.Empty();
	m_Tristrips.Empty();
	m_Trifans.Empty();
	m_Lines.Empty();

	SetBoundsInvalid();
}


//-------------------------------------------------------
//	generate a square mesh from a 2d box
//-------------------------------------------------------
void TLAsset::TMesh::GenerateQuad(const TLMaths::TBox2D& Box,const TColour& Colour,float z)
{
	float3 TopLeft = Box.GetMin().xyz( z );
	float3 BottomRight = Box.GetMin().xyz( z );
	float3 TopRight( BottomRight.x, TopLeft.y, z );
	float3 BottomLeft( TopLeft.x, BottomRight.y, z );

	//	generate quad with outline
	GenerateQuad( TopLeft, TopRight, BottomRight, BottomLeft, Colour );
}


//-------------------------------------------------------
//	generate a square mesh around a point
//-------------------------------------------------------
void TLAsset::TMesh::GenerateQuad(const float2& Center,float Size,const TColour& Colour,float z)
{
	float HalfSize = Size * 0.5f;
	float3 TopLeft		( Center.x - HalfSize, Center.y - HalfSize, z );
	float3 TopRight		( Center.x + HalfSize, Center.y - HalfSize, z );
	float3 BottomLeft	( Center.x - HalfSize, Center.y + HalfSize, z );
	float3 BottomRight	( Center.x + HalfSize, Center.y + HalfSize, z );

	//	generate quad with outline
	GenerateQuad( TopLeft, TopRight, BottomRight, BottomLeft, Colour );
}


//-------------------------------------------------------
//	generate a cube mesh from a math box
//-------------------------------------------------------
void TLAsset::TMesh::GenerateCube(const TLMaths::TBox& Box)
{
	u16 FirstVert = m_Vertexes.GetSize();

	//	for vertex order, see TLMaths::TBox::GetBoxCorners
	Box.GetBoxCorners( m_Vertexes );

	//	triangles
	//	top
	m_Triangles.Add( Triangle( FirstVert+0, FirstVert+1, FirstVert+2 ) );	
	m_Triangles.Add( Triangle( FirstVert+2, FirstVert+3, FirstVert+0 ) );

	//	front
	m_Triangles.Add( Triangle( FirstVert+3, FirstVert+2, FirstVert+6 ) );	
	m_Triangles.Add( Triangle( FirstVert+6, FirstVert+7, FirstVert+3 ) );

	//	back
	m_Triangles.Add( Triangle( FirstVert+0, FirstVert+1, FirstVert+5 ) );	
	m_Triangles.Add( Triangle( FirstVert+5, FirstVert+4, FirstVert+0 ) );

	//	right
	m_Triangles.Add( Triangle( FirstVert+2, FirstVert+1, FirstVert+5 ) );	
	m_Triangles.Add( Triangle( FirstVert+5, FirstVert+2, FirstVert+6 ) );

	//	left
	m_Triangles.Add( Triangle( FirstVert+0, FirstVert+3, FirstVert+7 ) );	
	m_Triangles.Add( Triangle( FirstVert+7, FirstVert+4, FirstVert+0 ) );

	//	bottom
	m_Triangles.Add( Triangle( FirstVert+4, FirstVert+5, FirstVert+6 ) );	
	m_Triangles.Add( Triangle( FirstVert+6, FirstVert+7, FirstVert+4 ) );

	OnPrimitivesChanged();
}


//-------------------------------------------------------
//	generate a sphere
//-------------------------------------------------------
void TLAsset::TMesh::GenerateSphere(float Radius,const float3& Center)
{
	TLMaths::TSphere Sphere;
	Sphere.Set( Center, Radius );

	GenerateSphere( Sphere );
}


//-------------------------------------------------------
//	generate a capsule
//-------------------------------------------------------
void TLAsset::TMesh::GenerateCapsule(float Radius,const float3& Start,const float3& End)
{
	TLMaths::TCapsule Capsule;
	Capsule.Set( Start, End, Radius );

	GenerateCapsule( Capsule );
}

//-------------------------------------------------------
//	generate a sphere
//-------------------------------------------------------
void TLAsset::TMesh::GenerateSphere(const TLMaths::TSphere& Sphere)
{
	Empty();

	//	generate section/detail from size of shape
	Type2<u32> Segments( 8, 8 );

	TArray<float3> Verts;
	TArray<float3> Normals;
	TArray<float2> TextureUV;
	TArray<Triangle> Triangles;
	Verts.SetSize( Segments.x * Segments.y );
	Normals.SetSize( Segments.x * Segments.y );
	TextureUV.SetSize( Segments.x * Segments.y );

	float2 Mult;
	Mult.x = 1.f / (float)( Segments.x-1 );
	Mult.y = 1.f / (float)( Segments.y-1 );
	
	for ( u32 y=0;	y<Segments.y-1;	y++ )
	{
		for ( u32 x=0;	x<Segments.x-1;	x++ )
		{
			float fx = (float)x * Mult.x;
			float fy = (float)y * Mult.y;
			float fx1 = (float)(x+1) * Mult.x;
			float fy1 = (float)(y+1) * Mult.y;
			
			int vi[4];
			vi[0] = (x+0) + ( (y+0)*Segments.x );
			vi[1] = (x+1) + ( (y+0)*Segments.x );
			vi[2] = (x+1) + ( (y+1)*Segments.x );
			vi[3] = (x+0) + ( (y+1)*Segments.x );

			float3& v1 = Verts[vi[0]];
			float3& v2 = Verts[vi[1]];
			float3& v3 = Verts[vi[2]];
			float3& v4 = Verts[vi[3]];
						
			v1.x = (float)sinf( fx * ( PI * 2.f )) * (float)sinf( fy * PI );
			v1.y = (float)cosf( fy * ( PI * 1.f ));
			v1.z = (float)cosf( fx * ( PI * 2.f )) * (float)sinf( fy * PI );

			v2.x = (float)sinf( fx1 * ( PI * 2.f )) * (float)sinf( fy * PI );
			v2.y = (float)cosf( fy * ( PI * 1.f ));
			v2.z = (float)cosf( fx1 * ( PI * 2.f )) * (float)sinf( fy * PI );

			v3.x = (float)sinf( fx1 * ( PI * 2.f )) * (float)sinf( fy1 * PI );
			v3.y = (float)cosf( fy1 * ( PI * 1.f ));
			v3.z = (float)cosf( fx1 * ( PI * 2.f )) * (float)sinf( fy1 * PI );

			v4.x = (float)sinf( fx * ( PI * 2.f )) * (float)sinf( fy1 * PI );
			v4.y = (float)cosf( fy1 * ( PI * 1.f ));
			v4.z = (float)cosf( fx * ( PI * 2.f )) * (float)sinf( fy1 * PI );

			Normals[vi[0]] = v1;
			Normals[vi[1]] = v2;
			Normals[vi[2]] = v3;
			Normals[vi[3]] = v4;

			TextureUV[vi[0]] = float2( fx, 1.f-fy );
			TextureUV[vi[1]] = float2( fx1, 1.f-fy );
			TextureUV[vi[2]] = float2( fx1, 1.f-fy1 );
			TextureUV[vi[3]] = float2( fx, 1.f-fy1 );

			v1 *= Sphere.GetRadius();
			v2 *= Sphere.GetRadius();
			v3 *= Sphere.GetRadius();
			v4 *= Sphere.GetRadius();

			v1 += Sphere.GetPos();
			v2 += Sphere.GetPos();
			v3 += Sphere.GetPos();
			v4 += Sphere.GetPos();

			Triangle Tri1( vi[0], vi[1], vi[3] );
			Triangle Tri2( vi[1], vi[2], vi[3] );

			m_Triangles.Add( Tri1 );
			m_Triangles.Add( Tri2 );
		}
	}
	
	m_Vertexes.Add( Verts );
	m_Triangles.Add( Triangles );

	OnPrimitivesChanged();
}


//-------------------------------------------------------
//	generate a capsule
//-------------------------------------------------------
void TLAsset::TMesh::GenerateCapsule(const TLMaths::TCapsule& Capsule)
{
	TLDebug_Break("todo");
	OnPrimitivesChanged();
}

//-------------------------------------------------------
//	generate a cube mesh
//-------------------------------------------------------
void TLAsset::TMesh::GenerateCube(float Scale)
{
	//	generate a box of this scale and create from that
	float HalfScale = Scale * 0.5f;
	TLMaths::TBox Cube( float3(-HalfScale,-HalfScale,-HalfScale), float3(HalfScale,HalfScale,HalfScale) );

	GenerateCube( Cube );
}


//-------------------------------------------------------
//	generate a line
//-------------------------------------------------------
void TLAsset::TMesh::GenerateLine(const TLMaths::TLine& LineShape,const TColour& ColourStart,const TColour& ColourEnd)
{
	//	add vertexes
	s32 StartVertex = AddVertex( LineShape.GetStart(), ColourStart );
	s32 EndVertex = AddVertex( LineShape.GetEnd(), ColourEnd );

	//	add line
	Line* pLine = m_Lines.AddNew();
	pLine->Add( StartVertex );
	pLine->Add( EndVertex );
}

//-------------------------------------------------------
//	generate a line
//-------------------------------------------------------
void TLAsset::TMesh::GenerateLine(const TArray<float3>& LinePoints,const TColour& Colour)
{
	if ( LinePoints.GetSize() == 0 )
		return;

	//	add line
	Line* pLine = m_Lines.AddNew();
	
	for ( u32 i=0;	i<LinePoints.GetSize();	i++ )
	{
		s32 VertexIndex = AddVertex( LinePoints[i], Colour );
		pLine->Add( (u16)VertexIndex );
	}
}


//-------------------------------------------------------
//	create colours for each vertex
//-------------------------------------------------------
void TLAsset::TMesh::GenerateRainbowColours()
{
	//	generate colours
	m_Colours.Empty();

	//	... for each vertex
	for ( u32 v=0;	v<m_Vertexes.GetSize();	v++ )
	{
		m_Colours.Add( TColour::Debug_GetColour( v ) );
	}
}


//-------------------------------------------------------
//	load asset data out binary data
//-------------------------------------------------------
SyncBool TLAsset::TMesh::ImportData(TBinaryTree& Data)		
{
	Data.ImportArrays( "Verts", m_Vertexes );
	Data.ImportArrays( "Colrs", m_Colours );

	Data.ImportArrays( "Tris", m_Triangles );
	Data.ImportArrays( "TStrp", m_Tristrips );
	Data.ImportArrays( "TFans", m_Trifans );
	Data.ImportArrays( "Lines", m_Lines );

	OnPrimitivesChanged();

	//	import bounds
	TLMaths::TBox& BoundsBox = GetBoundsBox();
	Data.ImportData( "BondB", BoundsBox );

	TLMaths::TSphere& BoundsSphere = GetBoundsSphere();
	Data.ImportData( "BondS", BoundsSphere );

	TLMaths::TCapsule& BoundsCapsule = GetBoundsCapsule();
	Data.ImportData( "BondC", BoundsCapsule );

	//	read flags
	if ( !Data.ImportData( "Flags", m_Flags.GetData() ) )
	{
		//	no flags in file, calc some on import
		CalcHasAlpha();
	}

	//	read datums
	TPtrArray<TBinaryTree> DatumChildrenData;
	Data.GetChildren("Datum", DatumChildrenData );
	for ( u32 i=0;	i<DatumChildrenData.GetSize();	i++ )
	{
		if ( !ImportDatum( *DatumChildrenData[i] ) )
			return SyncFalse;
	}

	//	store off any data we haven't read to keep this data intact
	ImportUnknownData( Data );

	return SyncTrue;
}


//-------------------------------------------------------
//	save asset data to binary data
//-------------------------------------------------------
SyncBool TLAsset::TMesh::ExportData(TBinaryTree& Data)				
{	
	Data.ExportArray( "Verts", m_Vertexes );
	Data.ExportArray( "Colrs", m_Colours );

	Data.ExportArray( "Tris", m_Triangles );
	Data.ExportArray( "TStrp", m_Tristrips );
	Data.ExportArray( "TFans", m_Trifans );
	Data.ExportArray( "Lines", m_Lines );

	//	export bounds if valid
	TLMaths::TBox& BoundsBox = GetBoundsBox();
	if ( BoundsBox.IsValid() )
		Data.ExportData( "BondB", BoundsBox );

	TLMaths::TSphere& BoundsSphere = GetBoundsSphere();
	if ( BoundsSphere.IsValid() )
		Data.ExportData( "BondS", BoundsSphere );

	TLMaths::TCapsule& BoundsCapsule = GetBoundsCapsule();
	if ( BoundsCapsule.IsValid() )
		Data.ExportData( "BondC", BoundsCapsule );

	//	export flags
	Data.ExportData( "Flags", m_Flags.GetData() );

	//	export datums
	for ( u32 i=0;	i<m_Datums.GetSize();	i++ )
	{
		TLKeyArray::TPair<TRef,TPtr<TLMaths::TShape> >& Datum = m_Datums.GetPairAt(i);

		//	create child data
		TPtr<TBinaryTree>& DatumData = Data.AddChild("Datum");

		//	export
		if ( !ExportDatum( *DatumData, Datum.m_Key, Datum.m_Item ) )
			return SyncFalse;
	}

	//	write back any data we didn't recognise
	ExportUnknownData( Data );

	return SyncTrue;
}	


//-------------------------------------------------------
//	calculate bounds box if invalid
//-------------------------------------------------------
TLMaths::TBox& TLAsset::TMesh::CalcBoundsBox(Bool ForceCalc)
{
	//	force recalculation
	if ( ForceCalc )
		m_BoundsBox.SetInvalid();

	//	is valid, just return it
	if ( m_BoundsBox.IsValid() )
		return m_BoundsBox;
	
	//	initialise to invalid, then accumulate from verts
	m_BoundsBox.SetInvalid();

	m_BoundsBox.Accumulate( m_Vertexes );

	//	todo: only apply to line verts
	if ( GetLines().GetSize() && m_LineWidth > 0.f )
	{
		m_BoundsBox.GetMin() -= float3( m_LineWidth, m_LineWidth, m_LineWidth );
		m_BoundsBox.GetMax() += float3( m_LineWidth, m_LineWidth, m_LineWidth );
	}

	//	debug the bounds we created
	/*
#ifdef _DEBUG
	float3 BoxSize = m_BoundsBox.GetSize();
	TTempString DebugString("Created bounds box for ");
	GetAssetRef().GetString( DebugString );
	DebugString.Appendf(". w: %.2f h:%.2f d:%.2f", BoxSize.x, BoxSize.y, BoxSize.z );
	TLDebug_Print( DebugString );
#endif
	*/

	return m_BoundsBox;
}


//-------------------------------------------------------
//	calculate bounds sphere if invalid
//-------------------------------------------------------
TLMaths::TSphere& TLAsset::TMesh::CalcBoundsSphere(Bool ForceCalc)
{
	//	force recalculation
	if ( ForceCalc )
		m_BoundsSphere.SetInvalid();

	//	is valid, just return it
	if ( m_BoundsSphere.IsValid() )
		return m_BoundsSphere;
	
	//	initialise to invalid, then accumulate from verts
	m_BoundsSphere.SetInvalid();

	m_BoundsSphere.Accumulate( CalcBoundsBox() );
/*
	m_BoundsSphere.Accumulate( m_Vertexes );

	//	todo: only apply to line verts
	if ( GetLines().GetSize() && m_LineWidth > 0.f )
	{
		m_BoundsSphere.GetRadius() += m_LineWidth;
	}
*/
	return m_BoundsSphere;
}


//-------------------------------------------------------
//	calculate bounds capsule if invalid
//-------------------------------------------------------
TLMaths::TCapsule& TLAsset::TMesh::CalcBoundsCapsule(Bool ForceCalc)
{
	//	force recalculation
	if ( ForceCalc )
		m_BoundsCapsule.SetInvalid();

	//	is valid, just return it
	if ( m_BoundsCapsule.IsValid() )
		return m_BoundsCapsule;
	
	//	initialise to invalid, then accumulate from verts
	m_BoundsCapsule.SetInvalid();

	m_BoundsCapsule.Accumulate( m_Vertexes );

	return m_BoundsCapsule;
}


//-------------------------------------------------------
//	scale all vertexes
//-------------------------------------------------------
void TLAsset::TMesh::ScaleVerts(const float3& Scale,u16 FirstVert,s32 LastVert)
{
	if ( Scale == float3(1.f, 1.f, 1.f ) )
		return;

	//	invalidate bounds box
	SetBoundsBoxInvalid();
	
	if ( LastVert == -1 )
		LastVert = m_Vertexes.GetLastIndex();

	//	scale verts
	for ( s32 v=FirstVert;	v<=LastVert;	v++ )
	{
		float3& Vert = m_Vertexes[v];
		Vert *= Scale;
	}
}


//-------------------------------------------------------
//	move all verts
//-------------------------------------------------------
void TLAsset::TMesh::MoveVerts(const float3& Movement,u16 FirstVert,s32 LastVert)
{
	//	move bounds box if valid
	if ( m_BoundsBox.IsValid() )
	{
		m_BoundsBox.GetMin() += Movement;
		m_BoundsBox.GetMax() += Movement;
	}

	if ( LastVert == -1 )
		LastVert = m_Vertexes.GetLastIndex();

	//	move verts
	for ( s32 v=FirstVert;	v<=LastVert;	v++ )
	{
		float3& Vert = m_Vertexes[v];
		Vert += Movement;
	}
}


//-------------------------------------------------------
//	add vertex to the list, makes up normals and colours if required
//-------------------------------------------------------
s32 TLAsset::TMesh::AddVertex(const float3& VertexPos)
{
	if ( m_Vertexes.GetSize() >= 0xffff )
	{
		TLDebug_Break("Mesh has reached it's max (u16) number of vertexes");
		return -1;
	}

	//	need to supply a colour so use other func with a default colour
	if ( m_Colours.GetSize() )
	{
		return AddVertex( VertexPos, g_DefaultVertexColour );
	}

	return m_Vertexes.Add( VertexPos );
}


//-------------------------------------------------------
//	add vertex with colour to the list, pad normals and colours if previously didnt exist
//-------------------------------------------------------
s32 TLAsset::TMesh::AddVertex(const float3& VertexPos,const TColour& Colour)
{
	if ( m_Vertexes.GetSize() >= 0xffff )
	{
		TLDebug_Break("Mesh has reached it's max (u16) number of vertexes");
		return -1;
	}

	//	don't have colours yet, so pad up the buffer with a default colour
	if ( m_Colours.GetSize() < m_Vertexes.GetSize() )
	{
		m_Colours.SetAllocSize( m_Vertexes.GetSize() + 1 );
		m_Colours.SetAll( g_DefaultVertexColour );
	}
	
	//	add vertex and colour
	s32 VertIndex = m_Vertexes.Add( VertexPos );
	if ( VertIndex == -1 )
		return -1;

	//	add colour
	m_Colours.Add( Colour );

	//	if this colour has alpha, enable the alpha flag
	if ( Colour.IsTransparent() )
		m_Flags.Set( MeshFlag_HasAlpha );

	return VertIndex;
}


//-------------------------------------------------------------------
//	find all uses of OldVertexIndex in polygons and swap them for NewVertexIndex 
//-------------------------------------------------------------------
Bool TLAsset::TMesh::ReplaceVertex(u16 OldVertexIndex,u16 NewVertexIndex)
{
	s32 i;
	Bool ChangedPolygon = FALSE;

	//	remove and correct polygons using this index

	for ( i=m_Triangles.GetLastIndex();	i>=0;	i-- )
	{
		Triangle& t = m_Triangles[i];

		//	correct indexes
		if ( t.x == OldVertexIndex )	
		{
			t.x = NewVertexIndex;	
			ChangedPolygon = TRUE;	
		}
		
		if ( t.y == OldVertexIndex )	
		{
			t.y = NewVertexIndex;	
			ChangedPolygon = TRUE;	
		}
		
		if ( t.z == OldVertexIndex )	
		{	
			t.z = NewVertexIndex;	
			ChangedPolygon = TRUE;	
		}
	}

	for ( i=m_Tristrips.GetLastIndex();	i>=0;	i-- )
	{
		Tristrip& Polygon = m_Tristrips[i];

		//	correct/remove indexes in this tristrip
		for ( s32 n=Polygon.GetLastIndex();	n>=0;	n-- )
		{
			if ( Polygon[n] == OldVertexIndex )
			{
				Polygon[n] = NewVertexIndex;
				ChangedPolygon = TRUE;	
			}
		}
	}

	for ( i=m_Trifans.GetLastIndex();	i>=0;	i-- )
	{
		Trifan& Polygon = m_Trifans[i];

		//	correct/remove indexes in this tristrip
		for ( s32 n=Polygon.GetLastIndex();	n>=0;	n-- )
		{
			if ( Polygon[n] == OldVertexIndex )
			{
				Polygon[n] = NewVertexIndex;
				ChangedPolygon = TRUE;	
			}
		}
	}

	
	for ( i=m_Lines.GetLastIndex();	i>=0;	i-- )
	{
		Line& Polygon = m_Lines[i];

		//	correct/remove indexes in this tristrip
		for ( s32 n=Polygon.GetLastIndex();	n>=0;	n-- )
		{
			if ( Polygon[n] == OldVertexIndex )
			{
				Polygon[n] = NewVertexIndex;
				ChangedPolygon = TRUE;	
			}
		}
	}

	if ( ChangedPolygon )
		OnPrimitivesChanged();

	return ChangedPolygon;
}


	
//-------------------------------------------------------------------
//	remove a vertex, remove it's colour, remove any polygons that use 
//	this vertex, and correct the vertex indexes in polygons (anything > VI needs reducing)
//	returns if any changes to polygons made
//-------------------------------------------------------------------
Bool TLAsset::TMesh::RemoveVertex(u16 VertexIndex)
{
	//	out of range
	if ( VertexIndex > m_Vertexes.GetSize() )
	{
		TLDebug_Break("Trying to remove vertex from mesh out of range");
		return FALSE;
	}

	u32 OldLastIndex = m_Vertexes.GetLastIndex();

	//	remove vertex and colour
	m_Vertexes.RemoveAt( VertexIndex );
	if ( m_Colours.GetSize() )
		m_Colours.RemoveAt( VertexIndex );

	s32 i;
	Bool ChangedPolygon = FALSE;


	//	remove and correct polygons using this index
	for ( i=m_Triangles.GetLastIndex();	i>=0;	i-- )
	{
		Triangle& t = m_Triangles[i];

		//	remove if it was using this vertex
		Bool Remove = FALSE;
		Remove |= ( t.x == VertexIndex );
		Remove |= ( t.y == VertexIndex );
		Remove |= ( t.z == VertexIndex );
		
		if ( Remove )
		{
			ChangedPolygon |= TRUE;
			m_Triangles.RemoveAt(i);	
			continue;	
		}

		//	correct indexes
		if ( t.x > VertexIndex )	{	t.x--;	}
		if ( t.y > VertexIndex )	{	t.y--;	}
		if ( t.z > VertexIndex )	{	t.z--;	}
	}

	for ( i=m_Tristrips.GetLastIndex();	i>=0;	i-- )
	{
		Tristrip& Polygon = m_Tristrips[i];

		//	correct/remove indexes in this tristrip
		for ( s32 n=Polygon.GetLastIndex();	n>=0;	n-- )
		{
			if ( Polygon[n] == VertexIndex )
			{
				ChangedPolygon |= TRUE;
				Polygon.RemoveAt(n);
			}
			else if ( Polygon[n] > VertexIndex )
			{
				Polygon[n]--;
			}
		}

		//	not enough left in this tristrip to form a triangle
		if ( Polygon.GetSize() < 3 )
		{
			ChangedPolygon |= TRUE;
			m_Tristrips.RemoveAt(i);
		}
	}

	for ( i=m_Trifans.GetLastIndex();	i>=0;	i-- )
	{
		Trifan& Polygon = m_Trifans[i];

		//	correct/remove indexes in this tristrip
		for ( s32 n=Polygon.GetLastIndex();	n>=0;	n-- )
		{
			if ( Polygon[n] == VertexIndex )
			{
				ChangedPolygon |= TRUE;
				Polygon.RemoveAt(n);
			}
			else if ( Polygon[n] > VertexIndex )
			{
				Polygon[n]--;
			}
		}

		//	not enough left in this trifan to form a triangle
		if ( Polygon.GetSize() < 3 )
		{
			ChangedPolygon |= TRUE;
			m_Trifans.RemoveAt(i);
		}
	}

	
	for ( i=m_Lines.GetLastIndex();	i>=0;	i-- )
	{
		Line& Polygon = m_Lines[i];

		//	correct/remove indexes in this tristrip
		for ( s32 n=Polygon.GetLastIndex();	n>=0;	n-- )
		{
			if ( Polygon[n] == VertexIndex )
			{
				ChangedPolygon |= TRUE;
				Polygon.RemoveAt(n);
			}
			else if ( Polygon[n] > VertexIndex )
			{
				Polygon[n]--;
			}
		}

		//	not enough left in this linestrip for a line
		if ( Polygon.GetSize() < 2 )
		{
			ChangedPolygon |= TRUE;
			m_Lines.RemoveAt(i);
		}
	}

	if ( ChangedPolygon )
		OnPrimitivesChanged();

	return ChangedPolygon;
}


//-------------------------------------------------
//	loop through colours to check if we have any alpha colours in the verts
//-------------------------------------------------
void TLAsset::TMesh::CalcHasAlpha()
{
	Bool HasAlpha = FALSE;

	for ( u32 i=0;	i<m_Colours.GetSize();	i++ )
	{
		if ( m_Colours[i].IsTransparent() )
		{
			HasAlpha = TRUE;
			break;
		}
	}

	//	set/clear flag
	GetFlags().Set( MeshFlag_HasAlpha, HasAlpha );
}


//-------------------------------------------------
//	remove any lines/parts of linestrips with these two points next to each other. returns TRUE if any changes made
//-------------------------------------------------
Bool TLAsset::TMesh::RemoveLine(u32 VertexIndexA,u32 VertexIndexB)
{
	Bool ChangedPolygon = FALSE;

	for ( s32 i=m_Lines.GetLastIndex();	i>=0;	i-- )
	{
		Line& Polygon = m_Lines[i];

		Bool PrevMatchA = FALSE;
		Bool PrevMatchB = FALSE;

		//	correct/remove indexes in this tristrip
		for ( s32 n=Polygon.GetLastIndex();	n>=0;	n-- )
		{
			//	matched a vertex
			if ( Polygon[n] == VertexIndexA )
			{
				//	previous vertex was a match to the other vertex
				if ( PrevMatchB )
				{
					ChangedPolygon |= TRUE;
					Polygon.RemoveAt(n);	//	remove this from line
					Polygon.RemoveAt(n+1);	//	remove prev too
				}
				PrevMatchA = TRUE;
				PrevMatchB = FALSE;
				continue;
			}

			//	matched a vertex
			if ( Polygon[n] == VertexIndexB )
			{
				//	previous vertex was a match to the other vertex
				if ( PrevMatchA )
				{
					ChangedPolygon |= TRUE;
					Polygon.RemoveAt(n);	//	remove this from line
					Polygon.RemoveAt(n+1);	//	remove prev too
				}
				PrevMatchB = TRUE;
				PrevMatchA = FALSE;
				continue;
			}
		}

		//	not enough left in this linestrip for a line
		if ( Polygon.GetSize() < 2 )
		{
			ChangedPolygon |= TRUE;
			m_Lines.RemoveAt(i);
		}
	}

	if ( ChangedPolygon )
		OnPrimitivesChanged();

	return ChangedPolygon;
}


//--------------------------------------------------------
//	turn an outline of points into a quad/tri-strip
//--------------------------------------------------------
Bool TLAsset::TMesh::GenerateQuad(const TArray<float3>& Outline,const TColour& VertexColour)
{
	if ( Outline.GetSize() != 4 )
	{
		TLDebug_Break("Trying to generate quad with an outline without 4 points");
		return FALSE;
	}

	return GenerateQuad( Outline[0], Outline[1], Outline[2], Outline[3], VertexColour );
}


//--------------------------------------------------------
//	turn an outline of points into a quad/tri-strip
//--------------------------------------------------------
Bool TLAsset::TMesh::GenerateQuad(const TArray<float3>& Outline,const TArray<TColour>& Colours)
{
	if ( Outline.GetSize() != 4 || Colours.GetSize() != 4 )
	{
		TLDebug_Break("Trying to generate quad with an outline without 4 points");
		return FALSE;
	}

	return GenerateQuad( Outline[0], Outline[1], Outline[2], Outline[3], Colours[0], Colours[1], Colours[2], Colours[3] );
}


//--------------------------------------------------------
//	turn an outline of points into a quad/tri-strip
//--------------------------------------------------------
Bool TLAsset::TMesh::GenerateQuad(const float3& OutlineA,const float3& OutlineB,const float3& OutlineC,const float3& OutlineD,const TColour& VertexColour)
{
	return GenerateQuad( OutlineA, OutlineB, OutlineC, OutlineD, VertexColour, VertexColour, VertexColour, VertexColour );
}


//--------------------------------------------------------
//	turn an outline of points into a quad/tri-strip
//--------------------------------------------------------
Bool TLAsset::TMesh::GenerateQuad(const float3& OutlineA,const float3& OutlineB,const float3& OutlineC,const float3& OutlineD,const TColour& ColourA,const TColour& ColourB,const TColour& ColourC,const TColour& ColourD)
{
	TFixedArray<s32,4> VertIndexes;
	VertIndexes[0] = AddVertex( OutlineA, ColourA );
	VertIndexes[1] = AddVertex( OutlineB, ColourB );
	VertIndexes[2] = AddVertex( OutlineC, ColourC );
	VertIndexes[3] = AddVertex( OutlineD, ColourD );
	
	return GenerateQuad( VertIndexes );
}



//--------------------------------------------------------
//	turn an outline of points into a quad/tri-strip
//--------------------------------------------------------
Bool TLAsset::TMesh::GenerateQuad(const float3& OutlineA,const float3& OutlineB,const float3& OutlineC,const float3& OutlineD)
{
	TFixedArray<s32,4> VertIndexes;
	VertIndexes[0] = AddVertex( OutlineA );
	VertIndexes[1] = AddVertex( OutlineB );
	VertIndexes[2] = AddVertex( OutlineC );
	VertIndexes[3] = AddVertex( OutlineD );

	return GenerateQuad( VertIndexes );
}



//--------------------------------------------------------
//	turn an outline of points into a quad/tri-strip
//--------------------------------------------------------
Bool TLAsset::TMesh::GenerateQuad(const TFixedArray<s32,4> OutlineVertIndexes)
{
#ifdef GENERATE_QUADS_AS_TRIANGLES
	//	create tri strip
	Triangle* pTriangleA = m_Triangles.AddNew();
	pTriangleA->x = OutlineVertIndexes[0];
	pTriangleA->y = OutlineVertIndexes[1];
	pTriangleA->z = OutlineVertIndexes[2];

	Triangle* pTriangleB = m_Triangles.AddNew();
	pTriangleB->x = OutlineVertIndexes[2];
	pTriangleB->y = OutlineVertIndexes[3];
	pTriangleB->z = OutlineVertIndexes[0];

#else
	//	create tri strip
	Tristrip* pNewStrip = m_Tristrips.AddNew();
	if ( !pNewStrip )
		return FALSE;

	//	add verts
	//	gr: note: ORDER IS 0,1,3,2 for tri strip!
	pNewStrip->Add( OutlineVertIndexes[0] );
	pNewStrip->Add( OutlineVertIndexes[1] );
	pNewStrip->Add( OutlineVertIndexes[3] );
	pNewStrip->Add( OutlineVertIndexes[2] );
#endif

	OnPrimitivesChanged();

	return TRUE;
}



//--------------------------------------------------------
//	wholly copy this mesh's contents
//--------------------------------------------------------
void TLAsset::TMesh::Copy(const TMesh* pMesh)
{
	m_Vertexes.Copy( pMesh->m_Vertexes );
	m_Colours.Copy( pMesh->m_Colours );

	m_Triangles.Copy( pMesh->m_Triangles );
	m_Tristrips.Copy( pMesh->m_Tristrips );
	m_Trifans.Copy( pMesh->m_Trifans );
	m_Triangles.Copy( pMesh->m_Triangles );
	m_Lines.Copy( pMesh->m_Lines );
	
	OnPrimitivesChanged();

	m_BoundsBox = pMesh->m_BoundsBox;
	m_BoundsSphere = pMesh->m_BoundsSphere;
	m_BoundsCapsule = pMesh->m_BoundsCapsule;

	m_LineWidth = pMesh->m_LineWidth;
	m_Flags = pMesh->m_Flags;
}


//--------------------------------------------------------
//	multiply all colours by this colour
//--------------------------------------------------------
void TLAsset::TMesh::ColoursMult(const TColour& Colour)
{
	for ( u32 i=0;	i<m_Colours.GetSize();	i++ )
	{
		m_Colours[i] *= Colour;
	}
}


//--------------------------------------------------------
//	just a check to make sure the integrety of all the polygons indexes are valid
//--------------------------------------------------------
void TLAsset::TMesh::OnPrimitivesChanged()
{
#ifdef _DEBUG
	u32 VertexCount = m_Vertexes.GetSize();
	u32 i;

	//	remove and correct polygons using this index
	for ( i=0;	i<m_Triangles.GetSize();	i++ )
	{
		Triangle& t = m_Triangles[i];
		TLDebug_CheckIndex( t.x, VertexCount );
		TLDebug_CheckIndex( t.y, VertexCount );
		TLDebug_CheckIndex( t.z, VertexCount );
	}

	for ( i=0;	i<m_Tristrips.GetSize();	i++ )
	{
		Tristrip& Polygon = m_Tristrips[i];
		for ( u32 n=0;	n<Polygon.GetSize();	n++ )
		{
			TLDebug_CheckIndex( Polygon[n], VertexCount );
		}
	}

	for ( i=0;	i<m_Trifans.GetSize();	i++ )
	{
		Trifan& Polygon = m_Trifans[i];
		for ( u32 n=0;	n<Polygon.GetSize();	n++ )
		{
			TLDebug_CheckIndex( Polygon[n], VertexCount );
		}
	}
	
	for ( i=0;	i<m_Lines.GetSize();	i++ )
	{
		Line& Polygon = m_Lines[i];
		for ( u32 n=0;	n<Polygon.GetSize();	n++ )
		{
			TLDebug_CheckIndex( Polygon[n], VertexCount );
		}
	}
	
#endif
}


//--------------------------------------------------------
//	
//--------------------------------------------------------
Bool TLAsset::TMesh::ImportDatum(TBinaryTree& Data)
{
	Data.ResetReadPos();

	//	read ref
	TRef DatumRef;
	if ( !Data.Read( DatumRef ) )
		return FALSE;

	//	read type of shape
	TRef ShapeRef;
	if ( !Data.Read( ShapeRef ) )
		return FALSE;

	//	import specific shape
	if ( ShapeRef == "sph2" )
	{
		TLMaths::TSphere2D Shape;
		if ( !Data.Read( Shape ) )
			return FALSE;
		TPtr<TLMaths::TShape> pShape = new TLMaths::TShapeSphere2D( Shape );
		AddDatum( DatumRef, pShape );
		return TRUE;
	}
	else if ( ShapeRef == "sph" )
	{
		TLMaths::TSphere Shape;
		if ( !Data.Read( Shape ) )
			return FALSE;
		TPtr<TLMaths::TShape> pShape = new TLMaths::TShapeSphere( Shape );
		AddDatum( DatumRef, pShape );
		return TRUE;
	}

	TLDebug_Break("Unknown datum shape type");
	return FALSE;
}

//--------------------------------------------------------
//	
//--------------------------------------------------------
Bool TLAsset::TMesh::ExportDatum(TBinaryTree& Data,TRefRef DatumRef,TPtr<TLMaths::TShape>& pShape)
{
	TRefRef ShapeRef = pShape->GetShapeType();

	//	write ref
	Data.Write( DatumRef );

	//	write type of shape
	Data.Write( ShapeRef );

	//	export specific data
	if ( ShapeRef == "sph2" )
	{
		const TLMaths::TSphere2D& Shape = pShape.GetObject<TLMaths::TShapeSphere2D>()->GetSphere();
		Data.Write( Shape );
		return TRUE;
	}
	else if ( ShapeRef == "sph" )
	{
		const TLMaths::TSphere& Shape = pShape.GetObject<TLMaths::TShapeSphere>()->GetSphere();
		Data.Write( Shape );
		return TRUE;
	}

	TLDebug_Break("Unknown datum shape type");
	return FALSE;
}


//--------------------------------------------------------
//	
//--------------------------------------------------------
Bool TLAsset::TMesh::CreateDatum(const TArray<float3>& PolygonPoints,TRefRef DatumRef,TRefRef DatumShapeType)
{
	if ( PolygonPoints.GetSize() < 2 )
	{
		TLDebug_Break("Not enough points to generate datum from");
		return FALSE;
	}

	if ( DatumShapeType == "sph2" )
	{
		//	get box of points for extents
		TLMaths::TBox2D Box;
		Box.Accumulate( PolygonPoints );

		//	work out radius (NOT the diagonal!)
		float HalfWidth = Box.GetHalfWidth();
		float HalfHeight = Box.GetHalfHeight();
		float Radius = (HalfWidth>HalfHeight) ? HalfWidth : HalfHeight;

		//	make up sphere and shape
		TLMaths::TSphere2D Sphere( Box.GetCenter(), Radius );
		TPtr<TLMaths::TShape> pSphereShape = new TLMaths::TShapeSphere2D( Sphere );

		//	add datum
		AddDatum( DatumRef, pSphereShape );
		return TRUE;
	}
	else if ( DatumShapeType == "sph" )
	{
		//	get box of points for extents
		TLMaths::TBox Box;
		Box.Accumulate( PolygonPoints );

		//	work out radius (NOT the diagonal!)
		float3 HalfSize = Box.GetSize() * 0.5f;
		float Radius = HalfSize.x;
		if ( HalfSize.y > Radius )	Radius = HalfSize.y;
		if ( HalfSize.z > Radius )	Radius = HalfSize.z;

		//	make up sphere and shape
		TLMaths::TSphere Sphere( Box.GetCenter(), Radius );
		TPtr<TLMaths::TShape> pSphereShape = new TLMaths::TShapeSphere( Sphere );

		//	add datum
		AddDatum( DatumRef, pSphereShape );
		return TRUE;
	}

	TLDebug_Break("Unknown shape type");
	return FALSE;
}
