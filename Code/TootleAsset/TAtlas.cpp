#include "TAtlas.h"
#include <TootleCore/TBinaryTree.h>
#include <TootleRender/TRasteriser.h>		//	required for vertex element type names


//-------------------------------------------------------
//	generate vertex definition
//-------------------------------------------------------
TLStruct::TDef TLAsset::TSpriteVertex::GetInitVertexDef()
{
	//	make up a struct array (that's the interface to make up this stuff) then copy the definition
	TStructArray Array;
	Array.AddMember( TVertexElementType_Position, &TLAsset::TSpriteVertex::m_Position ); 
	Array.AddMember( TVertexElementType_TexCoord, &TLAsset::TSpriteVertex::m_TexCoord ); 
	return Array.GetDefinition();
}

	
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
	//	old format
	if ( Data.GetSizeUnread() > 0 )
	{
		if ( !Data.Read( m_UVs ) )			return FALSE;
		if ( !Data.Read( m_GlyphBox ) )		return FALSE;
		if ( !Data.Read( m_SpacingBox ) )	return FALSE;
	}
	
	//	new format!
	Data.ImportData("Uvs", m_UVs );
	Data.ImportData("GlyBox", m_GlyphBox );
	Data.ImportData("SpcBox", m_SpacingBox );

	//if ( !Data.ImportData("Sprite",m_Sprite ) )
	{
		//	generate sprite data
		m_Sprite.m_Vertexes[0].m_TexCoord = m_UVs[0];
		m_Sprite.m_Vertexes[1].m_TexCoord = m_UVs[1];
		m_Sprite.m_Vertexes[2].m_TexCoord = m_UVs[2];
		m_Sprite.m_Vertexes[3].m_TexCoord = m_UVs[3];
		m_Sprite.m_Vertexes[0].m_Position = m_GlyphBox.GetTopLeft();
		m_Sprite.m_Vertexes[1].m_Position = m_GlyphBox.GetTopRight();
		m_Sprite.m_Vertexes[2].m_Position = m_GlyphBox.GetBottomRight();
		m_Sprite.m_Vertexes[3].m_Position = m_GlyphBox.GetBottomLeft();
	}
	
	return TRUE;
}


//-------------------------------------------------------
//	save glyph data
//-------------------------------------------------------
void TLAsset::TAtlasGlyph::ExportData(TBinaryTree& Data)
{
	Data.ExportData("Uvs", m_UVs );
	Data.ExportData("GlyBox", m_GlyphBox );
	Data.ExportData("SpcBox", m_SpacingBox );
	Data.ExportData("Sprite", m_Sprite );
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
			if ( pGlyph )
			{
				//	import data
				if ( !pGlyph->ImportData( *pChildData ) )
					return SyncFalse;
			}
			else
			{
				TDebugString Debug_String;
				Debug_String << "Warning: failed to add glyph " << GlyphKey << " to atlas " << this->GetAssetAndTypeRef();
				TLDebug_Print( Debug_String );
			}
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

//-------------------------------------------------------
//	get all the glyph keys into an array
//-------------------------------------------------------
void TLAsset::TAtlas::GetGlyphs(TArray<u16>& GlyphKeys) const
{
	for ( u32 g=0;	g<m_Glyphs.GetSize();	g++ )
	{
		GlyphKeys.Add( m_Glyphs.GetKeyAt( g ) );
	}
}


