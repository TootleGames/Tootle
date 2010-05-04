#include "TFileAtlas.h"
#include <TootleAsset/TAtlas.h>
#include <TootleAsset/TTexture.h>
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
SyncBool TLFileSys::TFileAtlas::ExportAsset(TPtr<TLAsset::TAsset>& pAsset,TRefRef ExportAssetType)
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
	const TString* pRefString = Tag.GetProperty("ref");
	const TString* pXY = Tag.GetProperty("xy");
	const TString* pWH = Tag.GetProperty("wh");
	const TString* pUV = Tag.GetProperty("uv");
	const TString* pST = Tag.GetProperty("st");

	//	get glyph's char/key (these are all integers, but to make code easier to read we read them out as floats)
	s32 Ref;	
	pRefString ? pRefString->GetInteger( Ref ) : 0;

	//	need at least 1 start and end coordinate
	if ( !pXY && !pUV )
	{
		TDebugString Debug_String;
		Debug_String << "Glyph " << Ref << " (in " << this->GetFilename() << ") missing xy or uv property. Cannot calculate glyph";
		TLDebug_Break( Debug_String );
		return SyncFalse;
	}

	if ( !pWH && !pST )
	{
		TLDebug_Break("Glyph missing wh or st property. Cannot calculate glyph");
		return SyncFalse;
	}

	//	convert params from strings
	float2 uv,st;
	int2 xy,wh;
	bool Success = true;
	if ( pXY )	Success &= pXY->GetInteger( xy );
	if ( pWH )	Success &= pWH->GetInteger( wh );
	if ( pUV )	Success &= pUV->GetFloat( uv );
	if ( pST )	Success &= pST->GetFloat( st );
	
	//	error reading params
	if ( !Success )
	{
		TLDebug_Break("Error parsing glyph properties");
		return SyncFalse;
	}

	//	convert texture pixel coords to uv's
	if ( pXY || pWH )
	{
		//	get texture
		TLAsset::TTexture* pTexture = TLAsset::GetAsset<TLAsset::TTexture>( Atlas.GetTextureRef() );
		if ( !pTexture )
		{
			TDebugString Debug_String;
			Debug_String << "Texture " << Atlas.GetTextureRef() << " not found for atlas xy/wh conversion to uv/st";
			TLDebug_Break( Debug_String );
			return SyncFalse;
		}

		//	convert coords to uv/st
		if ( pXY )
			uv = float2(xy) / float2(pTexture->GetSize());

		if ( pWH )
			st = uv + ( float2(wh) / float2(pTexture->GetSize()) );
	}

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
	
	//	work out the real glyph size if a texture has been specified
	float2 ImageScale( 1.f, 1.f );

	if ( Atlas.GetTextureRef().IsValid() )
	{
		//	get texture
		TLAsset::TTexture* pTexture = TLAsset::GetAsset<TLAsset::TTexture>( Atlas.GetTextureRef() );
		if ( !pTexture )
		{
			TDebugString Debug_String;
			Debug_String << "Texture " << Atlas.GetTextureRef() << " not found for glyph image size calculation";
			TLDebug_Break( Debug_String );
			return SyncFalse;
		}
		ImageScale = pTexture->GetSize();
	}

	//	image box inside texture
	TLMaths::TBox2D ImageBox( uv, st );

	//	work out uv's (normalised position inside texture)
	Glyph.GetUV_TopLeft() =		uv;
	Glyph.GetUV_TopRight() =	float2( st.x, uv.y );
	Glyph.GetUV_BottomRight() =	st;
	Glyph.GetUV_BottomLeft() =	float2( uv.x, st.y );

	//	work out the glyph's box (normalised to character sizes)
	//	gr: for general atlases this is just the size of 1
	Glyph.m_GlyphBox = TLMaths::TBox2D( float2(0.f,0.f), float2(1.f,1.f) );
	Glyph.m_SpacingBox = Glyph.m_GlyphBox;

	return SyncTrue;
}

