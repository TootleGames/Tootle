/*------------------------------------------------------

	Texture-atlas asset 

-------------------------------------------------------*/
#pragma once

#include "TLAsset.h"
#include "TAsset.h"
#include <TootleCore/TKeyArray.h>
#include <TootleMaths/TBox.h>
#include <TootleCore/TStructArray.h>
#include <TootleCore/TTransform.h>


namespace TLAsset
{
	class TAtlas;
	
	//	todo: merge these
	class TAtlasGlyph;
	class TSpriteGlyph;	
	class TSpriteVertex;
};


class TLAsset::TSpriteVertex
{
private:
	static TLStruct::TDef			GetInitVertexDef();	//	generate vertex definition
public:
	static const TLStruct::TDef&	GetVertexDef()		{	static TLStruct::TDef g_Def = GetInitVertexDef();	return g_Def;	}
	
public:
	float3	m_Position;
	float2	m_TexCoord;
};


class TLAsset::TSpriteGlyph
{
public:
	FORCEINLINE u16			GetVertexCount() const		{	return 4;	}

	FORCEINLINE void		SetDefault();				//	setup default sprite, 0..1 in w/h/u/v
	FORCEINLINE void		Transform(const TLMaths::TTransform& Transform);
	
public:
	TSpriteVertex			m_Vertexes[4];
};

TLCore_DeclareIsDataType( TLAsset::TSpriteGlyph );


class TLAsset::TAtlasGlyph
{
public:
	TAtlasGlyph()	{}
	TAtlasGlyph(const TLMaths::TBox2D& UVBox,const Type2<u16>& TextureSize);

	Bool						ImportData(TBinaryTree& Data);	//	load glyph data
	void						ExportData(TBinaryTree& Data);	//	save glyph data

	FORCEINLINE float2			GetSize() const					{	return GetUV_BottomRight() - GetUV_TopLeft();	}
	FORCEINLINE const float2&	GetTopLeft() const				{	return GetUV_TopLeft();	}

	FORCEINLINE float2&			GetUV_TopLeft()					{	return m_UVs.x;	}
	FORCEINLINE float2&			GetUV_TopRight()				{	return m_UVs.y;	}
	FORCEINLINE float2&			GetUV_BottomRight()				{	return m_UVs.z;	}
	FORCEINLINE float2&			GetUV_BottomLeft()				{	return m_UVs.w;	}
	FORCEINLINE const float2&	GetUV_TopLeft() const			{	return m_UVs.x;	}
	FORCEINLINE const float2&	GetUV_TopRight() const			{	return m_UVs.y;	}
	FORCEINLINE const float2&	GetUV_BottomRight() const		{	return m_UVs.z;	}
	FORCEINLINE const float2&	GetUV_BottomLeft() const		{	return m_UVs.w;	}

public:
	Type4<float2>			m_UVs;			//	uv's in quad order - topleft, topright, bottomright, bottomleft
	TLMaths::TBox2D			m_GlyphBox;		//	size of the glyph for use as geometry
	
	//	gr: change this to an origin point (for lead-in) and some spacing w/h
	TLMaths::TBox2D			m_SpacingBox;	//	size of the glyph irrelavant of geometry - lead in/leadout/offset etc
	
	TSpriteGlyph			m_Sprite;		//	
};

TLCore_DeclareIsDataType( TLAsset::TAtlasGlyph );



class TLAsset::TAtlas : public TLAsset::TAsset
{
public:
	TAtlas(TRefRef AssetRef);

	static TRef					GetAssetType_Static()						{	return TRef_Static(A,t,l,a,s);	}

	FORCEINLINE const TAtlasGlyph*	GetGlyph(u16 Key) 						{	return m_Glyphs.Find(Key);	}
	FORCEINLINE const TAtlasGlyph*	GetGlyph(u16 Key) const					{	return m_Glyphs.Find(Key);	}
	FORCEINLINE TAtlasGlyph*	AddGlyph(u16 Key)							{	return m_Glyphs.AddNew( Key );	}
	FORCEINLINE TAtlasGlyph*	AddGlyph(u16 Key,const TAtlasGlyph& Glyph)	{	return m_Glyphs.Add( Key, Glyph );	}
	const TKeyArray<u16,TAtlasGlyph>&	GetGlyphs() const					{	return m_Glyphs;	}
	void						GetGlyphs(TArray<u16>& GlyphKeys) const;	//	get all the glyph keys into an array

	FORCEINLINE void			SetTextureRef(TRefRef TextureRef)			{	m_TextureRef = TextureRef;	}
	FORCEINLINE TRefRef			GetTextureRef() const						{	return m_TextureRef;	}

protected:
	virtual SyncBool			ImportData(TBinaryTree& Data);	//	load asset data out binary data
	virtual SyncBool			ExportData(TBinaryTree& Data);	//	save asset data to binary data

protected:
	TKeyArray<u16,TAtlasGlyph>	m_Glyphs;			//	key array to glyph info
	TRef						m_TextureRef;		//	originally assigned texture - assume the texture map only works with the one texture
};




//---------------------------------------------------
//	setup default sprite, 0..1 in w/h/u/v
//---------------------------------------------------
void TLAsset::TSpriteGlyph::SetDefault()
{
	//	todo: memcpy this from some default
	
	m_Vertexes[0].m_Position = float3( 0.f, 0.f, 0.f );
	m_Vertexes[0].m_TexCoord = float2( 0.f, 0.f );
	
	m_Vertexes[1].m_Position = float3( 1.f, 0.f, 0.f );
	m_Vertexes[1].m_TexCoord = float2( 1.f, 0.f );
	
	m_Vertexes[2].m_Position = float3( 1.f, 1.f, 0.f );
	m_Vertexes[2].m_TexCoord = float2( 1.f, 1.f );
	
	m_Vertexes[3].m_Position = float3( 0.f, 1.f, 0.f );
	m_Vertexes[3].m_TexCoord = float2( 0.f, 1.f );
}


void TLAsset::TSpriteGlyph::Transform(const TLMaths::TTransform& Transform)
{
	if ( Transform.HasAnyTransform() )
	{
		Transform.Transform( m_Vertexes[0].m_Position );
		Transform.Transform( m_Vertexes[1].m_Position );
		Transform.Transform( m_Vertexes[2].m_Position );
		Transform.Transform( m_Vertexes[3].m_Position );
	}
}

