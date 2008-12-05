/*------------------------------------------------------

	Font asset
	contains a Mesh asset for each glyph (character/symbol)

-------------------------------------------------------*/
#pragma once

#include "TLAsset.h"
#include "TAsset.h"
#include "TMesh.h"
#include <TootleCore/TKeyArray.h>


namespace TLAsset
{
	class TFont;
};



class TLAsset::TFont : public TLAsset::TAsset
{
public:
	TFont(const TRef& AssetRef);

	TPtr<TLAsset::TMesh>	GetGlyph(u16 Character)		{	return m_Glyphs.FindPtr(Character);	}
	TPtr<TLAsset::TMesh>	GetGlyph(char Character)	{	u16 Char16 = ((u16)Character) & 0x00ff;	return GetGlyph( Char16);	}
	u32						GetGlyphCount() const		{	return m_Glyphs.GetSize();	}
	TPtr<TLAsset::TMesh>	AddGlyph(u16 Character);	//	add a new glyph to the font
	void					AddGlyph(u16 Character,TPtr<TLAsset::TMesh>& pMesh);	//	add a new glyph to the font

protected:
	virtual SyncBool		ImportData(TBinaryTree& Data);	//	load asset data out binary data
	virtual SyncBool		ExportData(TBinaryTree& Data);	//	save asset data to binary data

protected:
	TPtrKeyArray<u16,TLAsset::TMesh>	m_Glyphs;			//	key array to mesh assets. each glyph is a mesh and we find them by their character.
	u32									m_ImportChildIndex;	//	current child we're importing
};

