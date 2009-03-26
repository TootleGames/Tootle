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

	FORCEINLINE float2&			GetUV_TopLeft()					{	return m_UVs.x;	}
	FORCEINLINE float2&			GetUV_TopRight()				{	return m_UVs.y;	}
	FORCEINLINE float2&			GetUV_BottomRight()				{	return m_UVs.z;	}
	FORCEINLINE float2&			GetUV_BottomLeft()				{	return m_UVs.w;	}
	FORCEINLINE const float2&	GetUV_TopLeft() const			{	return m_UVs.x;	}
	FORCEINLINE const float2&	GetUV_TopRight() const			{	return m_UVs.y;	}
	FORCEINLINE const float2&	GetUV_BottomRight() const		{	return m_UVs.z;	}
	FORCEINLINE const float2&	GetUV_BottomLeft() const		{	return m_UVs.w;	}

public:
//	TFixedArray<float2,4>	m_UVs;			//	uv's in quad order - topleft, topright, bottomright, bottomleft
	Type4<float2>			m_UVs;			//	uv's in quad order - topleft, topright, bottomright, bottomleft
	TLMaths::TBox2D			m_GlyphBox;		//	normalised (to the overall font's size) size of the glyph for use as geometry
	TLMaths::TBox2D			m_SpacingBox;	//	size of the glyph irrelavant of geometry - lead in/leadout/offset etc
};

TLCore_DeclareIsDataType( TLAsset::TAtlasGlyph );



class TLAsset::TAtlas : public TLAsset::TAsset
{
public:
	TAtlas(TRefRef AssetRef);

	FORCEINLINE const TAtlasGlyph*	GetGlyph(u16 Key) 						{	return m_Glyphs.Find(Key);	}
	FORCEINLINE const TAtlasGlyph*	GetGlyph(char KeyChar) 					{	u16 Key16 = ((u16)KeyChar) & 0x00ff;	return GetGlyph( Key16);	}
	FORCEINLINE const TAtlasGlyph*	GetGlyph(u16 Key) const					{	return m_Glyphs.Find(Key);	}
	FORCEINLINE const TAtlasGlyph*	GetGlyph(char KeyChar) const			{	u16 Key16 = ((u16)KeyChar) & 0x00ff;	return GetGlyph( Key16);	}
	FORCEINLINE TAtlasGlyph*	AddGlyph(u16 Key)							{	return m_Glyphs.AddNew( Key );	}
	FORCEINLINE TAtlasGlyph*	AddGlyph(u16 Key,const TAtlasGlyph& Glyph)	{	return m_Glyphs.Add( Key, Glyph );	}

	FORCEINLINE void			SetTextureRef(TRefRef TextureRef)			{	m_TextureRef = TextureRef;	}
	FORCEINLINE TRefRef			GetTextureRef() const						{	return m_TextureRef;	}

protected:
	virtual SyncBool			ImportData(TBinaryTree& Data);	//	load asset data out binary data
	virtual SyncBool			ExportData(TBinaryTree& Data);	//	save asset data to binary data

protected:
	TKeyArray<u16,TAtlasGlyph>	m_Glyphs;			//	key array to glyph info
	TRef						m_TextureRef;		//	originally assigned texture - assume the texture map only works with the one texture
};

