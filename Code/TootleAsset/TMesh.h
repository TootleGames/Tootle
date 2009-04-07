/*------------------------------------------------------

	Mesh/model asset

-------------------------------------------------------*/
#pragma once

#include "TLAsset.h"
#include "TAsset.h"
#include <TootleCore/TFlags.h>
#include <TootleCore/TArray.h>
#include <TootleCore/TColour.h>
#include <TootleMaths/TBox.h>
#include <TootleMaths/TSphere.h>
#include <TootleMaths/TCapsule.h>
#include <TootleMaths/TShape.h>
#include <TootleCore/TKeyArray.h>


namespace TLAsset
{
	class TMesh;
	class TFont;

	const u32	g_MaxLineStripSize = 10;


	//	gr:	removed, I may implement again with a more vertex-description format (define byte offset of colour/pos/normal and type of colour/pos/normal)
	//	opengl best-performance vertex format info
	//	https://devforums.apple.com/thread/6475?tstart=15

	/*
	class TFixedVertex
	{
	public:
#if defined(TL_TARGET_IPOD)
		Type3<u32>		m_PositionFixed;
#endif
		
#if defined(TL_TARGET_PC)
		Type3<float>	m_Position;
#endif
		
		u32				m_Colour;
			
		FORCEINLINE	Bool	operator<(const TFixedVertex& Vertex) const	{	return FALSE;	}
	};
	*/
	
};



class TLAsset::TMesh : public TLAsset::TAsset
{
public:
	friend class TLAsset::TFont;
	typedef Type3<u16>	Triangle;
	typedef TArray<u16>	Tristrip;
	typedef TArray<u16>	Trifan;
	typedef TArray<u16>	Linestrip;
	typedef Type2<u16>	Line;

	enum TMeshFlags
	{
		MeshFlag_HasAlpha = 0,
	};

public:
	TMesh(const TRef& AssetRef);

	void					Empty();
	FORCEINLINE Bool		IsEmpty() const										{	return !( m_Vertexes.GetSize() || m_Colours.GetSize() || m_Triangles.GetSize() || m_Tristrips.GetSize() || m_Trifans.GetSize() || m_Lines.GetSize() );	}
	FORCEINLINE void		Copy(const TMesh& OtherMesh)						{	Empty();	Merge( OtherMesh );	}
	void					Merge(const TMesh& OtherMesh);						//	merge othermesh into this mesh - add verts, primitives, datums etc

	//	manipulation
	void					GenerateShape(const TLMaths::TBox& Box)				{	GenerateCube( Box );	}
	void					GenerateShape(const TLMaths::TSphere& Sphere)		{	GenerateSphere( Sphere );	}
	void					GenerateShape(const TLMaths::TCapsule& Capsule)		{	GenerateCapsule( Capsule );	}

	void					GenerateCube(float Scale);							//	generate a cube mesh
	void					GenerateCube(const TLMaths::TBox& Box);				//	generate a cube mesh from a math box

	void					GenerateSphere(float Radius,const float3& Center=float3(0,0,0));		//	generate a sphere
	void					GenerateSphere(const TLMaths::TSphere& Sphere,const TColour& Colour)		{	GenerateSphere( Sphere, &Colour );	}
	void					GenerateSphere(const TLMaths::TSphere& Sphere,const TColour* pColour=NULL);		//	generate a sphere
	void					GenerateSphereOutline(const TLMaths::TSphere2D& Sphere,const TColour* pColour=NULL,float z=0.f);	//	generate a 2D spehere (x/y) out of lines
	void					GenerateSphere(const TLMaths::TSphere2D& Sphere,const TColour* pColour=NULL,float z=0.f);	//	generate a 2D spehere (x/y)

	void					GenerateCapsule(float Radius,const float3& Start,const float3& End,const TColour& Colour);	//	generate a capsule
	void					GenerateCapsule(const TLMaths::TCapsule& Capsule,const TColour& Colour)			{	GenerateCapsule( Capsule, &Colour );	}
	void					GenerateCapsule(const TLMaths::TCapsule& Capsule,const TColour* pColour=NULL);	//	generate a capsule
	void					GenerateCapsule(const TLMaths::TCapsule2D& Capsule,const TColour& Colour,float z=0.f);	//	generate a capsule

	void					GenerateLine(const TLMaths::TLine& LineShape,const TColour& Colour)									{	GenerateLine( LineShape.GetStart(), LineShape.GetEnd(), &Colour, &Colour );	}
	void					GenerateLine(const TLMaths::TLine& LineShape,const TColour* pColour=NULL)							{	GenerateLine( LineShape.GetStart(), LineShape.GetEnd(), pColour, pColour );	}
	void					GenerateLine(const TLMaths::TLine& LineShape,const TColour* pColourStart,const TColour* pColourEnd)	{	GenerateLine( LineShape.GetStart(), LineShape.GetEnd(), pColourStart, pColourEnd );	}
	void					GenerateLine(const float3& LineStart,const float3& LineEnd,const TColour* pColour=NULL)				{	GenerateLine( LineStart, LineEnd, pColour, pColour );	}
	void					GenerateLine(const float3& LineStart,const float3& LineEnd,const TColour* pColourStart,const TColour* pColourEnd);	//	generate a line
	void					GenerateLine(u16 StartVertex,u16 EndVertex);
	void					GenerateLine(const TArray<float3>& LinePoints,const TColour& Colour)								{	GenerateLine( LinePoints, &Colour );	}
	void					GenerateLine(const TArray<float3>& LinePoints,const TColour* pColour=NULL);	//	generate a line

	void					GenerateQuad(const TLMaths::TBox2D& Box,const TColour* pColour=NULL,float z=0.f);		//	generate a square mesh from a 2d box
	void					GenerateQuad(const float2& Center,float Size,const TColour* pColour=NULL,float z=0.f);			//	generate a square mesh around a point
	Bool					GenerateQuad(const TArray<float3>& Outline,const TColour* pColour=NULL);			//	turn an outline of points into a quad/tri-strip
	Bool					GenerateQuad(const TArray<float3>& Outline,const TArray<TColour>& Colours);	//	turn an outline of points into a quad/tri-strip
	Bool					GenerateQuad(const float3& OutlineA,const float3& OutlineB,const float3& OutlineC,const float3& OutlineD,const TColour* pColour=NULL);	//	turn an outline of points into a quad/tri-strip
	Bool					GenerateQuad(const float3& OutlineA,const float3& OutlineB,const float3& OutlineC,const float3& OutlineD,const TColour* pColourA,const TColour* pColourB,const TColour* pColourC,const TColour* pColourD);	//	turn an outline of points into a quad/tri-strip
	Bool					GenerateQuad(const TArray<u16>& OutlineVertIndexes);		//	turn an outline of points into a quad/tri-strip
	void					GenerateQuadOutline(const TLMaths::TBox2D& Box,const TColour* pColour=NULL,float z=0.f);		//	generate a square mesh from a 2d box

	void					GenerateTriangle(u16 VertA,u16 VertB,u16 VertC);		//	generate triangle
	void					GenerateTristrip(const TArray<u16>& TristripVerts);		//	generate triangle strips from points. this is IN TRISTRIP ORDER

	void					GenerateRainbowColours();							//	create colours for each vertex

	//	vertex manipulation
	s32						AddVertex(const float3& VertexPos,const TColour* pColour=NULL,const float2* pUV=NULL);	//	add vertex to the list, makes up normals and colours if required
	FORCEINLINE s32			AddVertex(const float3& VertexPos,const TColour& Colour)		{	return AddVertex( VertexPos, &Colour );	}
	Bool					RemoveVertex(u16 VertexIndex,Bool CheckUsage=TRUE);				//	remove a vertex, remove it's colour, remove any polygons that use this vertex, and correct the vertex indexes in polygons (anything > VI needs reducing). returns if any changes to polygons made
	Bool					ReplaceVertex(u16 OldVertexIndex,u16 NewVertexIndex);			//	find all uses of OldVertexIndex in polygons and swap them for NewVertexIndex 
	void					RemoveTriangle(u16 TriangleIndex,Bool RemoveVertexes=TRUE,Bool CheckVertexUsage=TRUE);	//	
	inline void				ScaleVerts(float Scale,u16 FirstVert=0,s32 LastVert=-1)				{	ScaleVerts( float3( Scale, Scale, Scale ), FirstVert, LastVert );	}
	void					ScaleVerts(const float3& Scale,u16 FirstVert=0,s32 LastVert=-1);	//	scale all vertexes
	void					MoveVerts(const float3& Movement,u16 FirstVert=0,s32 LastVert=-1);	//	move all verts
	void					ColoursMult(const TColour& Colour);		//	multiply all colours by this colour

	//	data accessors
	TFlags<TMeshFlags,u8>&				GetFlags()								{	return m_Flags;	}
	const TFlags<TMeshFlags,u8>&		GetFlags() const						{	return m_Flags;	}
	FORCEINLINE Bool					HasAlpha() const						{	return m_Flags( MeshFlag_HasAlpha );	}
	FORCEINLINE SyncBool				HasColours() const						{	return (!m_Vertexes.GetSize()) ? SyncWait : ( !m_Colours.GetSize() ? SyncFalse : SyncTrue );	};	//	return if we have colours - wait means unknown as we have no vertexes
	FORCEINLINE SyncBool				HasColours24() const					{	return (!m_Vertexes.GetSize()) ? SyncWait : ( !m_Colours24.GetSize() ? SyncFalse : SyncTrue );	};	//	return if we have colours - wait means unknown as we have no vertexes
	FORCEINLINE SyncBool				HasColours32() const					{	return (!m_Vertexes.GetSize()) ? SyncWait : ( !m_Colours32.GetSize() ? SyncFalse : SyncTrue );	};	//	return if we have colours - wait means unknown as we have no vertexes
	FORCEINLINE SyncBool				HasUVs() const							{	return (!m_Vertexes.GetSize()) ? SyncWait : ( !m_UVs.GetSize() ? SyncFalse : SyncTrue );	};	//	return if we have colours - wait means unknown as we have no vertexes

	FORCEINLINE float3&						GetVertex(u32 VertIndex)			{	return m_Vertexes[VertIndex];	}
	FORCEINLINE float2&						GetVertexUV(u32 VertIndex)			{	return m_UVs[VertIndex];	}
	FORCEINLINE TColour&					GetVertexColour(u32 VertIndex)		{	return m_Colours[VertIndex];	}
	FORCEINLINE TArray<float3>&				GetVertexes() 						{	return m_Vertexes;	}
	FORCEINLINE const TArray<float3>&		GetVertexes() const					{	return m_Vertexes;	}
	FORCEINLINE u32							GetVertexCount() const				{	return m_Vertexes.GetSize(); }
	FORCEINLINE TArray<TColour>&			GetColours()						{	return m_Colours;	}
	FORCEINLINE const TArray<TColour>&		GetColours() const					{	return m_Colours;	}
	FORCEINLINE const TArray<TColour>*		GetColoursNotEmpty() const			{	return m_Colours.GetSize() ? &m_Colours : NULL;	}
	FORCEINLINE const TArray<TColour24>&	GetColours24() const				{	return m_Colours24;	}
	FORCEINLINE const TArray<TColour24>*	GetColours24NotEmpty() const		{	return m_Colours24.GetSize() ? &m_Colours24 : NULL;	}
	FORCEINLINE const TArray<TColour32>&	GetColours32() const				{	return m_Colours32;	}
	FORCEINLINE const TArray<TColour32>*	GetColours32NotEmpty() const		{	return m_Colours32.GetSize() ? &m_Colours32 : NULL;	}
	FORCEINLINE TArray<float2>&				GetUVs()							{	return m_UVs;	}
	FORCEINLINE const TArray<float2>&		GetUVs() const						{	return m_UVs;	}
	FORCEINLINE const TArray<float2>*		GetUVsNotEmpty() const				{	return m_UVs.GetSize() ? &m_UVs : NULL;	}

	TArray<Triangle>&					GetTriangles()						{	return m_Triangles;	}
	TArray<Tristrip>&					GetTristrips()						{	return m_Tristrips;	}
	TArray<Trifan>&						GetTrifans()						{	return m_Trifans;	}
	TArray<Linestrip>&					GetLinestrips()						{	return m_Linestrips;	}
	TArray<Line>&						GetLines()							{	return m_Lines;	}
	const TArray<Triangle>&				GetTriangles() const				{	return m_Triangles;	}
	const TArray<Tristrip>&				GetTristrips() const				{	return m_Tristrips;	}
	const TArray<Trifan>&				GetTrifans() const					{	return m_Trifans;	}
	const TArray<Linestrip>&			GetLinestrips() const				{	return m_Linestrips;	}
	const TArray<Line>&					GetLines() const					{	return m_Lines;	}
	const TArray<Triangle>*				GetTrianglesNotEmpty() const		{	return m_Triangles.GetSize() ? &m_Triangles : NULL;	}
	const TArray<Tristrip>*				GetTristripsNotEmpty() const		{	return m_Tristrips.GetSize() ? &m_Tristrips : NULL;	}
	const TArray<Trifan>*				GetTrifansNotEmpty() const			{	return m_Trifans.GetSize() ? &m_Trifans : NULL;	}
	const TArray<Linestrip>*			GetLinestripsNotEmpty() const		{	return m_Linestrips.GetSize() ? &m_Linestrips : NULL;	}
	const TArray<Line>*					GetLinesNotEmpty() const			{	return m_Lines.GetSize() ? &m_Lines : NULL;	}
	
	//	line stuff
	void					SetLineWidth(float LineWidth)		{	m_LineWidth = LineWidth;	TLMaths::Limit( m_LineWidth, 0.f, 100.f );	}
	float					GetLineWidth() const				{	return m_LineWidth;	}
	Bool					RemoveLine(u32 VertexIndexA,u32 VertexIndexB);	//	remove any lines/parts of linestrips with these two points next to each other. returns TRUE if any changes made

	//	bounds stuff
	void					SetBoundsInvalid()					{	SetBoundsBoxInvalid();		SetBoundsSphereInvalid();		SetBoundsCapsuleInvalid();	}
	void					CalcBounds(Bool ForceCalc=FALSE)	{	CalcBoundsBox(ForceCalc);	CalcBoundsSphere(ForceCalc);	CalcBoundsCapsule(ForceCalc);	}

	void					SetBoundsBoxInvalid()				{	m_BoundsBox.SetInvalid();	}
	TLMaths::TBox&			CalcBoundsBox(Bool ForceCalc=FALSE);	//	calculate bounds box if invalid
	TLMaths::TBox&			GetBoundsBox()						{	return m_BoundsBox;	}
	
	void					SetBoundsSphereInvalid()			{	m_BoundsSphere.SetInvalid();	}
	TLMaths::TSphere&		CalcBoundsSphere(Bool ForceCalc=FALSE);	//	calculate bounds Sphere if invalid
	TLMaths::TSphere&		GetBoundsSphere()					{	return m_BoundsSphere;	}
	
	void					SetBoundsCapsuleInvalid()			{	m_BoundsCapsule.SetInvalid();	}
	TLMaths::TCapsule&		CalcBoundsCapsule(Bool ForceCalc=FALSE);	//	calculate bounds box if invalid
	TLMaths::TCapsule&		GetBoundsCapsule()					{	return m_BoundsCapsule;	}

	//	datum access
	FORCEINLINE TPtr<TLMaths::TShape>&	AddDatum(TRefRef DatumRef,TPtr<TLMaths::TShape>& pShape);
	Bool					RemoveDatum(TRefRef DatumRef) 		{	return m_Datums.Remove( DatumRef );	}
	Bool					DatumExists(TRefRef DatumRef) const	{	return m_Datums.Exists( DatumRef );	}
	TPtr<TLMaths::TShape>&	GetDatum(TRefRef DatumRef)			{	return m_Datums.FindPtr( DatumRef );	}
	template<class SHAPETYPE>
	SHAPETYPE*				GetDatum(TRefRef DatumRef);
	Bool					CreateDatum(const TArray<float3>& PolygonPoints,TRefRef DatumRef,TRefRef DatumShapeType);
	
	void					OnVertexesChanged()					{	SetBoundsInvalid();	}
	void					OnPrimitivesChanged();				//	just a check to make sure the integrety of all the polygons indexes are valid

protected:
	virtual SyncBool		ImportData(TBinaryTree& Data);		//	load asset data out binary data
	virtual SyncBool		ExportData(TBinaryTree& Data);		//	save asset data to binary data

	Bool					ImportDatum(TBinaryTree& Data);
	Bool					ExportDatum(TBinaryTree& Data,TRefRef DatumRef,TPtr<TLMaths::TShape>& pShape);

	void					CalcHasAlpha();						//	loop through colours to check if we have any alpha colours in the verts
	void					PadColours();						//	ensure number of colours matches number of vertexes. Also adds missing 24/32 bit colours
	void					PadUVs();							//	ensure number of uvs matches number of vertexes
	void					AddColour(const TColour& Colour,u16 Count=1);	//	add colour to colour arrays
	void					RemoveColourAt(u16 VertexIndex);	//	remove a vertex colour 

	void					AddTriangles(const TArray<Triangle>& OtherPolygons,u32 OffsetVertexIndex);
	void					AddTristrips(const TArray<Tristrip>& OtherPolygons,u32 OffsetVertexIndex);
	void					AddTrifans(const TArray<Trifan>& OtherPolygons,u32 OffsetVertexIndex);
	void					AddLinestrips(const TArray<Linestrip>& OtherPolygons,u32 OffsetVertexIndex);
	void					AddLines(const TArray<Line>& OtherPolygons,u32 OffsetVertexIndex);

private:
	const TColour*			GetGenerationColour(const TColour* pColour);	//	returns a valid colour pointer if we expect one (mesh already has colours) - NULL's if we dont have colours - returns the original pointer if we can have colours
	const float2*			GetGenerationUV(const float2* pUV);	//	returns a valid colour pointer if we expect one (mesh already has colours) - NULL's if we dont have colours - returns the original pointer if we can have colours

protected:
	TArray<float3>			m_Vertexes;				//	vertexes of mesh
	TArray<TColour>			m_Colours;				//	vertex colours - float format (4*4)
	TArray<TColour24>		m_Colours24;			//	vertex colours - u8 format, no alpha (3*1)
	TArray<TColour32>		m_Colours32;			//	vertex colours - u8 format (4*1)
	TArray<float2>			m_UVs;					//	vertex texture mapping

	TArray<Triangle>		m_Triangles;			//	triangles in mesh
	TArray<Tristrip>		m_Tristrips;			//	tristrips in mesh
	TArray<Trifan>			m_Trifans;				//	trifans in mesh
	TArray<Linestrip>		m_Linestrips;			//	lineSTRIPS in mesh
	TArray<Line>			m_Lines;				//	lines in mesh - evaluates to one large list of verts
	
	TPtrKeyArray<TRef,TLMaths::TShape>	m_Datums;	//	datum shapes on mesh

	//	todo: replace with named Datums
	TLMaths::TBox			m_BoundsBox;			//	bounding box vertex extents
	TLMaths::TSphere		m_BoundsSphere;			//	bounding sphere
	TLMaths::TCapsule		m_BoundsCapsule;		//	bounding capsule 

	float					m_LineWidth;			//	width of lines, don't set dynamiccly just yet, only if you create the asset
	TFlags<TMeshFlags,u8>	m_Flags;				//	mesh flags
};




FORCEINLINE TPtr<TLMaths::TShape>& TLAsset::TMesh::AddDatum(TRefRef DatumRef,TPtr<TLMaths::TShape>& pShape) 
{
	TPtr<TLMaths::TShape>* ppShape = m_Datums.Add( DatumRef, pShape, TRUE );

	//	failed to add, return null Ptr
	if ( !ppShape )
		return TLPtr::GetNullPtr<TLMaths::TShape>();

	return *ppShape;
}


template<class SHAPETYPE>
SHAPETYPE* TLAsset::TMesh::GetDatum(TRefRef DatumRef)			
{	
	TPtr<TLMaths::TShape>& pShapePtr = GetDatum( DatumRef );
	if ( !pShapePtr )
		return NULL;

	//	cast down
	SHAPETYPE* pShape = pShapePtr.GetObject<SHAPETYPE>();	

	//	gr: unfortunetly as it's templated has to be in the header, so this check will only work if the calling file's project is debug
#ifdef _DEBUG
	if ( pShape->GetShapeType() != SHAPETYPE::GetShapeType_Static() )
	{
		TTempString Debug_String("Shape ");
		pShape->GetShapeType().GetString( Debug_String );
		Debug_String.Append(" tried to be cast to ");
		SHAPETYPE::GetShapeType_Static().GetString( Debug_String );
		TLDebug_Break( Debug_String );
		return NULL;
	}
#endif

	return pShape;
}
