/*------------------------------------------------------

	Texture-atlas asset 

-------------------------------------------------------*/
#pragma once

#include "TLAsset.h"
#include "TAsset.h"
#include <TootleCore/TKeyArray.h>
#include <TootleMaths/TBox.h>


namespace TLAsset
{
	class TAtlas;
	class TAtlasGlyph;
};




class TLAsset::TAtlasGlyph
{
public:
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

	FORCEINLINE void			SetTextureRef(TRefRef TextureRef)			{	m_TextureRef = TextureRef;	}
	FORCEINLINE TRefRef			GetTextureRef() const						{	return m_TextureRef;	}

protected:
	virtual SyncBool			ImportData(TBinaryTree& Data);	//	load asset data out binary data
	virtual SyncBool			ExportData(TBinaryTree& Data);	//	save asset data to binary data

protected:
	TKeyArray<u16,TAtlasGlyph>	m_Glyphs;			//	key array to glyph info
	TRef						m_TextureRef;		//	originally assigned texture - assume the texture map only works with the one texture
};

