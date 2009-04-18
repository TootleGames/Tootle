#include "TMesh.h"
#include <TootleCore/TBinaryTree.h>
#include <TootleMaths/TShapeSphere.h>
#include <TootleMaths/TShapeOblong.h>
#include <TootleMaths/TShapeCapsule.h>

#ifdef _DEBUG
//#define DEBUG_CHECK_PRIMITIVES
#endif

#define LINE_PADDING_HALF			(1.f)

#define GENERATE_QUADS_AS_TRIANGLES			//	if not defined, tri-strips are created

#define MIN_TRISTRIP_SIZE			10		//	if a tri-strip has less than this number of points (tris=points-2) then degenerate it to triangles instead. 10 is 4 faces around a cube (ie. a building :)
//	triangle vertex(index) count = 3 * T
//	tri-strip vertex(index) count = T + 2
//	therefore;
//	1triange = 3(tri) = 3(strip)
//	2triange = 6(tri) = 4(strip)
//	4triange = 12(tri) = 6(strip)
//	10triange = 30(tri) = 12(strip)


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
	m_Colours24.Empty();
	m_Colours32.Empty();
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
void TLAsset::TMesh::GenerateQuad(const TLMaths::TBox2D& Box,const TColour* pColour,Bool GenerateUV,float z)
{
	float3 TopLeft = Box.GetMin().xyz( z );
	float3 BottomRight = Box.GetMax().xyz( z );
	float3 TopRight( BottomRight.x, TopLeft.y, z );
	float3 BottomLeft( TopLeft.x, BottomRight.y, z );

	//	generate quad with outline
	GenerateQuad( TopLeft, TopRight, BottomRight, BottomLeft, pColour, GenerateUV );
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
void TLAsset::TMesh::GenerateSphere(const TLMaths::TSphere& Sphere,const TColour* pColour,Bool GenerateUVs)
{
	u32 SegmentCount = TLAsset::TLMesh::GetSphereSegmentCount( Sphere.GetRadius() );
	Type2<u32> Segments( SegmentCount, SegmentCount );

	TArray<float3> Verts;
	TArray<float3> Normals;
	TArray<float2> TextureUV;

	Verts.SetSize( Segments.x * Segments.y );
	Normals.SetSize( Segments.x * Segments.y );
	TextureUV.SetSize( Segments.x * Segments.y );

	u16 FirstVertex = m_Vertexes.GetSize();


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
						
			v1.x = TLMaths::Sinf( fx * ( PI * 2.f )) * TLMaths::Sinf( fy * PI );
			v1.y = TLMaths::Cosf( fy * ( PI * 1.f ));
			v1.z = TLMaths::Cosf( fx * ( PI * 2.f )) * TLMaths::Sinf( fy * PI );

			v2.x = TLMaths::Sinf( fx1 * ( PI * 2.f )) * TLMaths::Sinf( fy * PI );
			v2.y = TLMaths::Cosf( fy * ( PI * 1.f ));
			v2.z = TLMaths::Cosf( fx1 * ( PI * 2.f )) * TLMaths::Sinf( fy * PI );

			v3.x = TLMaths::Sinf( fx1 * ( PI * 2.f )) * TLMaths::Sinf( fy1 * PI );
			v3.y = TLMaths::Cosf( fy1 * ( PI * 1.f ));
			v3.z = TLMaths::Cosf( fx1 * ( PI * 2.f )) * TLMaths::Sinf( fy1 * PI );

			v4.x = TLMaths::Sinf( fx * ( PI * 2.f )) * TLMaths::Sinf( fy1 * PI );
			v4.y = TLMaths::Cosf( fy1 * ( PI * 1.f ));
			v4.z = TLMaths::Cosf( fx * ( PI * 2.f )) * TLMaths::Sinf( fy1 * PI );

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

	
	//	add vertexes FIRST as colours are padded/culled to the vertex count
	m_Vertexes.Add( Verts );

	//	correct colour usage and add colours
	pColour = GetGenerationColour( pColour );
	if ( pColour )
	{
		AddColour( *pColour, Segments.x * Segments.y );
	}
	
	//	if the mesh already has UV's then we must add them
	if ( GenerateUVs || (HasUVs() == SyncTrue) )
	{
		m_UVs.Add( TextureUV );
		PadUVs();
	}


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
void TLAsset::TMesh::GenerateCapsule(const TLMaths::TCapsule2D& Capsule,const TColour* pColour,float z)
{
	const TLMaths::TLine2D& CapsuleLine = Capsule.GetLine();
	TFixedArray<float3,2> Line(2);
	Line[0] = CapsuleLine.GetStart().xyz( z );
	Line[1] = CapsuleLine.GetEnd().xyz( z );

	TFixedArray<float3,2> LineOutside;
	TFixedArray<float3,2> LineInside;

	TLMaths::ExpandLineStrip( Line, Capsule.GetRadius()*2.f, LineOutside, LineInside );

	//	draw the capsule's outlines
	GenerateQuad( LineOutside[0], LineOutside[1], LineInside[1], LineInside[0], pColour );

	//	draw a sphere at each end of the capsule
	GenerateSphere( TLMaths::TSphere2D( CapsuleLine.GetStart(), Capsule.GetRadius() ), pColour, z );
	GenerateSphere( TLMaths::TSphere2D( CapsuleLine.GetEnd(), Capsule.GetRadius() ), pColour, z );
}


//-------------------------------------------------------
//	generate a capsule
//-------------------------------------------------------
void TLAsset::TMesh::GenerateCapsuleOutline(const TLMaths::TCapsule2D& Capsule,const TColour* pColour,float z)
{
	const TLMaths::TLine2D& CapsuleLine = Capsule.GetLine();
	TFixedArray<float3,2> Line(2);
	Line[0] = CapsuleLine.GetStart().xyz( z );
	Line[1] = CapsuleLine.GetEnd().xyz( z );

	TFixedArray<float3,2> LineOutside;
	TFixedArray<float3,2> LineInside;

	TLMaths::ExpandLineStrip( Line, Capsule.GetRadius()*2.f, LineOutside, LineInside );

	//	draw the capsule's outlines
	GenerateLine( LineOutside, pColour );
	GenerateLine( LineInside, pColour );

	//	draw a sphere at each end of the capsule
	GenerateSphereOutline( TLMaths::TSphere2D( CapsuleLine.GetStart(), Capsule.GetRadius() ), pColour, z );
	GenerateSphereOutline( TLMaths::TSphere2D( CapsuleLine.GetEnd(), Capsule.GetRadius() ), pColour, z );
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
	m_Colours24.Empty();
	m_Colours32.Empty();

	//	... for each vertex
	for ( u32 v=0;	v<m_Vertexes.GetSize();	v++ )
	{
		AddColour( TLColour::Debug_GetColour( v ) );
	}
}


//-------------------------------------------------------
//	load asset data out binary data
//-------------------------------------------------------
SyncBool TLAsset::TMesh::ImportData(TBinaryTree& Data)		
{
	Data.ImportArrays( "Verts", m_Vertexes );
	Data.ImportArrays( "Colrs", m_Colours );
	Data.ImportArrays( "Col24", m_Colours24 );
	Data.ImportArrays( "Col32", m_Colours32 );
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
	Data.ExportArray( "Col24", m_Colours24 );
	Data.ExportArray( "Col32", m_Colours32 );
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
		TRefRef DatumRef = m_Datums.GetKeyAt(i);
		TPtr<TLMaths::TShape>& pDatumShape = m_Datums.GetItemAt(i);

		//	create child data
		TPtr<TBinaryTree>& DatumData = Data.AddChild("Datum");

		//	export
		if ( !ExportDatum( *DatumData, DatumRef, *pDatumShape ) )
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
		AddColour( *pColour );
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
Bool TLAsset::TMesh::RemoveVertex(u16 VertexIndex,Bool CheckUsage)
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
	RemoveColourAt( VertexIndex );
	if ( m_UVs.GetSize() )
		m_UVs.RemoveAt( VertexIndex );

	s32 i;
	Bool ChangedPolygon = FALSE;

	if ( CheckUsage )
	{
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

	
void TLAsset::TMesh::RemoveTriangle(u16 TriangleIndex,Bool RemoveVertexes,Bool CheckVertexUsage)
{
	//	check valid triangle
	if ( TriangleIndex >= m_Triangles.GetSize() )
	{
		TLDebug_Break("Tried to remove invalid triangle index");
		return;
	}

	//	remove vertexes first
	if ( RemoveVertexes )
	{
		Triangle& Tri = m_Triangles[TriangleIndex];

		//	sort triangle vertex order so that we remove the largest index first without having to modify all the other vertex indexes
		if ( !CheckVertexUsage )
			Tri.SortComponents();
			
		RemoveVertex( Tri.z, CheckVertexUsage );
		RemoveVertex( Tri.y, CheckVertexUsage );
		RemoveVertex( Tri.x, CheckVertexUsage );
	}

	//	remove triangle
	m_Triangles.RemoveAt( TriangleIndex );

	//	check geometry
	OnPrimitivesChanged();
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
	
	TFixedArray<u16,4> VertIndexes;
	VertIndexes.Add( AddVertex( Outline[0], Colours[0] ) );
	VertIndexes.Add( AddVertex( Outline[1], Colours[1] ) );
	VertIndexes.Add( AddVertex( Outline[2], Colours[2] ) );
	VertIndexes.Add( AddVertex( Outline[3], Colours[3] ) );

	return GenerateQuad( VertIndexes );
}


//--------------------------------------------------------
//	turn an outline of points into a quad/tri-strip
//--------------------------------------------------------
Bool TLAsset::TMesh::GenerateQuad(const float3& OutlineA,const float3& OutlineB,const float3& OutlineC,const float3& OutlineD,const TColour* pColour,Bool GenerateUVs)
{
	TFixedArray<u16,4> VertIndexes;

	if ( GenerateUVs )
	{
		VertIndexes.Add( AddVertex( OutlineA, pColour, float2(0.f, 0.f) ) );
		VertIndexes.Add( AddVertex( OutlineB, pColour, float2(1.f, 0.f) ) );
		VertIndexes.Add( AddVertex( OutlineC, pColour, float2(1.f, 1.f) ) );
		VertIndexes.Add( AddVertex( OutlineD, pColour, float2(0.f, 1.f) ) );
	}
	else
	{
		VertIndexes.Add( AddVertex( OutlineA, pColour ) );
		VertIndexes.Add( AddVertex( OutlineB, pColour ) );
		VertIndexes.Add( AddVertex( OutlineC, pColour ) );
		VertIndexes.Add( AddVertex( OutlineD, pColour ) );
	}

	return GenerateQuad( VertIndexes );
}



//--------------------------------------------------------
//	turn an outline of points into a quad/tri-strip
//--------------------------------------------------------
Bool TLAsset::TMesh::GenerateQuad(const TArray<u16>& OutlineVertIndexes)
{
#ifdef GENERATE_QUADS_AS_TRIANGLES
	
	GenerateTriangle( OutlineVertIndexes[0], OutlineVertIndexes[1], OutlineVertIndexes[2] );
	GenerateTriangle( OutlineVertIndexes[2], OutlineVertIndexes[3], OutlineVertIndexes[0] );

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
//	generate triangle
//--------------------------------------------------------
void TLAsset::TMesh::GenerateTriangle(u16 VertA,u16 VertB,u16 VertC)
{
	Triangle* pTriangle = m_Triangles.AddNew();
	pTriangle->x = VertA;
	pTriangle->y = VertB;
	pTriangle->z = VertC;
}


//--------------------------------------------------------
//	generate triangle strips from points. this is IN TRISTRIP ORDER
//--------------------------------------------------------
void TLAsset::TMesh::GenerateTristrip(const TArray<u16>& TristripVerts)
{
	//	too small, better off as a bunch of triangles...
	if ( TristripVerts.GetSize() < MIN_TRISTRIP_SIZE )
	{
		for ( u32 i=2;	i<TristripVerts.GetSize();	i++ )
		{
			GenerateTriangle( TristripVerts[i-2], TristripVerts[i-1], TristripVerts[i] );
		}
	}
	else
	{
		Tristrip* pTristrip = m_Tristrips.AddNew();
		pTristrip->Add( TristripVerts );

		OnPrimitivesChanged();
	}

}


//--------------------------------------------------------
//	merge othermesh into this mesh - add verts, primitives, datums etc
//--------------------------------------------------------
void TLAsset::TMesh::Merge(const TMesh& OtherMesh)
{
	u32 FirstVertex = m_Vertexes.GetSize();
	SyncBool HadColours = HasColours();
	SyncBool HadUVs = HasUVs();
	
	//	copy vertexes
	m_Vertexes.Add( OtherMesh.GetVertexes() );
	OnVertexesChanged();

	//	didnt have colours/uvs, now does, pad
	if ( HadColours == SyncFalse && OtherMesh.HasColours() )
		PadColours();
	if ( HadUVs == SyncFalse && OtherMesh.HasUVs() )
		PadUVs();

	//	add colours and uvs
	m_Colours.Add( OtherMesh.GetColours() );
	m_Colours24.Add( OtherMesh.GetColours24() );
	m_Colours32.Add( OtherMesh.GetColours32() );
	m_UVs.Add( OtherMesh.GetUVs() );

	//	pad up colours/uvs if the old mesh had colours
	if ( HadColours == SyncTrue )
		PadColours();
	if ( HadUVs == SyncTrue )
		PadUVs();

	//	add geometry - but need to offset all the geometry indexes
	AddTriangles( OtherMesh.GetTriangles(), FirstVertex );
	AddTristrips( OtherMesh.GetTristrips(), FirstVertex );
	AddTrifans( OtherMesh.GetTrifans(), FirstVertex );
	AddLines( OtherMesh.GetLines(), FirstVertex );
	AddLinestrips( OtherMesh.GetLinestrips(), FirstVertex );

	//	merge flags
//	m_Flags.Or( OtherMesh.GetFlags() );

	//	merge line widths...
	if ( OtherMesh.GetLineWidth() > m_LineWidth )
		m_LineWidth = OtherMesh.GetLineWidth();

	//	add datums - dont add duplicates
	for ( u32 d=0;	d<OtherMesh.m_Datums.GetSize();	d++ )
	{
		const TPtr<TLMaths::TShape>& pOtherDataum = OtherMesh.m_Datums.ElementAt(d);
		TRefRef OtherDatumRef = OtherMesh.m_Datums.GetKeyAt( d );
		m_Datums.Add( OtherDatumRef, pOtherDataum, FALSE );
	}
}


//--------------------------------------------------------
//	multiply all colours by this colour
//--------------------------------------------------------
void TLAsset::TMesh::ColoursMult(const TColour& MultColour)
{
	//	make sure other colour arrays are allocated
	m_Colours24.SetSize( m_Colours.GetSize() );
	m_Colours32.SetSize( m_Colours.GetSize() );

	for ( u32 i=0;	i<m_Colours.GetSize();	i++ )
	{
		TColour& Colour = m_Colours[i];
		Colour *= MultColour;
		m_Colours24[i] = Colour;
		m_Colours32[i] = Colour;
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

	//	import shape
	TPtr<TLMaths::TShape> pShape = TLMaths::ImportShapeData( Data );
	if ( !pShape )
		return FALSE;

	//	add datum
	AddDatum( DatumRef, pShape );

	return TRUE;
}

//--------------------------------------------------------
//	
//--------------------------------------------------------
Bool TLAsset::TMesh::ExportDatum(TBinaryTree& Data,TRefRef DatumRef,TLMaths::TShape& Shape)
{
	TRefRef ShapeRef = Shape.GetShapeType();

	//	write ref
	Data.Write( DatumRef );

	//	write shape
	return TLMaths::ExportShapeData( Data, Shape );
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
		TPtr<TLMaths::TShape> pSphereShape = new TLMaths::TShapeSphere2D( Box );

		//	add datum
		AddDatum( DatumRef, pSphereShape );
		return TRUE;
	}
	else if ( DatumShapeType == TLMaths::TSphere::GetTypeRef() )
	{
		//	get box of points for extents
		TLMaths::TBox Box;
		Box.Accumulate( PolygonPoints );
		TPtr<TLMaths::TShape> pSphereShape = new TLMaths::TShapeSphere( Box );

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
		TPtr<TLMaths::TShape> pShape = new TLMaths::TShapeCapsule2D( Box );

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



//--------------------------------------------------------
//	add a colour to the colour array
//--------------------------------------------------------
void TLAsset::TMesh::AddColour(const TColour& Colour,u16 Count)
{
	m_Colours.AddAllocSize( Count );
	for ( u32 i=0;	i<Count;	i++ )
		m_Colours.Add( Colour );

	//	update alpha status
	if ( Colour.IsTransparent() )
		m_Flags.Set( MeshFlag_HasAlpha );

	//	re-align colour buffer and add other colour types
	PadColours();
}



//--------------------------------------------------------
//	add a colour to the colour array
//--------------------------------------------------------
void TLAsset::TMesh::RemoveColourAt(u16 VertexIndex)
{
	if ( VertexIndex < m_Colours.GetSize() )
		m_Colours.RemoveAt( VertexIndex );

	if ( VertexIndex < m_Colours24.GetSize() )
		m_Colours24.RemoveAt( VertexIndex );

	if ( VertexIndex < m_Colours32.GetSize() )
		m_Colours32.RemoveAt( VertexIndex );
}


//--------------------------------------------------------
//	ensure number of colours matches number of vertexes
//--------------------------------------------------------
void TLAsset::TMesh::PadColours()
{
	//	prealloc data
	if ( m_Colours.GetAllocSize() < m_Vertexes.GetSize() )
		m_Colours.SetAllocSize( m_Vertexes.GetSize() );

	//	add default colours
	while ( m_Colours.GetSize() < m_Vertexes.GetSize() )
	{
		m_Colours.Add( TLAsset::g_DefaultVertexColour );
	}

	//	change size of colours if different to vertex size (ie. we have too many colours then shrink)
	m_Colours.SetSize( m_Vertexes.GetSize() );

	u32 i;

	//	pad out other colour types
	for ( i=m_Colours24.GetSize();	i<m_Colours.GetSize();	i++ )
		m_Colours24.Add( m_Colours[i] );

	for ( i=m_Colours32.GetSize();	i<m_Colours.GetSize();	i++ )
		m_Colours32.Add( m_Colours[i] );

	//	shrink as required
	m_Colours24.SetSize( m_Colours.GetSize() );
	m_Colours32.SetSize( m_Colours.GetSize() );
}


//--------------------------------------------------------
//	ensure number of UVs matches number of vertexes
//--------------------------------------------------------
void TLAsset::TMesh::PadUVs()
{
	while ( m_UVs.GetSize() < m_Vertexes.GetSize() )
	{
		m_UVs.Add( TLAsset::g_DefaultVertexUV );
	}

	//	change size of colours if different to vertex size (ie. we have too many colours)
	m_UVs.SetSize( m_Vertexes.GetSize() );
}




void TLAsset::TMesh::AddTriangles(const TArray<Triangle>& OtherPolygons,u32 OffsetVertexIndex)
{
	s32 FirstPoly = m_Triangles.Add( OtherPolygons );

	if ( OffsetVertexIndex > 0 )
	{
		for ( u32 i=FirstPoly;	i<m_Triangles.GetSize();	i++ )
		{
			Triangle& Polygon = m_Triangles[i];
			Polygon.x += OffsetVertexIndex;
			Polygon.y += OffsetVertexIndex;
			Polygon.z += OffsetVertexIndex;
		}
	}
}

void TLAsset::TMesh::AddTristrips(const TArray<Tristrip>& OtherPolygons,u32 OffsetVertexIndex)
{
	s32 FirstPoly = m_Tristrips.Add( OtherPolygons );

	if ( OffsetVertexIndex > 0 )
	{
		for ( u32 i=FirstPoly;	i<m_Tristrips.GetSize();	i++ )
		{
			Tristrip& Polygon = m_Tristrips[i];
			for ( u32 p=0;	p<Polygon.GetSize();	p++ )
			{
				Polygon[p] += OffsetVertexIndex;
			}
		}
	}
}

void TLAsset::TMesh::AddTrifans(const TArray<Trifan>& OtherPolygons,u32 OffsetVertexIndex)
{
	s32 FirstPoly = m_Trifans.Add( OtherPolygons );

	if ( OffsetVertexIndex > 0 )
	{
		for ( u32 i=FirstPoly;	i<m_Trifans.GetSize();	i++ )
		{
			Trifan& Polygon = m_Trifans[i];
			for ( u32 p=0;	p<Polygon.GetSize();	p++ )
			{
				Polygon[p] += OffsetVertexIndex;
			}
		}
	}
}

void TLAsset::TMesh::AddLinestrips(const TArray<Linestrip>& OtherPolygons,u32 OffsetVertexIndex)
{
	s32 FirstPoly = m_Linestrips.Add( OtherPolygons );

	if ( OffsetVertexIndex > 0 )
	{
		for ( u32 i=FirstPoly;	i<m_Linestrips.GetSize();	i++ )
		{
			Linestrip& Polygon = m_Linestrips[i];
			for ( u32 p=0;	p<Polygon.GetSize();	p++ )
			{
				Polygon[p] += OffsetVertexIndex;
			}
		}
	}
}

void TLAsset::TMesh::AddLines(const TArray<Line>& OtherPolygons,u32 OffsetVertexIndex)
{
	s32 FirstPoly = m_Lines.Add( OtherPolygons );

	if ( OffsetVertexIndex > 0 )
	{
		for ( u32 i=FirstPoly;	i<m_Lines.GetSize();	i++ )
		{
			Line& Polygon = m_Lines[i];
			Polygon.x += OffsetVertexIndex;
			Polygon.y += OffsetVertexIndex;
		}
	}
}

