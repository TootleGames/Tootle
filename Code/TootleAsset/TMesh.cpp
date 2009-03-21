#include "TMesh.h"
#include <TootleCore/TBinaryTree.h>
#include <TootleMaths/TOblong.h>
#include <TootleMaths/TCapsule.h>

#ifdef _DEBUG
//#define DEBUG_CHECK_PRIMITIVES
#endif

#define LINE_PADDING_HALF			(1.f)

//#define GENERATE_QUADS_AS_TRIANGLES	//	if not defined, tri-strips are created

#define SPHERE_SEGMENT_SCALE		0.3f	//	* Radius = segment count. 
#define SPHERE_SEGMENT_MIN			((u32)6)
#define SPHERE_SEGMENT_MAX			(20)


namespace TLAsset
{
	const TColour	g_DefaultVertexColour( 0.5f, 0.5f, 0.5f, 1.f );
	const float2	g_DefaultVertexUV( 0.5f, 0.5f );

	namespace TLMesh
	{
		FORCEINLINE u32			GetSphereSegmentCount(float Radius);
	}
}



FORCEINLINE u32 TLAsset::TLMesh::GetSphereSegmentCount(float Radius)
{
	u32 SegmentCount = (u32)(Radius * SPHERE_SEGMENT_SCALE);
	if ( SegmentCount < SPHERE_SEGMENT_MIN )
		return SPHERE_SEGMENT_MIN;
	
	if ( SegmentCount > SPHERE_SEGMENT_MAX )
		return SPHERE_SEGMENT_MAX;

	return SegmentCount;
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
	m_UVs.Empty();

	m_Triangles.Empty();
	m_Tristrips.Empty();
	m_Trifans.Empty();
	m_Linestrips.Empty();
	m_Lines.Empty();
	
	m_Datums.Empty();

	SetBoundsInvalid();
}


//-------------------------------------------------------
//	generate a square mesh from a 2d box
//-------------------------------------------------------
void TLAsset::TMesh::GenerateQuad(const TLMaths::TBox2D& Box,const TColour* pColour,float z)
{
	float3 TopLeft = Box.GetMin().xyz( z );
	float3 BottomRight = Box.GetMin().xyz( z );
	float3 TopRight( BottomRight.x, TopLeft.y, z );
	float3 BottomLeft( TopLeft.x, BottomRight.y, z );

	//	generate quad with outline
	GenerateQuad( TopLeft, TopRight, BottomRight, BottomLeft, pColour );
}


//-------------------------------------------------------
//	generate a square mesh around a point
//-------------------------------------------------------
void TLAsset::TMesh::GenerateQuad(const float2& Center,float Size,const TColour* pColour,float z)
{
	float HalfSize = Size * 0.5f;
	float3 TopLeft		( Center.x - HalfSize, Center.y - HalfSize, z );
	float3 TopRight		( Center.x + HalfSize, Center.y - HalfSize, z );
	float3 BottomLeft	( Center.x - HalfSize, Center.y + HalfSize, z );
	float3 BottomRight	( Center.x + HalfSize, Center.y + HalfSize, z );

	//	generate quad with outline
	GenerateQuad( TopLeft, TopRight, BottomRight, BottomLeft, pColour );
}


//-------------------------------------------------------
//	generate a box's outline
//-------------------------------------------------------
void TLAsset::TMesh::GenerateQuadOutline(const TLMaths::TBox2D& Box,const TColour* pColour,float z)
{
	TFixedArray<float3,4> Outline(4);
	float3& TopLeft = Outline[0];
	float3& BottomRight = Outline[2];
	float3& TopRight = Outline[1];
	float3& BottomLeft = Outline[3];

	TopLeft = Box.GetMin().xyz( z );
	BottomRight = Box.GetMax().xyz( z );
	TopRight = float3( BottomRight.x, TopLeft.y, z );
	BottomLeft = float3( TopLeft.x, BottomRight.y, z );
	
	//	generate oultine
	GenerateLine( Outline, pColour );
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
void TLAsset::TMesh::GenerateCapsule(float Radius,const float3& Start,const float3& End,const TColour& Colour)
{
	TLMaths::TCapsule Capsule;
	Capsule.Set( Start, End, Radius );

	GenerateCapsule( Capsule, Colour );
}

//-------------------------------------------------------
//	generate a sphere
//-------------------------------------------------------
void TLAsset::TMesh::GenerateSphere(const TLMaths::TSphere& Sphere,const TColour* pColour)
{
	u32 SegmentCount = TLAsset::TLMesh::GetSphereSegmentCount( Sphere.GetRadius() );
	Type2<u32> Segments( SegmentCount, SegmentCount );

	TArray<float3> Verts;
	TArray<float3> Normals;
	TArray<float2> TextureUV;
	TArray<TColour> Colours;

	Verts.SetSize( Segments.x * Segments.y );
	Normals.SetSize( Segments.x * Segments.y );
	TextureUV.SetSize( Segments.x * Segments.y );

	u16 FirstVertex = m_Vertexes.GetSize();

	//	correct colour usage
	pColour = GetGenerationColour( pColour );

	//	alloc colours
	if ( pColour )
	{
		Colours.SetSize( Segments.x * Segments.y );
		Colours.SetAll( *pColour );
	}


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

			Triangle Tri1( vi[0] + FirstVertex, vi[1] + FirstVertex, vi[3] + FirstVertex );
			Triangle Tri2( vi[1] + FirstVertex, vi[2] + FirstVertex, vi[3] + FirstVertex );

			m_Triangles.Add( Tri1 );
			m_Triangles.Add( Tri2 );
		}
	}

	
	if ( pColour )
		m_Colours.Add( Colours );
	m_Vertexes.Add( Verts );

	OnPrimitivesChanged();
}


//-------------------------------------------------------
//	generate a 2D sphere
//-------------------------------------------------------
void TLAsset::TMesh::GenerateSphereOutline(const TLMaths::TSphere2D& Sphere,const TColour* pColour,float z)
{
	u32 SegmentCount = TLAsset::TLMesh::GetSphereSegmentCount( Sphere.GetRadius() );

	TFixedArray<float3,100> Outline;

	//	create linestrip points
	float AngleStep = 360.f / (float)SegmentCount;
	for ( float AngleDeg=0.f;	AngleDeg<360.f;	AngleDeg+=AngleStep )
	{
		float AngleRad = TLMaths::TAngle::DegreesToRadians( AngleDeg );
		
		float3 Pos( Sphere.GetPos().xyz( z ) );
		
		Pos.x += cosf( AngleRad ) * Sphere.GetRadius();
		Pos.y += sinf( AngleRad ) * Sphere.GetRadius();

		Outline.Add( Pos );
	}

	//	complete the loop
	Outline.Add( Outline[0] );

	//	create the geometry
	GenerateLine( Outline, pColour );
}


//-------------------------------------------------------
//	generate a 2D sphere
//-------------------------------------------------------
void TLAsset::TMesh::GenerateSphere(const TLMaths::TSphere2D& Sphere,const TColour* pColour,float z)
{
	u32 SegmentCount = TLAsset::TLMesh::GetSphereSegmentCount( Sphere.GetRadius() );

	TColour TempColour(1.f,1.f,1.f,1.f);

	TFixedArray<u16,100> OutlineVerts;

	float3 Center3 = Sphere.GetPos().xyz( z );
	u16 CenterVert = AddVertex( Center3, pColour );

	//	create outline points
	float AngleStep = 360.f / (float)SegmentCount;
	for ( float AngleDeg=0.f;	AngleDeg<360.f;	AngleDeg+=AngleStep )
	{
		float AngleRad = TLMaths::TAngle::DegreesToRadians( AngleDeg );
		
		float3 Pos( Sphere.GetPos().xyz( z ) );
		
		Pos.x += cosf( AngleRad ) * Sphere.GetRadius();
		Pos.y += sinf( AngleRad ) * Sphere.GetRadius();

		u16 Vertex = AddVertex( Pos, pColour );
		s32 VertexIndex = OutlineVerts.Add( Vertex );

		//	make triangles when we have at least 2 outline points
		if ( OutlineVerts.GetSize() <= 1 )
			continue;

		TLAsset::TMesh::Triangle Triangle( OutlineVerts[VertexIndex-1], OutlineVerts[VertexIndex], CenterVert );
		m_Triangles.Add( Triangle );
	}

	//	complete the loop by linking first and last vertexes
	TLAsset::TMesh::Triangle Triangle( OutlineVerts[0], OutlineVerts.ElementLast(), CenterVert );
	m_Triangles.Add( Triangle );

	OnPrimitivesChanged();
}


//-------------------------------------------------------
//	generate a capsule
//-------------------------------------------------------
void TLAsset::TMesh::GenerateCapsule(const TLMaths::TCapsule2D& Capsule,const TColour& Colour,float z)
{
	const TLMaths::TLine2D& CapsuleLine = Capsule.GetLine();
	TFixedArray<float3,2> Line(2);
	Line[0] = CapsuleLine.GetStart().xyz( z );
	Line[1] = CapsuleLine.GetEnd().xyz( z );

	TFixedArray<float3,2> LineOutside;
	TFixedArray<float3,2> LineInside;

	TLMaths::ExpandLineStrip( Line, Capsule.GetRadius()*2.f, LineOutside, LineInside );

	//	draw the capsule's outlines
	GenerateLine( LineOutside, Colour );
	GenerateLine( LineInside, Colour );

	//	draw a sphere at each end of the capsule
	GenerateSphereOutline( TLMaths::TSphere2D( CapsuleLine.GetStart(), Capsule.GetRadius() ), &Colour, z );
	GenerateSphereOutline( TLMaths::TSphere2D( CapsuleLine.GetEnd(), Capsule.GetRadius() ), &Colour, z );
}


//-------------------------------------------------------
//	generate a capsule
//-------------------------------------------------------
void TLAsset::TMesh::GenerateCapsule(const TLMaths::TCapsule& Capsule,const TColour* pColour)
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
void TLAsset::TMesh::GenerateLine(const float3& LineStart,const float3& LineEnd,const TColour* pColourStart,const TColour* pColourEnd)
{
	//	add vertexes
	s32 StartVertex = AddVertex( LineStart, pColourStart );
	s32 EndVertex = AddVertex( LineEnd, pColourEnd );

	//	add line
	GenerateLine( StartVertex, EndVertex );
}


//-------------------------------------------------------
//	
//-------------------------------------------------------
void TLAsset::TMesh::GenerateLine(u16 StartVertex,u16 EndVertex)
{
	//	add line
	Line* pLine = m_Lines.AddNew();
	pLine->x = StartVertex;
	pLine->y = EndVertex;
}


//-------------------------------------------------------
//	generate a line
//-------------------------------------------------------
void TLAsset::TMesh::GenerateLine(const TArray<float3>& LinePoints,const TColour* pColour)
{
	if ( LinePoints.GetSize() < 2 )
	{
		TLDebug_Break("Line too short");
		return;
	}

	//	use simple line system rather than a strip when possible
	if ( LinePoints.GetSize() == 2 )
	{
		GenerateLine( LinePoints[0], LinePoints[1], pColour, pColour );
		return;
	}

	//	add line
	Linestrip* pLine = m_Linestrips.AddNew();

	//	pre-alloc data
	m_Vertexes.AddAllocSize( LinePoints.GetSize() );
	m_Colours.AddAllocSize( LinePoints.GetSize() );
	pLine->AddAllocSize( LinePoints.GetSize() );
	
	for ( u32 i=0;	i<LinePoints.GetSize();	i++ )
	{
		s32 VertexIndex = AddVertex( LinePoints[i], pColour );
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
	Data.ImportArrays( "UVs", m_UVs );

	Data.ImportArrays( "Tris", m_Triangles );
	Data.ImportArrays( "TStrp", m_Tristrips );
	Data.ImportArrays( "TFans", m_Trifans );
	Data.ImportArrays( "Lines", m_Linestrips );
	Data.ImportArrays( "SmpLines", m_Lines );

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
	Data.ExportArray( "UVs", m_UVs );

	Data.ExportArray( "Tris", m_Triangles );
	Data.ExportArray( "TStrp", m_Tristrips );
	Data.ExportArray( "TFans", m_Trifans );
	Data.ExportArray( "Lines", m_Linestrips );
	Data.ExportArray( "SmpLines", m_Lines );

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
	Bool HasLines = (GetLines().GetSize() + GetLinestrips().GetSize()) > 0;
	if ( HasLines && m_LineWidth > 0.f )
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
//	add vertex with colour to the list, pad normals and colours if previously didnt exist
//-------------------------------------------------------
s32 TLAsset::TMesh::AddVertex(const float3& VertexPos,const TColour* pColour,const float2* pUV)
{
	if ( m_Vertexes.GetSize() >= 0xffff )
	{
		TLDebug_Break("Mesh has reached it's max (u16) number of vertexes");
		return -1;
	}

	
	//	get a colour if we require it
	Bool ColourProvided = (pColour != NULL);
	pColour = GetGenerationColour( pColour );

	//	notification if we provided a colour... but cannot use it
	if ( !pColour && ColourProvided )
	{
		TLDebug_Warning("AddVertex's colour that was provided was ditched as the rest of the mesh has no colour");
	}

	
	//	get a colour if we require it
	pUV = GetGenerationUV( pUV );
	Bool UVProvided = (pUV != NULL);

	//	notification if we provided a colour... but cannot use it
	if ( !pUV && UVProvided )
	{
		TLDebug_Warning("AddVertex's UV that was provided was ditched as the rest of the mesh has no UV");
	}


	//	add vertex and colour
	s32 VertIndex = m_Vertexes.Add( VertexPos );
	if ( VertIndex == -1 )
		return -1;

	//	add colour
	if ( pColour )
	{
		m_Colours.Add( *pColour );

		//	if this colour has alpha, enable the alpha flag
		if ( pColour->IsTransparent() )
			m_Flags.Set( MeshFlag_HasAlpha );
	}

	//	add UV
	if ( pUV )
	{
		m_UVs.Add( *pUV );
	}

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

	
	for ( i=m_Linestrips.GetLastIndex();	i>=0;	i-- )
	{
		Linestrip& Polygon = m_Linestrips[i];

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

		if ( Polygon.x == OldVertexIndex )
		{
			Polygon.x = NewVertexIndex;
			ChangedPolygon = TRUE;	
		}

		if ( Polygon.y == OldVertexIndex )
		{
			Polygon.y = NewVertexIndex;
			ChangedPolygon = TRUE;	
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

	
	for ( i=m_Linestrips.GetLastIndex();	i>=0;	i-- )
	{
		Linestrip& Polygon = m_Linestrips[i];

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
			m_Linestrips.RemoveAt(i);
		}
	}


	for ( i=m_Lines.GetLastIndex();	i>=0;	i-- )
	{
		Line& Polygon = m_Lines[i];

		//	correct indexes in this line - if any removed we need to remove the line
		if ( Polygon.x == VertexIndex )
		{
			ChangedPolygon |= TRUE;
			m_Lines.RemoveAt(i);
			continue;
		}
		
		if ( Polygon.y == VertexIndex )
		{
			ChangedPolygon |= TRUE;
			m_Lines.RemoveAt(i);
			continue;
		}
		
		//	correct indexes
		if ( Polygon.x > VertexIndex )
		{
			ChangedPolygon |= TRUE;
			Polygon.x--;
		}

		if ( Polygon.y > VertexIndex )
		{
			ChangedPolygon |= TRUE;
			Polygon.y--;
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

	for ( s32 i=m_Linestrips.GetLastIndex();	i>=0;	i-- )
	{
		Linestrip& Polygon = m_Linestrips[i];

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
			m_Linestrips.RemoveAt(i);
		}
	}

	for ( s32 i=m_Lines.GetLastIndex();	i>=0;	i-- )
	{
		Line& Polygon = m_Lines[i];

		//	match
		if ( Polygon.x == VertexIndexA && Polygon.y == VertexIndexB )
		{
			ChangedPolygon |= TRUE;
			m_Lines.RemoveAt( i );
			continue;
		}

		if ( Polygon.x == VertexIndexB && Polygon.y == VertexIndexA )
		{
			ChangedPolygon |= TRUE;
			m_Lines.RemoveAt( i );
			continue;
		}
	}

	if ( ChangedPolygon )
		OnPrimitivesChanged();

	return ChangedPolygon;
}


//--------------------------------------------------------
//	turn an outline of points into a quad/tri-strip
//--------------------------------------------------------
Bool TLAsset::TMesh::GenerateQuad(const TArray<float3>& Outline,const TColour* pColour)
{
	if ( Outline.GetSize() != 4 )
	{
		TLDebug_Break("Trying to generate quad with an outline without 4 points");
		return FALSE;
	}

	return GenerateQuad( Outline[0], Outline[1], Outline[2], Outline[3], pColour );
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

	return GenerateQuad( Outline[0], Outline[1], Outline[2], Outline[3], &Colours[0], &Colours[1], &Colours[2], &Colours[3] );
}


//--------------------------------------------------------
//	turn an outline of points into a quad/tri-strip
//--------------------------------------------------------
Bool TLAsset::TMesh::GenerateQuad(const float3& OutlineA,const float3& OutlineB,const float3& OutlineC,const float3& OutlineD,const TColour* pColour)
{
	return GenerateQuad( OutlineA, OutlineB, OutlineC, OutlineD, pColour, pColour, pColour, pColour );
}


//--------------------------------------------------------
//	turn an outline of points into a quad/tri-strip
//--------------------------------------------------------
Bool TLAsset::TMesh::GenerateQuad(const float3& OutlineA,const float3& OutlineB,const float3& OutlineC,const float3& OutlineD,const TColour* pColourA,const TColour* pColourB,const TColour* pColourC,const TColour* pColourD)
{
	TFixedArray<u16,4> VertIndexes;
	VertIndexes.Add( AddVertex( OutlineA, pColourA ) );
	VertIndexes.Add( AddVertex( OutlineB, pColourB ) );
	VertIndexes.Add( AddVertex( OutlineC, pColourC ) );
	VertIndexes.Add( AddVertex( OutlineD, pColourD ) );
	
	return GenerateQuad( VertIndexes );
}



//--------------------------------------------------------
//	turn an outline of points into a quad/tri-strip
//--------------------------------------------------------
Bool TLAsset::TMesh::GenerateQuad(const TArray<u16>& OutlineVertIndexes)
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
	m_UVs.Copy( pMesh->m_UVs );

	m_Triangles.Copy( pMesh->m_Triangles );
	m_Tristrips.Copy( pMesh->m_Tristrips );
	m_Trifans.Copy( pMesh->m_Trifans );
	m_Triangles.Copy( pMesh->m_Triangles );
	m_Lines.Copy( pMesh->m_Lines );
	m_Linestrips.Copy( pMesh->m_Linestrips );
	
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
#ifdef DEBUG_CHECK_PRIMITIVES
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
	if ( ShapeRef == TLMaths::TSphere2D::GetTypeRef() )
	{
		TLMaths::TSphere2D Shape;
		if ( !Data.Read( Shape ) )
			return FALSE;
		TPtr<TLMaths::TShape> pShape = new TLMaths::TShapeSphere2D( Shape );
		AddDatum( DatumRef, pShape );
		return TRUE;
	}
	else if ( ShapeRef == TLMaths::TSphere::GetTypeRef() )
	{
		TLMaths::TSphere Shape;
		if ( !Data.Read( Shape ) )
			return FALSE;
		TPtr<TLMaths::TShape> pShape = new TLMaths::TShapeSphere( Shape );
		AddDatum( DatumRef, pShape );
		return TRUE;
	}
	else if ( ShapeRef == TLMaths::TOblong2D::GetTypeRef() )
	{
		TLMaths::TOblong2D Shape;
		Bool IsValid;
		if ( !Data.Read( IsValid ) )
			return FALSE;
		Shape.SetValid( IsValid );
		if ( !Data.ReadArray( Shape.GetBoxCorners() ) )
			return FALSE;
		TPtr<TLMaths::TShape> pShape = new TLMaths::TShapeOblong2D( Shape );
		AddDatum( DatumRef, pShape );
		return TRUE;
	}
	else if ( ShapeRef == TLMaths::TCapsule2D::GetTypeRef() )
	{
		TLMaths::TCapsule2D Shape;
		if ( !Data.Read( Shape ) )
			return FALSE;
		TPtr<TLMaths::TShape> pShape = new TLMaths::TShapeCapsule2D( Shape );
		AddDatum( DatumRef, pShape );
		return TRUE;
	}

#ifdef _DEBUG
	TTempString Debug_String("Unknown datum shape type ");
	ShapeRef.GetString( Debug_String );
	TLDebug_Break(Debug_String);
#endif

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
	if ( ShapeRef == TLMaths::TSphere2D::GetTypeRef() )
	{
		const TLMaths::TSphere2D& Shape = pShape.GetObject<TLMaths::TShapeSphere2D>()->GetSphere();
		Data.Write( Shape );
		return TRUE;
	}
	else if ( ShapeRef == TLMaths::TSphere::GetTypeRef() )
	{
		const TLMaths::TSphere& Shape = pShape.GetObject<TLMaths::TShapeSphere>()->GetSphere();
		Data.Write( Shape );
		return TRUE;
	}
	else if ( ShapeRef == TLMaths::TOblong2D::GetTypeRef() )
	{
		const TLMaths::TOblong2D& Shape = pShape.GetObject<TLMaths::TShapeOblong2D>()->GetOblong();
		Data.Write( Shape.IsValid() );
		Data.WriteArray( Shape.GetBoxCorners() );
		return TRUE;
	}
	else if ( ShapeRef == TLMaths::TCapsule2D::GetTypeRef() )
	{
		const TLMaths::TCapsule2D& Shape = pShape.GetObject<TLMaths::TShapeCapsule2D>()->GetCapsule();
		Data.Write( Shape );
		return TRUE;
	}

#ifdef _DEBUG
	TTempString Debug_String("Unknown datum shape type ");
	ShapeRef.GetString( Debug_String );
	TLDebug_Break(Debug_String);
#endif
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

	if ( DatumShapeType == TLMaths::TSphere2D::GetTypeRef() )
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
	else if ( DatumShapeType == TLMaths::TSphere::GetTypeRef() )
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
	else if ( DatumShapeType == TLMaths::TOblong2D::GetTypeRef() )
	{
		//	gr: currently, simple mehtod just works with 4-point meshes
		if ( PolygonPoints.GetSize() != 4 )
		{
			TLDebug_Break("Cannot create an oblong datum shape from a polygon/outline with other than 4 points");
			return FALSE;
		}

		TLMaths::TOblong2D Oblong;
		TFixedArray<float2,4>& Corners = Oblong.GetBoxCorners();
		Corners[0] = PolygonPoints[0].xy();
		Corners[1] = PolygonPoints[1].xy();
		Corners[2] = PolygonPoints[2].xy();
		Corners[3] = PolygonPoints[3].xy();

		//	set as explicitly valid
		Oblong.SetValid();

		TPtr<TLMaths::TShape> pShape = new TLMaths::TShapeOblong2D( Oblong );

		//	add datum
		AddDatum( DatumRef, pShape );
		return TRUE;
	}
	else if ( DatumShapeType == TLMaths::TCapsule2D::GetTypeRef() )
	{
		//	get box of points for extents
		TLMaths::TBox2D Box;
		Box.Accumulate( PolygonPoints );

		//	create capsule from box
		TLMaths::TCapsule2D Capsule;
		Capsule.Set( Box );

		TPtr<TLMaths::TShape> pShape = new TLMaths::TShapeCapsule2D( Capsule );

		//	add datum
		AddDatum( DatumRef, pShape );
		return TRUE;
	}

#ifdef _DEBUG
	TTempString Debug_String("Unknown datum shape type ");
	DatumShapeType.GetString( Debug_String );
	TLDebug_Break(Debug_String);
#endif

	return FALSE;
}



//--------------------------------------------------------
//	returns a valid colour pointer if we expect one (mesh already has colours) - NULL's if we dont have colours - returns the original pointer if we can have colours
//--------------------------------------------------------
const TColour* TLAsset::TMesh::GetGenerationColour(const TColour* pColour)
{
	//	mesh has no colours
	if ( m_Vertexes.GetSize() != m_Colours.GetSize() )
		return NULL;
	
	//	needs a colour and none provided
	if ( m_Colours.GetSize() && !pColour )	
	{
		return &TLAsset::g_DefaultVertexColour;
	}

	return pColour;
}


//--------------------------------------------------------
//	returns a valid UV pointer if we expect one (mesh already has UV) - NULL's if we dont have UV - returns the original pointer if we can have UVs
//--------------------------------------------------------
const float2* TLAsset::TMesh::GetGenerationUV(const float2* pUV)
{
	//	mesh has no uvs
	if ( m_Vertexes.GetSize() != m_UVs.GetSize() )
		return NULL;
	
	//	needs a colour and none provided
	if ( m_UVs.GetSize() && !pUV )	
	{
		return &TLAsset::g_DefaultVertexUV;
	}

	return pUV;
}
