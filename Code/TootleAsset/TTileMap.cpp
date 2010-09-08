#include "TTileMap.h"
#include <TootleAsset/TScheme.h>
#include <TootleCore/TLCore.h>
#include <TootleCore/TKeyArray.h>


//-------------------------------------------------------
//	if the tile has a flag, this is it
//-------------------------------------------------------
TRef TLAsset::TTile::GetFlag() const
{
	TLAsset::TTile& This = const_cast<TLAsset::TTile&>(*this);
	TBinaryTree& Data = This.GetData();
	TRef Flag;
	Data.ImportData( TRef_Static4(F,l,a,g), Flag );
	return Flag;
}


//-------------------------------------------------------
//	generate init data for a render node - return NULL if we don't need to put a render node here
//-------------------------------------------------------
TPtr<TBinaryTree> TLAsset::TTile::GetRenderNodeData(const float3& Translate,float LayerZDepth)
{
	//	add data for the layers
	TPtrArray<TBinaryTree> LayerDatas;
	
	//	first layer is the tile's data
	LayerDatas.Add( GetDataPtr() );	

	//	get the other layers
	GetData().GetChildren("Layer", LayerDatas );

	TPtr<TBinaryTree> pTileRenderData;

	for ( u32 Layer=0;	Layer<LayerDatas.GetSize();	Layer++ )
	{
		TBinaryTree& TileData = *LayerDatas[Layer];
		float LayerZOffset = (float)Layer * LayerZDepth;

		//	pull out texture and atlas
		TPtr<TBinaryTree>& pTextureData = TileData.GetChild( TRef_Static(T,e,x,t,u) );
		TPtr<TBinaryTree>& pAtlasData = TileData.GetChild( TRef_Static(A,t,l,a,s) );
		if ( !pTextureData || !pAtlasData )
			continue;

		//	if this is the first render node, then create the base data
		TPtr<TBinaryTree> pLayerData;
		if ( !pTileRenderData )
		{
			pTileRenderData = new TBinaryTree(TLCore::InitialiseRef);
			pLayerData = pTileRenderData;
		}
		else
		{
			pLayerData = pTileRenderData->AddChild("Child");
		}

		pLayerData->AddChild( pTextureData );
		pLayerData->AddChild( pAtlasData );
		float3 LayerTranslate( Translate.x, Translate.y, Translate.z + LayerZOffset );
		pLayerData->ExportData( TRef_Static(T,r,a,n,s), LayerTranslate );

		//	pull out first glyph from the frames - todo: anim support
		TFixedArray<u16,50> Frames;
		if ( !TileData.ImportArrays("Frames", Frames) )
			Frames.Add( (u16)0 );
		pLayerData->ExportData( TRef_Static(G,l,y,p,h), Frames[0] );

		//	export type
		pLayerData->ExportData( TRef_Static4(T,y,p,e), TRef_Static(S,p,r,i,t) );
	}

	return pTileRenderData;
}



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

	//	pre-alloc tiles
	m_Tiles.AddAllocSize( TileDatas.GetSize() );

	//	add tiles...
	for ( u32 t=0;	t<TileDatas.GetSize();	t++ )
	{
		TPtr<TBinaryTree>& pTileData = TileDatas[t];
		TTile* pNewTile = AddTile( pTileData );

		//	if this is a flagged tile, save off the coord
		TRef TileFlag = pNewTile ? pNewTile->GetFlag() : TRef();
		if ( TileFlag.IsValid() )
		{
			m_FlaggedTiles.Add( t );
		}			
	}

	//	import scheme asset
	TPtr<TBinaryTree>& pSchemeData = Data.GetChild("Scheme");
	if ( pSchemeData )
	{
		m_pScheme = new TScheme( GetAssetRef() );
		SyncBool ImportResult = m_pScheme->ImportData( *pSchemeData );
		if ( ImportResult != SyncTrue )
		{
			TLDebug_Break("failed to import scheme data in tile map");
			m_pScheme = NULL;
		}
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
	TBinaryTree* pTileDataRoot = Data.AddChild("Tiles");

	//	pre-alloc children
	pTileDataRoot->AddAllocChildren( m_Tiles.GetSize() );

	//	add tile data
	for ( u32 t=0;	t<m_Tiles.GetSize();	t++ )
	{
		TTile& Tile = m_Tiles[t];

		//	just add the tile data straight under the <tiles> node
		pTileDataRoot->AddChild( Tile.GetDataPtr() );
	}

	//	export scheme
	if ( !m_pScheme )
		GenerateScheme( int2(5,5) );

	if ( m_pScheme )
	{
		TPtr<TBinaryTree>& pSchemeData = Data.AddChild("Scheme");
		if ( pSchemeData )
		{
			if ( m_pScheme->ExportData( *pSchemeData ) != SyncTrue )
			{
				TLDebug_Break("Failed to export scheme data when exporting tile map");
				Data.RemoveChild( pSchemeData );
			}
		}
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
TLAsset::TTile* TLAsset::TTileMap::AddTile(TPtr<TBinaryTree>& pTileData)
{
	if ( !pTileData )
	{
		TLDebug_Break("Tile data expected");
		return NULL;
	}

	//	don't exceed tile count
	if ( m_Tiles.GetSize() >= GetTileCount() )
	{
		TLDebug_Break("Trying to add too many tiles to tilemap.");
		return NULL;
	}

	//	add a new tile
	TTile* pTile = m_Tiles.AddNew();
	if ( !pTile )
		return NULL;

	pTile->SetData( pTileData );

	return pTile;
}


//-------------------------------------------------------
//	add more tiles till we fill the array
//-------------------------------------------------------
void TLAsset::TTileMap::PadTiles()
{
	//	get number of tiles we want
	u32 TileCount = GetTileCount();

	//	pre-alloc data
	m_Tiles.AddAllocSize( TileCount );

	//	add new tiles
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


//-------------------------------------------------------
//	generate a new scheme asset from the tiles
//-------------------------------------------------------
bool TLAsset::TTileMap::GenerateScheme(const int2& TileGroupSize)
{
	if ( TileGroupSize.x < 1 || TileGroupSize.y < 1 )
	{
		TLDebug_Break("TileGroupSize must never be zero");
		return false;
	}

	//	make up new scheme
	m_pScheme = new TLAsset::TScheme( GetAssetRef() );

	//	generate render node init messages for each tile
	int2 Group(0,0);
	TPtrKeyArray<int2,TSchemeNode> GroupNodes;

	for ( u32 x=0;	x<GetWidth();	x++ )
	{
		for ( u32 y=0;	y<GetHeight();	y++ )
		{
			//	generate the data for this tile
			TTile& Tile = GetTile( Type2<u16>( x, y ) );
			TPtr<TBinaryTree> pRenderNodeData = Tile.GetRenderNodeData( float3(x,y,0) );
			if ( !pRenderNodeData )
				continue;

			//	get group index
			int2 GroupIndex( x/TileGroupSize.x, y/TileGroupSize.y );
			TPtr<TSchemeNode> pGroupNode = GroupNodes.FindPtr( GroupIndex );

			//	if there's no group yet, make it up
			if ( !pGroupNode )
			{
				//	add to index and add to scheme
				pGroupNode = new TSchemeNode();
				GroupNodes.Add( GroupIndex, pGroupNode );
				m_pScheme->AddNode( pGroupNode );
			}

			//	add to group
			TRef Type;
			pRenderNodeData->ImportData( TRef_Static4(T,y,p,e), Type );
			TPtr<TSchemeNode> pRenderNodeNode = new TSchemeNode( TRef(), TRef(), Type );
			pRenderNodeNode->GetData().ReferenceDataTree( *pRenderNodeData );
			pGroupNode->AddChild( pRenderNodeNode );
		}
	}

	return true;
}
