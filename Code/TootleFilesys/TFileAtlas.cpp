#include "TFileAtlas.h"
#include <TootleAsset/TAtlas.h>
#include <TootleFileSys/TLFileSys.h>



//---------------------------------------------------------
//	
//---------------------------------------------------------
TLFileSys::TFileAtlas::TFileAtlas(TRefRef FileRef,TRefRef FileTypeRef) :
	TFileXml	( FileRef, FileTypeRef )
{
}


//--------------------------------------------------------
//	import the XML
//--------------------------------------------------------
SyncBool TLFileSys::TFileAtlas::ExportAsset(TPtr<TLAsset::TAsset>& pAsset,Bool& Supported)
{
	if ( pAsset )
	{
		TLDebug_Break("Async export not supported yet. asset should be NULL");
		return SyncFalse;
	}

	Supported = TRUE;

	//	import xml
	SyncBool ImportResult = TFileXml::Import();
	if ( ImportResult != SyncTrue )
		return ImportResult;

	//	get the root tag
	TPtr<TXmlTag> pRootTag = m_XmlData.GetChild("Atlas");

	//	malformed XML
	if ( !pRootTag )
	{
		TLDebug_Print("Scheme file missing root <Atlas> tag");
		return SyncFalse;
	}

	//	make up new storage asset type
	TPtr<TLAsset::TAtlas> pNewAsset = new TLAsset::TAtlas( GetFileRef() );
	ImportResult = ImportAtlas( *pRootTag, *pNewAsset );

	//	failed to import
	if ( ImportResult != SyncTrue )
	{
		return SyncFalse;
	}

	//	assign resulting asset
	pAsset = pNewAsset;

	return SyncTrue;
}

//--------------------------------------------------------
//	
//--------------------------------------------------------
SyncBool TLFileSys::TFileAtlas::ImportAtlas(TXmlTag& Tag,TLAsset::TAtlas& Atlas)
{
	/*
<?xml version="1.0" ?>
<Atlas>
	<Glyph	x=0	y=0	w=0.5	h=0.5 />
	<Glyph	x=0.5	y=0	w=0.5	h=0.5 />
	<Glyph	x=0	y=0.5	w=0.5	h=0.5 />
	<Glyph	x=0.5	y=0.5	w=0.5	h=0.5 />
</Atlas>
	*/
	
  
	//	get texture ref if provided
	const TString* pTextureRefString = Tag.GetProperty("TextureRef");
	if ( pTextureRefString )
	{
		TRef TextureRef( *pTextureRefString );
		Atlas.SetTextureRef( TextureRef );
	}

	//	now extract all the glyphs's
	for ( u32 c=0;	c<Tag.GetChildren().GetSize();	c++ )
	{
		TPtr<TXmlTag>& pChildTag = Tag.GetChildren().ElementAt(c);
		if ( pChildTag->GetTagName() != "glyph" )
			continue;

		SyncBool TagImportResult = ImportGlyph( *pChildTag, Atlas );

		//	failed
		if ( TagImportResult == SyncFalse )
			return SyncFalse;

		//	async
		if ( TagImportResult == SyncWait )
		{
			TLDebug_Break("todo: async Scheme import");
			return SyncFalse;
		}
	}
	
	return SyncTrue;
}

//--------------------------------------------------------
//	
//--------------------------------------------------------
SyncBool TLFileSys::TFileAtlas::ImportGlyph(TXmlTag& Tag,TLAsset::TAtlas& Atlas)
{
	/*
	<Glyph	x=0	y=0	w=0.5	h=0.5 />
	.....
	*/

	//	get properties we're going to use
	const TString* pXString = Tag.GetProperty("x");
	const TString* pYString = Tag.GetProperty("y");
	const TString* pWidthString = Tag.GetProperty("w");
	const TString* pHeightString = Tag.GetProperty("h");
	const TString* pRefString = Tag.GetProperty("ref");

	//	get char's char/key (these are all integers, but to make code easier to read we read them out as floats)
	s32 Ref;		pRefString ? pRefString->GetInteger( Ref ) : 0;
	float ImageX;	pXString ? pXString->GetFloat( ImageX ) : 0.f;
	float ImageY;	pYString ? pYString->GetFloat( ImageY ) : 0.f;
	float Width;	pWidthString ? pWidthString->GetFloat( Width ) : 1.f;
	float Height;	pHeightString ? pHeightString->GetFloat( Height ) : 1.f;
	
	//	check ref isn't already in use
	if ( Atlas.GetGlyph( (u16)Ref ) )
	{
		if ( !TLDebug_Break( TString("Glyph with Ref %d already exists on atlas", Ref ) ) )
			return SyncFalse;
	}

	//	make up glyph
	TLAsset::TAtlasGlyph* pGlyph = Atlas.AddGlyph( (u16)Ref );
	if ( !pGlyph )
		return SyncFalse;
	TLAsset::TAtlasGlyph& Glyph = *pGlyph;
	
	//	image box inside texture
	TLMaths::TBox2D ImageBox( float2( ImageX, ImageY ), float2( ImageX + Width, ImageY + Height ) );

	//	work out uv's (normalised position inside texture)
	Glyph.GetUV_TopLeft() =		float2( ImageBox.GetLeft(),		ImageBox.GetTop() )		/ 1.f;
	Glyph.GetUV_TopRight() =	float2( ImageBox.GetRight(),	ImageBox.GetTop() )		/ 1.f;
	Glyph.GetUV_BottomRight() =	float2( ImageBox.GetRight(),	ImageBox.GetBottom() )	/ 1.f;
	Glyph.GetUV_BottomLeft() =	float2( ImageBox.GetLeft(),		ImageBox.GetBottom() )	/ 1.f;

	//	work out the glyph's box (normalised to character sizes)
	//	gr: for general atlases this is just the size of 1
	Glyph.m_GlyphBox = TLMaths::TBox2D( float2(0.f,0.f), float2(1.f,1.f) );
	Glyph.m_SpacingBox = Glyph.m_GlyphBox;

	return SyncTrue;
}

