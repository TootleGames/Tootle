#include "TTileMap.h"




TLAsset::TTileMap::TTileMap(TRefRef AssetRef) :
	TAsset	( GetAssetType_Static(), AssetRef )
{
}





//-------------------------------------------------------
//	load asset data out binary data
//-------------------------------------------------------
SyncBool TLAsset::TTileMap::ImportData(TBinaryTree& Data)		
{
	if ( !Data.ImportData("Size", m_GridSize ) )
	{
		TLDebug_Break("Size data missing from tilemap");
		return SyncFalse;
	}

	if ( !Data.ImportData("TlSize", m_TileSize ) )
	{
		TLDebug_Break("TileSize data missing from tilemap");
		return SyncFalse;
	}

	//	import tiles
	TPtr<TBinaryTree>& pTileDataRoot = Data.GetChild("Tiles");
	if ( !pTileDataRoot )
		return SyncFalse;

	//	tile for each child of the data root
	TPtrArray<TBinaryTree>& TileDatas = pTileDataRoot->GetChildren();
	for ( u32 t=0;	t<TileDatas.GetSize();	t++ )
	{
		TPtr<TBinaryTree>& pTileData = TileDatas[t];

		AddTile( pTileData );
	}

	return SyncTrue;
}


//-------------------------------------------------------
//	save asset data to binary data
//-------------------------------------------------------
SyncBool TLAsset::TTileMap::ExportData(TBinaryTree& Data)				
{
	Data.ExportData("Size", m_GridSize );
	Data.ExportData("TLSize", m_TileSize );

	//	export tiles
	TPtr<TBinaryTree> pTileDataRoot = Data.AddChild("Tiles");

	for ( u32 t=0;	t<m_Tiles.GetSize();	t++ )
	{
		TTile& Tile = m_Tiles[t];

		//	just add the tile data straight under the <tiles> node
		pTileDataRoot->AddChild( Tile.GetDataPtr() );
	}

	return SyncTrue;
}	


//-------------------------------------------------------
//	re-size grid
//-------------------------------------------------------
void TLAsset::TTileMap::SetSize(const Type2<u16>& Size)
{
	//	todo: nice resize to re-align the tiles
	m_GridSize = Size;

	u32 TileCount = GetTileCount();

	//	shrink size
	if ( TileCount < m_Tiles.GetSize() )
	{
		m_Tiles.SetSize( TileCount );
	}

	//	for asset generation we set the size then fill it with tiles, so don't pad here
	//	gr: make this a param?
	/*
	else ( TileCount > m_Tiles.GetSize() )
	{
		PadTiles();
	}
	*/
}


//-------------------------------------------------------
//	add a new tile, returns false if we cannot fit any more tiles or invalid data etc
//-------------------------------------------------------
bool TLAsset::TTileMap::AddTile(TPtr<TBinaryTree>& pTileData)
{
	if ( !pTileData )
	{
		TLDebug_Break("Tile data expected");
		return false;
	}

	//	don't exceed tile count
	if ( m_Tiles.GetSize() >= GetTileCount() )
	{
		TLDebug_Break("Trying to add too many tiles to tilemap.");
		return false;
	}

	//	add a new tile
	TTile* pTile = m_Tiles.AddNew();
	if ( !pTile )
		return false;

	pTile->SetData( pTileData );

	return true;
}


//-------------------------------------------------------
//	add more tiles till we fill the array
//-------------------------------------------------------
void TLAsset::TTileMap::PadTiles()
{
	//	get number of tiles we want
	u32 TileCount = GetTileCount();

	while ( m_Tiles.GetSize() < TileCount )
	{
		TPtr<TBinaryTree> pTileData = new TBinaryTree("Tile");
		if ( !AddTile( pTileData ) )
		{
			TLDebug_Break("Failed to add tile");
			break;
		}
	}
}

