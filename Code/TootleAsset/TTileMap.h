/*------------------------------------------------------

	A tile map is for 2D tiled games. Specifies a rectangular grid
	with a "tile" in each point.

	A tile can just be null (an invalid texture reference), a texture 
	reference (full size sprite), an atlas frame (part of a texture) or 
	a list of atlas frames (to make an animation)

-------------------------------------------------------*/
#pragma once

#include "TLAsset.h"
#include "TAsset.h"
#include "TScheme.h"


namespace TLAsset
{
	class TTile;
	class TTileMap;
};


//----------------------------------------------
//	a tile in a tilemap
//----------------------------------------------
class TLAsset::TTile
{
public:
	TTile()	{}
	TTile(TPtr<TBinaryTree>& pData) : m_pData	( pData )	{}

	//	different types of tile
	bool				IsValid() const						{	return m_pData;	}
	bool				IsNullTile() const					{	return !IsValid();	}

	void				SetData(TPtr<TBinaryTree>& pData)	{	m_pData = pData;	}
	TBinaryTree&		GetData()							{	return *m_pData;	}
	TPtr<TBinaryTree>&	GetDataPtr()						{	return m_pData;	}

	TPtr<TBinaryTree>	GetRenderNodeData(const float3& Translate,float LayerZDepth=1.f);	//	generate init data for a render node - return NULL if we don't need to put a render node here

protected:
	TPtr<TBinaryTree>	m_pData;	//	data for this tile. never null
};



//----------------------------------------------
//	Tilemap
//----------------------------------------------
class TLAsset::TTileMap : public TLAsset::TAsset
{
public:
	TTileMap(TRefRef AssetRef);

	static TRef					GetAssetType_Static()			{	return TRef_Static(T,i,l,e,m);	}
	
	void						SetSize(const Type2<u16>& Size);	//	re-size grid
	u16							GetWidth() const				{	return m_GridSize.x;	}
	u16							GetHeight() const				{	return m_GridSize.y;	}
	u32							GetIndex(const Type2<u16>& xy)	{	return xy.x + ( xy.y * m_GridSize.x );	}
	TTile&						GetTile(const Type2<u16>& xy)	{	return m_Tiles[ GetIndex( xy ) ];	}
	TTile&						GetTile(u16 x,u16 y)			{	return GetTile( Type2<u16>( x,y ) );	}
	u32							GetTileCount() const			{	return m_GridSize.x * m_GridSize.y;	}

	void						SetTileSize(const float2& TileSize)		{	m_TileSize = TileSize;	}
	const float2&				GetTileSize() const						{	return m_TileSize;	}

	bool						AddTile(TPtr<TBinaryTree>& pTileData);	//	add a new tile, returns false if we cannot fit any more tiles or invalid data etc
	void						PadTiles();								//	add more tiles till we fill the array

	TPtr<TScheme>&				GetScheme()								{	return m_pScheme;	}

protected:
	virtual SyncBool			ImportData(TBinaryTree& Data);			//	load asset data out binary data
	virtual SyncBool			ExportData(TBinaryTree& Data);			//	save asset data to binary data

	bool						GenerateScheme(const int2& TileGroupSize);	//	generate a new scheme asset from the tiles
	void						DeleteScheme()							{	m_pScheme = NULL;	}

protected:
	THeapArray<TTile>	m_Tiles;		//	tile at each grid coordinate
	Type2<u16>			m_GridSize;		//	grid size
	float2				m_TileSize;		//	expected size of render node tiles - gr: maybe don't store that here?
	TPtr<TScheme>		m_pScheme;		//	tilemap converted into a scheme, for easy loading
};


