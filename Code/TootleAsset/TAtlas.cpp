#include "TAtlas.h"
#include <TootleCore/TBinaryTree.h>



//-------------------------------------------------------
//	setup a glyph from a UV box
//-------------------------------------------------------
TLAsset::TAtlasGlyph::TAtlasGlyph(const TLMaths::TBox2D& UVBox,const Type2<u16>& TextureSize)
{
	m_UVs[0] = UVBox.GetTopLeft();	
	m_UVs[1] = UVBox.GetTopRight();	
	m_UVs[2] = UVBox.GetBottomRight();	
	m_UVs[3] = UVBox.GetBottomLeft();	

	//	scale up the glyphbox
	m_GlyphBox = UVBox;
	m_GlyphBox *= float2( (float)TextureSize.x, (float)TextureSize.y );

	//	spacing box is just a copy of the glyph box
	m_SpacingBox = m_GlyphBox;
}


//-------------------------------------------------------
//	load glyph data
//-------------------------------------------------------
Bool TLAsset::TAtlasGlyph::ImportData(TBinaryTree& Data)
{
	if ( !Data.Read( m_UVs ) )			return FALSE;
	if ( !Data.Read( m_GlyphBox ) )		return FALSE;
	if ( !Data.Read( m_SpacingBox ) )	return FALSE;

	return TRUE;
}


//-------------------------------------------------------
//	save glyph data
//-------------------------------------------------------
void TLAsset::TAtlasGlyph::ExportData(TBinaryTree& Data)
{
	Data.Write( m_UVs );
	Data.Write( m_GlyphBox );
	Data.Write( m_SpacingBox );
}



TLAsset::TAtlas::TAtlas(TRefRef AssetRef) :
	TAsset				( GetAssetType_Static(), AssetRef )
{
}




//-------------------------------------------------------
//	load asset data out binary data
//-------------------------------------------------------
SyncBool TLAsset::TAtlas::ImportData(TBinaryTree& Data)		
{
	if ( !Data.ImportData("Texture", m_TextureRef ) )
		return SyncFalse;

	//	find children
	TPtrArray<TBinaryTree> GlyphDataArray;
	if ( Data.GetChildren("Glyph", GlyphDataArray) > 0 )
	{
		//	process next children
		for ( u32 i=0;	i<GlyphDataArray.GetSize();	i++ )
		{
			TPtr<TBinaryTree>& pChildData = GlyphDataArray[i];
			if ( !pChildData )
			{
				TLDebug_Break("Child data expected");
				continue;
			}

			//	reset data
			pChildData->ResetReadPos();

			//	get key
			u16 GlyphKey;
			if ( !pChildData->Read( GlyphKey ) )
				return SyncFalse;

			//	create glyph
			TAtlasGlyph* pGlyph = AddGlyph( GlyphKey );

			//	import data
			if ( !pGlyph->ImportData( *pChildData ) )
				return SyncFalse;
		}
	}

	//	store unread data
	ImportUnknownData( Data );

	return SyncTrue;
}


//-------------------------------------------------------
//	save asset data to binary data
//-------------------------------------------------------
SyncBool TLAsset::TAtlas::ExportData(TBinaryTree& Data)				
{	
	Data.ExportData("Texture", m_TextureRef );

	//	export each glyph
	for ( u32 i=0;	i<m_Glyphs.GetSize();	i++ )
	{
		TAtlasGlyph& Glyph = m_Glyphs.ElementAt(i);
		u16& Key = m_Glyphs.GetKeyAt(i);

		//	add to exporting data
		TPtr<TBinaryTree>& pChildData = Data.AddChild("Glyph");
		if ( !pChildData )
		{
			TLDebug_Break("Failed to create child");
			continue;
		}

		//	write the ref of the child to the child's data
		pChildData->Write( Key );

		//	now export the data for that child
		Glyph.ExportData( *pChildData );
	}

	//	export other data
	ExportUnknownData( Data );

	return SyncTrue;
}	
