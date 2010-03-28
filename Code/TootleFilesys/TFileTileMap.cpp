#include "TFileTileMap.h"
#include <TootleFileSys/TLFileSys.h>
#include <TootleAsset/TTileMap.h>
#include <TootleFileSys/TLFile.h>



//---------------------------------------------------------
//	
//---------------------------------------------------------
TLFileSys::TFileTileMap::TFileTileMap(TRefRef FileRef,TRefRef FileTypeRef) :
	TFileXml	( FileRef, FileTypeRef )
{
}


//--------------------------------------------------------
//	import the XML
//--------------------------------------------------------
SyncBool TLFileSys::TFileTileMap::ExportAsset(TPtr<TLAsset::TAsset>& pAsset,Bool& Supported)
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
	TPtr<TXmlTag> pRootTag = m_XmlData.GetChild("TileMap");

	//	malformed XML
	if ( !pRootTag )
	{
		TLDebug_Print("Scheme file missing root <TileMap> tag");
		return SyncFalse;
	}

	//	make up new storage asset type
	TPtr<TLAsset::TTileMap> pNewAsset = new TLAsset::TTileMap( GetFileRef() );
	ImportResult = ImportTileMap( *pRootTag, *pNewAsset );

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
SyncBool TLFileSys::TFileTileMap::ImportTileMap(TXmlTag& Tag,TLAsset::TTileMap& TileMap)
{
	/*
<?xml version="1.0" ?>
<TileMap Width="2" Height="2" TileSize="16">
	<Tile>
		<Data DataRef="Texture"><TRef>Twitter</TRef></Data>
	</Tile>
	<Tile>
		<Data DataRef="Atlas"><TRef>BallBounce</TRef></Data>
		<Data DataRef="Frames"><u16>0</u16><u16>1</u16><u16>2</u16><u16>3</u16></Data>
		<Data DataRef="FrRate"><float>0.1</float></Data>
	</Tile>
	<Tile>
		<Data DataRef="Texture"><TRef>Theremin</TRef></Data>
	</Tile>
	<Tile>
		<Data DataRef="Atlas"><TRef>BallBounce</TRef></Data>
		<Data DataRef="Frames"><u16>0</u16></Data>
	</Tile>

</TileMap>
	*/
	
	//	get tilemap properties
	const TString* pWidthString = Tag.GetProperty("Width");
	const TString* pHeightString = Tag.GetProperty("Height");
	const TString* pTileSizeString = Tag.GetProperty("TileSize");
  
	if ( !pWidthString || !pHeightString || !pTileSizeString )
	{
		if ( !pWidthString )	TLDebug_Break("Missing Width property");
		if ( !pHeightString )	TLDebug_Break("Missing Height property");
		if ( !pTileSizeString )	TLDebug_Break("Missing TileSize property");
		return SyncFalse;
	}

		
	s32 Width;		pWidthString->GetInteger( Width );
	s32 Height;		pHeightString->GetInteger( Height );
	float TileSize;	pTileSizeString->GetFloat( TileSize );

	if ( Width < 1 || Height < 1 || Width >= 0xffff || Height >= 0xffff )
	{
		if ( !TLDebug_Break( TString("Invalid tile map size %dx%d", Width, Height ) ) )
			return SyncFalse;
	}

	//	init tilemap's info
	TileMap.SetSize( Type2<u16>( Width, Height ) );
	TileMap.SetTileSize( TileSize );

	//	now extract all the tile's
	for ( u32 c=0;	c<Tag.GetChildren().GetSize();	c++ )
	{
		TPtr<TXmlTag>& pChildTag = Tag.GetChildren().ElementAt(c);
		if ( pChildTag->GetTagName() != "tile" )
			continue;

		SyncBool TagImportResult = ImportTile( *pChildTag, TileMap );

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

	//	pad out any tiles we didn't specify in the xml
	TileMap.PadTiles();

	return SyncTrue;
}

//--------------------------------------------------------
//	
//--------------------------------------------------------
SyncBool TLFileSys::TFileTileMap::ImportTile(TXmlTag& Tag,TLAsset::TTileMap& TileMap)
{
	/*
	<Tile>
		<Data DataRef="Atlas"><TRef>BallBounce</TRef></Data>
		<Data DataRef="Frames"><u16>0</u16><u16>1</u16><u16>2</u16><u16>3</u16></Data>
		<Data DataRef="FrRate"><float>0.1</float></Data>
	</Tile>
	<Tile>
		<Data DataRef="Texture"><TRef>Theremin</TRef></Data>
	</Tile>
	.....
	*/


	//	create data for tile
	TPtr<TBinaryTree> pTileData = new TBinaryTree("Tile");

	//	import the data's into a new tile
	//	now extract all the glyphs's
	for ( u32 c=0;	c<Tag.GetChildren().GetSize();	c++ )
	{
		TXmlTag& ChildTag = *Tag.GetChildren().ElementAt(c);
		if ( ChildTag.GetTagName() != "data" )
			continue;

		if ( !TLFile::ParseXMLDataTree( ChildTag, *pTileData ) )
			return SyncFalse;
	}

	//	put our new tile data into the tile map
	TileMap.AddTile( pTileData );

	return SyncTrue;
}

