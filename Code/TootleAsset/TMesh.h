/*------------------------------------------------------

	Mesh/model asset

-------------------------------------------------------*/
#pragma once

#include "TLAsset.h"
#include "TAsset.h"
#include <TootleCore/TArray.h>
#include <TootleCore/TColour.h>
#include <TootleMaths/TBox.h>
#include <TootleMaths/TSphere.h>
#include <TootleMaths/TCapsule.h>

namespace TLAsset
{
	class TMesh;
	class TFont;

	const u32	g_MaxLineStripSize = 10;

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
	
	
};



class TLAsset::TMesh : public TLAsset::TAsset
{
public:
	friend class TLAsset::TFont;
	typedef Type3<u16>	Triangle;
	typedef TArray<u16>	Tristrip;
	typedef TArray<u16>	Trifan;
	typedef TArray<u16>	Line;

public:
	TMesh(const TRef& AssetRef);

	//	manipulation
	void					GenerateShape(const TLMaths::TBox& Box)				{	GenerateCube( Box );	}
	void					GenerateShape(const TLMaths::TSphere& Sphere)		{	GenerateSphere( Sphere );	}
	void					GenerateShape(const TLMaths::TCapsule& Capsule)		{	GenerateCapsule( Capsule );	}
	void					GenerateCube(float Scale);							//	generate a cube mesh
	void					GenerateCube(const TLMaths::TBox& Box);				//	generate a cube mesh from a math box
	void					GenerateSphere(float Radius,const float3& Center=float3(0,0,0));		//	generate a sphere
	void					GenerateCapsule(float Radius,const float3& Start,const float3& End);	//	generate a capsule
	void					GenerateSphere(const TLMaths::TSphere& Sphere);		//	generate a sphere
	void					GenerateCapsule(const TLMaths::TCapsule& Capsule);	//	generate a capsule
	void					GenerateRainbowColours();							//	create colours for each vertex

	s32						AddVertex(const float3& VertexPos);							//	add vertex to the list, makes up normals and colours if required
	s32						AddVertex(const float3& VertexPos,const TColour& Colour);	//	add vertex with colour to the list, pad normals and colours if previously didnt exist
	Bool					RemoveVertex(u32 VertexIndex);								//	remove a vertex, remove it's colour, remove any polygons that use this vertex, and correct the vertex indexes in polygons (anything > VI needs reducing). returns if any changes to polygons made
	Bool					ReplaceVertex(u32 OldVertexIndex,u32 NewVertexIndex);		//	find all uses of OldVertexIndex in polygons and swap them for NewVertexIndex 
	inline void				ScaleVerts(float Scale,u32 FirstVert=0,s32 LastVert=-1)				{	ScaleVerts( float3( Scale, Scale, Scale ), FirstVert, LastVert );	}
	void					ScaleVerts(const float3& Scale,u32 FirstVert=0,s32 LastVert=-1);	//	scale all vertexes
	void					MoveVerts(const float3& Movement,u32 FirstVert=0,s32 LastVert=-1);	//	move all verts

	//	data accessors
	const TArray<float3>&	GetVertexes() const					{	return m_Vertexes;	}
	float3&					GetVertex(u32 VertIndex)			{	return m_Vertexes[VertIndex];	}
	inline u32				GetVertexCount()			const	{	return m_Vertexes.GetSize(); }
	TArray<TColour>&		GetColours()						{	return m_Colours;	}
	const TArray<TColour>&	GetColours() const					{	return m_Colours;	}

	TArray<Triangle>&		GetTriangles()						{	return m_Triangles;	}
	TArray<Tristrip>&		GetTristrips()						{	return m_Tristrips;	}
	TArray<Trifan>&			GetTrifans()						{	return m_Trifans;	}
	TArray<Line>&			GetLines()							{	return m_Lines;	}
	const TArray<Triangle>&	GetTriangles() const				{	return m_Triangles;	}
	const TArray<Tristrip>&	GetTristrips() const				{	return m_Tristrips;	}
	const TArray<Trifan>&	GetTrifans() const					{	return m_Trifans;	}
	const TArray<Line>&		GetLines() const					{	return m_Lines;	}
	
	void					SetLineWidth(float LineWidth)		{	m_LineWidth = LineWidth;	TLMaths::Limit( m_LineWidth, 0.f, 100.f );	}
	float					GetLineWidth() const				{	return m_LineWidth;	}

	void					SetBoundsInvalid()					{	SetBoundsBoxInvalid();		SetBoundsSphereInvalid();		SetBoundsCapsuleInvalid();	}
	void					CalcBounds(Bool ForceCalc=FALSE)	{	CalcBoundsBox(ForceCalc);	CalcBoundsSphere(ForceCalc);	CalcBoundsCapsule(ForceCalc);	}

	void					SetBoundsBoxInvalid()			{	m_BoundsBox.SetInvalid();	}
	TLMaths::TBox&			CalcBoundsBox(Bool ForceCalc=FALSE);	//	calculate bounds box if invalid
	TLMaths::TBox&			GetBoundsBox()					{	return m_BoundsBox;	}
	
	void					SetBoundsSphereInvalid()			{	m_BoundsSphere.SetInvalid();	}
	TLMaths::TSphere&		CalcBoundsSphere(Bool ForceCalc=FALSE);	//	calculate bounds Sphere if invalid
	TLMaths::TSphere&		GetBoundsSphere()					{	return m_BoundsSphere;	}
	
	void					SetBoundsCapsuleInvalid()			{	m_BoundsCapsule.SetInvalid();	}
	TLMaths::TCapsule&		CalcBoundsCapsule(Bool ForceCalc=FALSE);	//	calculate bounds box if invalid
	TLMaths::TCapsule&		GetBoundsCapsule()					{	return m_BoundsCapsule;	}

	const TArray<TFixedVertex>&	GetFixedVertexes();				//	return fixed vertex array for verts. calc if required
	void					SetFixedVertexesInvalid()			{	m_FixedVertexes.Empty();	}
	
protected:
	virtual SyncBool		ImportData(TBinaryTree& Data);	//	load asset data out binary data
	virtual SyncBool		ExportData(TBinaryTree& Data);	//	save asset data to binary data

	void					Empty();

protected:
	TArray<float3>			m_Vertexes;				//	vertexes of mesh
	TArray<TColour>			m_Colours;				//	vertex colours
	TArray<TFixedVertex>	m_FixedVertexes;		//	

	TArray<Triangle>		m_Triangles;			//	triangles in mesh
	TArray<Tristrip>		m_Tristrips;			//	tristrips in mesh
	TArray<Trifan>			m_Trifans;				//	trifans in mesh
	TArray<Line>			m_Lines;				//	lineSTRIPS in mesh
	
	TLMaths::TBox			m_BoundsBox;			//	bounding box vertex extents
	TLMaths::TSphere		m_BoundsSphere;			//	bounding sphere
	TLMaths::TCapsule		m_BoundsCapsule;		//	bounding capsule 

	float					m_LineWidth;			//	width of lines, don't set dynamiccly just yet, only if you create the asset
};


namespace TLCore
{
TLCore_DeclareIsDataType( TLAsset::TFixedVertex );
}
