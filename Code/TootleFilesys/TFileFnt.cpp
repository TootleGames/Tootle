#include "TFileFnt.h"
#include <TootleAsset/TAtlas.h>
#include <TootleFileSys/TLFileSys.h>




//---------------------------------------------------------
//	
//---------------------------------------------------------
TLFileSys::TFileFnt::TFileFnt(TRefRef FileRef,TRefRef FileTypeRef) :
	TFileXml	( FileRef, FileTypeRef )
{
}


//--------------------------------------------------------
//	import the XML
//--------------------------------------------------------
SyncBool TLFileSys::TFileFnt::ExportAsset(TPtr<TLAsset::TAsset>& pAsset,TRefRef ExportAssetType)
{
	if ( pAsset )
	{
		TLDebug_Break("Async export not supported yet. asset should be NULL");
		return SyncFalse;
	}

	//	import xml
	SyncBool ImportResult = TFileXml::Import();
	if ( ImportResult != SyncTrue )
		return ImportResult;

	//	get the root tag
	TPtr<TXmlTag> pRootTag = m_XmlData.GetChild("font");

	//	malformed XML
	if ( !pRootTag )
	{
		TLDebug_Print("Scheme file missing root <font> tag");
		return SyncFalse;
	}

	//	make up new storage asset type
	TPtr<TLAsset::TAtlas> pNewAsset = new TLAsset::TAtlas( GetFileRef() );
	ImportResult = ImportFont( *pRootTag, *pNewAsset );

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
SyncBool TLFileSys::TFileFnt::ImportFont(TXmlTag& Tag,TLAsset::TAtlas& Atlas)
{
	/*
<font>
  <info face="CreativeBlock BB" size="32" bold="1" italic="0" charset="" unicode="1" stretchH="100" smooth="1" aa="1" padding="0,0,0,0" spacing="1,1" outline="0"/>
  <common lineHeight="32" base="28" scaleW="256" scaleH="256" pages="1" packed="0" alphaChnl="0" redChnl="0" greenChnl="0" blueChnl="0"/>
  <pages>
    <page id="0" file="tcomic_00.png" />
  </pages>
  <chars count="91">
    <char id="32" x="253" y="0" width="1" height="1" xoffset="0" yoffset="28" xadvance="14" page="0" chnl="15" />
    <char id="33" x="240" y="120" width="10" height="27" xoffset="2" yoffset="2" xadvance="13" page="0" chnl="15" />
	.....
	*/
	
	//	get info tags 
	TXmlTag* pInfoTag = Tag.GetChild("info");
	TXmlTag* pCommonTag = Tag.GetChild("common");
	TXmlTag* pPagesTag = Tag.GetChild("pages");
	TXmlTag* pCharsTag = Tag.GetChild("chars");

	//	missing required tags
	if ( !pInfoTag || !pCommonTag || !pPagesTag || !pCharsTag )
	{
		TLDebug_Assert( pInfoTag, "expected <info> tag in .fnt");
		TLDebug_Assert( pCommonTag, "expected <common> tag in .fnt");
		TLDebug_Assert( pPagesTag, "expected <pages> tag in .fnt");
		TLDebug_Assert( pCharsTag, "expected <chars> tag in .fnt");
		return SyncFalse;
	}

	//	get base char dimensions
	Type2<s32> ImageSize;	//	total image size - divide all dimenions by this to get normalised scale
	s32 LineHeight;			//	height of line - ie. general scale of a char
	s32 LineBaseLine;		//	y of base line of a char
	pCommonTag->GetProperty("scaleW")->GetInteger( ImageSize.x );
	pCommonTag->GetProperty("scaleH")->GetInteger( ImageSize.y );
	pCommonTag->GetProperty("lineHeight")->GetInteger( LineHeight );
	pCommonTag->GetProperty("base")->GetInteger( LineBaseLine );

	//	get page info tags
	TPtrArray<TXmlTag> PageTags;
	pPagesTag->GetChildren("page", PageTags );
	if ( PageTags.GetSize() > 1 )
	{
		TLDebug_Break("Only 1 page of fonts are supported in texture atlas's - try using less chars in your font, shrinking the chars, or making the texture bigger");
		return SyncFalse;
	}

	//	extract the texture ref from the page tag
	TXmlTag* pPageTag = PageTags[0];
	if ( !pPageTag )
	{
		TLDebug_Break("Missing atlas page tag");
		return SyncFalse;
	}
  
	const TString* pPageTextureFileString = pPageTag->GetProperty("file");
	if ( !pPageTextureFileString )
	{
		TLDebug_Break(".fnt <page> filename expected");
		return SyncFalse;
	}

	//	extract a ref from the filename - note; use the file sys filename splitting so that "x.fnt" doesnt turn into a ref of "xfnt"
	TTypedRef FileRef = TLFileSys::GetFileAndTypeRef( *pPageTextureFileString );
	Atlas.SetTextureRef( FileRef.GetRef() );

	//	now extract all the char's
	for ( u32 c=0;	c<pCharsTag->GetChildren().GetSize();	c++ )
	{
		TPtr<TXmlTag>& pChildTag = pCharsTag->GetChildren().ElementAt(c);
		if ( pChildTag->GetTagName() != "char" )
			continue;

		SyncBool TagImportResult = ImportChar( *pChildTag, Atlas, float2( (float)ImageSize.x, (float)ImageSize.y ), (float)LineHeight, LineBaseLine );

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
SyncBool TLFileSys::TFileFnt::ImportChar(TXmlTag& Tag,TLAsset::TAtlas& Atlas,const float2& ImageScale,float GlyphScale,u32 LineBaseLine)
{
	/*
    <char id="33" x="240" y="120" width="10" height="27" xoffset="2" yoffset="2" xadvance="13" page="0" chnl="15" />
	.....
	*/

	//	get properties we're going to use
	const TString* pIDString = Tag.GetProperty("id");
	const TString* pXString = Tag.GetProperty("x");
	const TString* pYString = Tag.GetProperty("y");
	const TString* pWidthString = Tag.GetProperty("width");
	const TString* pHeightString = Tag.GetProperty("height");
	const TString* pXOffsetString = Tag.GetProperty("xoffset");
	const TString* pYOffsetString = Tag.GetProperty("yoffset");
	const TString* pXKerningString = Tag.GetProperty("xadvance");

	//	get char's char/key (these are all integers, but to make code easier to read we read them out as floats)
	s32 Char;		pIDString->GetInteger( Char );
	float ImageX;	pXString->GetFloat( ImageX );
	float ImageY;	pYString->GetFloat( ImageY );
	float Width;	pWidthString->GetFloat( Width );
	float Height;	pHeightString->GetFloat( Height );
	float XOffset;	pXOffsetString->GetFloat( XOffset );
	float YOffset;	pYOffsetString->GetFloat( YOffset );
	float XKern;	pXKerningString->GetFloat( XKern );

	//	make up glyph
	TLAsset::TAtlasGlyph* pGlyph = Atlas.AddGlyph( (u16)Char );
	if ( !pGlyph )
		return SyncFalse;
	TLAsset::TAtlasGlyph& Glyph = *pGlyph;
	
	//	image box inside texture
	TLMaths::TBox2D ImageBox( float2( ImageX, ImageY ), float2( ImageX + Width, ImageY + Height ) );

	//	work out uv's (normalised position inside texture)
	Glyph.GetUV_TopLeft() =		float2( ImageBox.GetLeft(),		ImageBox.GetTop() )		/ ImageScale;
	Glyph.GetUV_TopRight() =	float2( ImageBox.GetRight(),	ImageBox.GetTop() )		/ ImageScale;
	Glyph.GetUV_BottomRight() =	float2( ImageBox.GetRight(),	ImageBox.GetBottom() )	/ ImageScale;
	Glyph.GetUV_BottomLeft() =	float2( ImageBox.GetLeft(),		ImageBox.GetBottom() )	/ ImageScale;

	//	work out the glyph's box (normalised to character sizes)
	float2 GlyphMin = float2( XOffset, YOffset ) / GlyphScale;
	float2 GlyphSize = float2( Width, Height ) / GlyphScale;
	Glyph.m_GlyphBox.Set( GlyphMin, GlyphMin + GlyphSize );

	//	work out the spacing... this is the glyphbox + kerning
	//	gr: kerning includes width
	float2 SpacingMin( 0, 0 );
	float2 SpacingSize = float2( XKern, Height ) / GlyphScale;
	Glyph.m_SpacingBox.Set( SpacingMin, SpacingMin + SpacingSize );

	return SyncTrue;
}

