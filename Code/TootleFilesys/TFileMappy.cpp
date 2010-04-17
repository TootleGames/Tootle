/*
 *  TFileMappy.cpp
 *  TootleFilesys
 *
 *  Created by Graham Reeves on 30/03/2010.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *



 The first 12 bytes are as follows:
4bytes ASCII = 'FORM'
long int = size of file less header (which is filesize-8)
4bytes ASCII = 'FMAP'
NOTE: The chunk size long ints like the one above are stored in Motorola format, NOT Intel. You will have to byteswap to get the correct value, ie: Bytes 1,2,3,4 need to become 4,3,2,1.

The chunks in the file follow on one after the other, and consist of an 8byte header, and the information specific to that chunk. See how the playback source reads in the information. The chunks can be in any order, and some chunks may not be used in a particular file. Also, don't rely on chunks being a certain size, for example the MPHD is now 4 bytes bigger than in the last version

Chunk header:
4bytes ASCII = ChunkID (example: 'MPHD')
long int = size of chunk data less header

These are the chunks as of V1.2:
ATHR - Up to 4 ASCII strings of author information, separated by 0 values, always an even size.
MPHD - Map header, see struct in the editor source download
EDHD - Editor information, see struct in mappy.c
CMAP - Colour palette for 8bit maps, red byte, green byte, blue byte for however many colours are needed (so usually 256*3 bytes).
BKDT - Block data. Contains BLKSTR structures for however many block structures were made.
ANDT - Animation data. Contains ANISTR structures for however many animation structures were made, and also animation data.
BGFX - The raw graphics in whatever format the map is in. Examples: 8bit: mapwidth*mapheight bytes per block, in forward format *numblocks 16bit: mapwidth*mapheight*2 bytes per block, each word contains 5 bits red, 6 bits green, 5 bits blue.
BODY - An array of short ints containing positive offsets into BKDT, and negative offsets into ANDT.
LYR? - Where ? is an ASCII number form 1 to 7. These are the same size and format as BODY, and allow object layers to be used.
You can add your own chunks to a map file, if you load it into mappy, when you save it, those additional chunks will be saved in the file, but not necessarily in the same place as before.


 */

#include "TFileMappy.h"
#include <TootleAsset/TTileMap.h>
#include <TootleAsset/TTexture.h>
#include <TootleAsset/TAtlas.h>

namespace Mappy
{
	class TIdent
	{
	public:
		TRef	m_IdentAsRef;	//	4-chars turned into a ref for easy use

		Bool	Read(TBinary& Data)
		{
			char Ascii[5] = {0,0,0,0,0};
			if ( !Data.Read( Ascii[0] ) )	return false;
			if ( !Data.Read( Ascii[1] ) )	return false;
			if ( !Data.Read( Ascii[2] ) )	return false;
			if ( !Data.Read( Ascii[3] ) )	return false;

			//	turn into ref
			m_IdentAsRef = Ascii;
			return true;
		}
	};

	//	mappy's int's are in little endian rather than big endian so we need to swap when we read
	class u32Swap
	{
	public:
		u32		m_Value;

		Bool	Read(TBinary& Data)
		{
			u8* pValue = (u8*)&m_Value;

			//	read in reverse order
			if ( !Data.Read( pValue[3] ) )	return false;
			if ( !Data.Read( pValue[2] ) )	return false;
			if ( !Data.Read( pValue[1] ) )	return false;
			if ( !Data.Read( pValue[0] ) )	return false;

			return true;
		}
	};

	class THeader
	{
	public:
		TIdent		m_FORM;		//	4bytes ASCII = 'FORM'
		u32Swap		m_FileSize;	//	little endian! size of file less header (which is filesize-8)
		TIdent		m_FMAP;		//	4bytes ASCII = 'FMAP'

		Bool		Read(TBinary& Data)
		{
			if ( !m_FORM.Read( Data ) )		return false;
			if ( !m_FileSize.Read( Data ) )	return false;
			if ( !m_FMAP.Read( Data ) )		return false;
			return true;
		}
	};

	class TChunk
	{
	public:
		TIdent		m_Ident;		//	4bytes ASCII = ChunkID (example: 'MPHD')
		TBinary		m_Data;			//	data in chunk

		Bool		Read(TBinary& Data)
		{
			if ( !m_Ident.Read( Data ) )	
				return false;

			//	size of chunk data less header
			u32Swap DataSize;			
			if ( !DataSize.Read( Data ) )	
				return false;

			//	read block of data
			if ( !Data.Read( m_Data, DataSize.m_Value ) )
				return false;

			//	prepare for processing
			m_Data.ResetReadPos();

			return true;
		}
	};

	class TImportData;	//	import data
	
	
	
	
	typedef struct {	/* Map header structure */
		/* char M,P,H,D;	4 byte chunk identification. */
		/* long int mphdsize;	size of map header. */
		char mapverhigh;	/* map version number to left of . (ie X.0). */
		char mapverlow;		/* map version number to right of . (ie 0.X). */
		char lsb;		/* if 1, data stored LSB first, otherwise MSB first. */
		char maptype;	/* 0 for 32 offset still, -16 offset anim shorts in BODY added FMP0.5*/
		short int mapwidth;	/* width in blocks. */
		short int mapheight;	/* height in blocks. */
		short int reserved1;
		short int reserved2;
		short int blockwidth;	/* width of a block (tile) in pixels. */
		short int blockheight;	/* height of a block (tile) in pixels. */
		short int blockdepth;	/* depth of a block (tile) in planes (ie. 256 colours is 8) */
		short int blockstrsize;	/* size of a block data structure */
		short int numblockstr;	/* Number of block structures in BKDT */
		short int numblockgfx;	/* Number of 'blocks' in graphics (BODY) */
		unsigned char ckey8bit, ckeyred, ckeygreen, ckeyblue; /* colour key values added FMP0.4*/
		/* info for non rectangular block maps added FMP0.5*/
		short int blockgapx, blockgapy, blockstaggerx, blockstaggery;
		short int clickmask, pillars;
	} MPHD;
	
	typedef struct {	/* Structure for data blocks */
		long int bgoff, fgoff;	/* offsets from start of graphic blocks */
		long int fgoff2, fgoff3; /* more overlay blocks */
		unsigned long int user1, user2;	/* user long data */
		unsigned short int user3, user4;	/* user short data */
		unsigned char user5, user6, user7;	/* user byte data */
		unsigned char tl : 1;	/* bits for collision detection */
		unsigned char tr : 1;
		unsigned char bl : 1;
		unsigned char br : 1;
		unsigned char trigger : 1;	/* bit to trigger an event */
		unsigned char unused1 : 1;
		unsigned char unused2 : 1;
		unsigned char unused3 : 1;
	} BLKSTR;
	
	typedef struct { /* Animation control structure */
		signed char antype;	/* Type of anim, AN_? */
		signed char andelay;	/* Frames to go before next frame */
		signed char ancount;	/* Counter, decs each frame, till 0, then resets to andelay */
		signed char anuser;	/* User info */
		long int ancuroff;	/* Points to current offset in list */
		long int anstartoff;	/* Points to start of blkstr offsets list, AFTER ref. blkstr offset */
		long int anendoff;	/* Points to end of blkstr offsets list */
	} ANISTR;

	typedef struct {	/* Editor prefs structure */
		/* char E,D,H,D;	4 byte chunk identification. */
		/* long int edhdsize;	size of editor header. */
		short int xmapoffset;	/* editor offset, in blocks, from left. */
		short int ymapoffset;	/* editor offset, in blocks, from right. */
		long int fgcolour;	/* fg colour for text, buttons etc. */
		long int bgcolour;	/* bg colour for text, buttons etc. */
		short int swidth;		/* width of current screen res */
		short int sheight;	/* height of current screen res */
		short int strtstr;	/* first structure in view */
		short int strtblk;	/* first block graphic in view */
		short int curstr;		/* current block structure */
		short int curanim;	/* current anim structure */
		short int animspd;	/* gap in frames between anims */
		short int span;		/* control panel height */
		short int numbrushes;	/* number of brushes to follow. */
		short int pad;
	} EDHD;

}



//-----------------------------------------
//	import data
//-----------------------------------------
class Mappy::TImportData
{
public:
	TFixedArray<TColour32,256>		m_ColourPalette;	//	colour palette for graphics
	TPtrArray<TBinaryTree>			m_BlockData;		//	block data (this isn't 1:1 with tiles, they're indexed so we have to store them off
	u16								m_BlockDepth;		//	plane-depth of blocks...???
	u16								m_BlockDataSize;	//	size of the block data
	Type2<u16>						m_BlockSize;		//	block texture dimensions
	TPtrArray<TArray<TColour32> >	m_BlockTextures;	//	texture data for blocks (not sure as texture assets atm because these might not be square/power2 etc)
	TPtrArray<TFixedArray<s32,4> >	m_BlockLibrary;		//	blocks by index in the form of an array of atlas indexes (4 because mappy has 4 graphics per block)
	TRef							m_AtlasRef;			//	ref for the atlas that will be generated
	TRef							m_TextureRef;		//	ref for the texture that will be generated
	u8								m_TransparentPaletteIndex;	//	when using a palette, this is the transparent colour index
	TColour24						m_TransparentColour;	//	in 16bit colours, this is the transparent colour
};


//---------------------------------------------------------
//	
//---------------------------------------------------------
TLFileSys::TFileMappy::TFileMappy(TRefRef FileRef,TRefRef FileTypeRef) :
	TFile	( FileRef, FileTypeRef )
{
}



TLFileSys::TFileMappy::~TFileMappy()
{
}


//--------------------------------------------------------------
//	turn this file into an asset
//--------------------------------------------------------------	
SyncBool TLFileSys::TFileMappy::ExportAsset(TPtr<TLAsset::TAsset>& pAsset,TRefRef ExportAssetType)
{
	if ( pAsset )
	{
		TLDebug_Break("Async not supported.");
		return SyncFalse;
	}

	TPtr<TLAsset::TTileMap> pTileMap = new TLAsset::TTileMap( this->GetFileRef() );
	TPtr<TLAsset::TTexture> pTexture = new TLAsset::TTexture( this->GetFileRef() );
	TPtr<TLAsset::TAtlas> pAtlas = new TLAsset::TAtlas( this->GetFileRef() );
	
	//	set the resulting asset as requested
	switch ( ExportAssetType.GetData() )
	{
		case TRef_Static(T,e,x,t,u):	pAsset = pTexture;	break;
		case TRef_Static(A,t,l,a,s):	pAsset = pAtlas;	break;
		case TRef_Static(T,i,l,e,M):	pAsset = pTileMap;	break;
		default:
		{
			TDebugString Debug_String;
			Debug_String << "TFileMappy doesn't support exporting asset type " << ExportAssetType;
			return SyncFalse;
		}
	};

	//	load header
	ResetReadPos();

	Mappy::THeader Header;
	if ( !Header.Read( GetData() ) )	
		return SyncFalse;

	//	verify header
	if ( Header.m_FORM.m_IdentAsRef != "FORM" )
		return SyncFalse;
	if ( Header.m_FMAP.m_IdentAsRef != "FMAP" )
		return SyncFalse;
	
	//	import data for processing the chunks
	Mappy::TImportData ImportData;
	ImportData.m_AtlasRef = pAtlas->GetAssetRef();
	ImportData.m_TextureRef = pTexture->GetAssetRef();

	//	load chunks!
	while ( GetData().GetSizeUnread() > 0 )
	{
		Mappy::TChunk Chunk;
		if ( !Chunk.Read( GetData() ) )
			return SyncFalse;
	
		//	do somethign with the chunk!
		Bool Result = true;
		switch ( Chunk.m_Ident.m_IdentAsRef.GetData() )
		{
		case TRef_Static4(A,T,H,R):	Result = ImportAuthor( *pTileMap, Chunk.m_Data );	break;
		case TRef_Static4(M,P,H,D):	Result = ImportMapHeader( *pTileMap, Chunk.m_Data, ImportData );	break;
		case TRef_Static4(E,D,H,D):	Result = ImportEditorInfo( *pTileMap, Chunk.m_Data );	break;
		case TRef_Static4(C,M,A,P):	Result = ImportColourPalette( *pTileMap, Chunk.m_Data, ImportData );	break;
		case TRef_Static4(B,K,D,T):	Result = ImportBlockData( *pTileMap, Chunk.m_Data, ImportData );	break;
		case TRef_Static4(A,N,D,T):	Result = ImportAnimationData( *pTileMap, Chunk.m_Data );	break;
		case TRef_Static4(B,G,F,X):	Result = ImportGraphics( *pTileMap, Chunk.m_Data, ImportData );	break;
		case TRef_Static4(B,O,D,Y):	Result = ImportLayer( *pTileMap, Chunk.m_Data, ImportData, 0 );	break;
		case TRef_Static4(L,Y,R,ONE):	Result = ImportLayer( *pTileMap, Chunk.m_Data, ImportData, 1 );	break;
		case TRef_Static4(L,Y,R,TWO):	Result = ImportLayer( *pTileMap, Chunk.m_Data, ImportData, 2 );	break;
		case TRef_Static4(L,Y,R,THREE):	Result = ImportLayer( *pTileMap, Chunk.m_Data, ImportData, 3 );	break;
		case TRef_Static4(L,Y,R,FOUR):	Result = ImportLayer( *pTileMap, Chunk.m_Data, ImportData, 4 );	break;
		case TRef_Static4(L,Y,R,FIVE):	Result = ImportLayer( *pTileMap, Chunk.m_Data, ImportData, 5 );	break;
		case TRef_Static4(L,Y,R,SIX):	Result = ImportLayer( *pTileMap, Chunk.m_Data, ImportData, 6 );	break;
		case TRef_Static4(L,Y,R,SEVEN):	Result = ImportLayer( *pTileMap, Chunk.m_Data, ImportData, 7 );	break;
		default:
			{
				//	gr: just show this as a warning, extra (non-standard) chunks are okay, it's just we don't use them
				TTempString Debug_String;
				Debug_String << "Unknown mappy chunk id: " << Chunk.m_Ident.m_IdentAsRef;
				TLDebug_Print( Debug_String );
				//Result = false;
			}
			break;
		}

		//	something failed in the import, abort
		if ( !Result )
			return SyncFalse;
	}

	//	gr: create the texture from all our data by packing the block textures together
	//	work out the box we're going to put them in to make a square texture (eg. 99 blocks = sqrt(99) = 9.94 == 10x10 == 100 block spaces (1 spare)
	//	gr: if the blocks aren't square, we could pack this more efficiently...
	
	//	work out what NxN size we want
	float TextureBlockSpanf = TLMaths::Sqrtf( (float)ImportData.m_BlockTextures.GetSize() );
	u32 TextureBlockSpan = (u32)( TextureBlockSpanf + 0.5f );	//	+0.5f to round up

	//	work out the texture MxM overall size to put all the blocks in
	u32 BlockSize = (ImportData.m_BlockSize.x > ImportData.m_BlockSize.y ) ? ImportData.m_BlockSize.x : ImportData.m_BlockSize.y;
	u32 TextureSize = (TextureBlockSpan+1) * BlockSize;
	if ( !TLMaths::IsPowerOf2( TextureSize ) )
		TextureSize = TLMaths::GetNextPowerOf2( TextureSize );

	//	setup texture 
	if ( !pTexture->SetSize( Type2<u16>( TextureSize, TextureSize ), true ) )
		return SyncFalse;

	//	paste in our textures
	//	todo: create the atlas whilst we do this
	for ( u32 y=0;	y<TextureBlockSpan;	y++ )
	{
		for ( u32 x=0;	x<TextureBlockSpan;	x++ )
		{
			u32 BlockIndex = x + ( y * TextureBlockSpan );
			if ( BlockIndex >= ImportData.m_BlockTextures.GetSize() )
				break;

			//	get the texture data we're about to paste into the main texture
			TArray<TColour32>& TextureData = *ImportData.m_BlockTextures[BlockIndex];

			//	get the glyph where we're pasting the texture to put into the atlas
			TLMaths::TBox2D PastedGlyph;
			
			if ( !pTexture->PasteTextureData( Type2<u16>( x * BlockSize, y * BlockSize ), TextureData, ImportData.m_BlockSize, &PastedGlyph ) )
				return SyncFalse;

			//	put glyph into atlas
			if ( pAtlas )
			{
				u16 AtlasKey = BlockIndex;
				TLAsset::TAtlasGlyph Glyph( PastedGlyph, pTexture->GetSize() );
				pAtlas->AddGlyph( AtlasKey, Glyph );
			}
		}
	}
	
	return SyncTrue;
}	


//--------------------------------------------------------------
//	import chunk data - 
//--------------------------------------------------------------
Bool TLFileSys::TFileMappy::ImportAuthor(TLAsset::TTileMap& TileMap,TBinary& Data)
{
	//	gr: don't care about author data
	return true;
}



//--------------------------------------------------------------
//	import chunk data - 
//--------------------------------------------------------------
Bool TLFileSys::TFileMappy::ImportMapHeader(TLAsset::TTileMap& TileMap,TBinary& Data,Mappy::TImportData& ImportData)
{

	Mappy::MPHD Header;
	if ( !Data.Read( Header ) )
		return false;

	/*	gr: redundant, the mapwidth/height is a u16
	//	check tilemap size isn't too large
	if ( Header.mapwidth > 0xffff || Header.mapheight > 0xffff )
	{
		TLDebug_Break("Tilemap is too large for our format");
		return false;
	}
	 */

	//	set tilemap size
	TileMap.SetSize( Type2<u16>( Header.mapwidth, Header.mapheight ) );

	//	pad tiles so we have data to work with directly
	TileMap.PadTiles();

	//	set tile size
	TileMap.SetTileSize( float2( Header.blockwidth, Header.blockheight ) );

	//	store block depth info
	ImportData.m_BlockDepth = Header.blockdepth;
	ImportData.m_BlockDataSize = Header.blockstrsize;
	ImportData.m_BlockSize.x = Header.blockwidth;
	ImportData.m_BlockSize.y = Header.blockheight;

	//	store transparent colour info
	ImportData.m_TransparentPaletteIndex = Header.ckey8bit;
	ImportData.m_TransparentColour = TColour24( Header.ckeyred, Header.ckeygreen, Header.ckeyblue );

	return true;
}


//--------------------------------------------------------------
//	import chunk data - 
//--------------------------------------------------------------
Bool TLFileSys::TFileMappy::ImportEditorInfo(TLAsset::TTileMap& TileMap,TBinary& Data)
{
	//	gr: not fussed about editor info
	return true;
}

//--------------------------------------------------------------
//	import chunk data - 
//--------------------------------------------------------------
Bool TLFileSys::TFileMappy::ImportColourPalette(TLAsset::TTileMap& TileMap,TBinary& Data,Mappy::TImportData& ImportData)
{
	//	data is all RGB bytes
	TColour24 TempColour;
	while ( Data.GetSizeUnread() > 0 )
	{
		if ( !Data.Read(TempColour) )
			return false;

		u8 Alpha = 255;

		//	if this colour is the transparent colour (aka, colour key) then set no alpha
		if ( ImportData.m_ColourPalette.GetSize() == ImportData.m_TransparentPaletteIndex )
			Alpha = 0;

		TColour32 TempColour32( TempColour, Alpha );
		ImportData.m_ColourPalette.Add( TempColour32 );
	}

	return true;
}

//--------------------------------------------------------------
//	import chunk data - 
//--------------------------------------------------------------
Bool TLFileSys::TFileMappy::ImportBlockData(TLAsset::TTileMap& TileMap,TBinary& Data,Mappy::TImportData& ImportData)
{	
	//	read block datas and convert them to our type
	Mappy::BLKSTR TempBlock;
	if ( sizeof(TempBlock) != ImportData.m_BlockDataSize )
		TLDebug_Break("block size is variable...");

	while ( Data.GetSizeUnread() > 0 )
	{
		if ( !Data.Read( TempBlock ) )
			return false;

		//	convert the block layers to a tile
		TPtr<TFixedArray<s32,4> > pBlockData = new TFixedArray<s32,4>;

		TFixedArray<long,4> GfxOffsets;
		GfxOffsets.Add( TempBlock.bgoff );
		GfxOffsets.Add( TempBlock.fgoff );
		GfxOffsets.Add( TempBlock.fgoff2 );
		GfxOffsets.Add( TempBlock.fgoff3 );

		for ( u32 i=0;	i<GfxOffsets.GetSize();	i++ )
		{
			long GfxOffset = GfxOffsets[i];
			if ( GfxOffset < 0 )
			{
				TLDebug_Break("Not expecting negative gfx index");
				GfxOffset = 0;
			}

			u32 GfxOffsetScalar = (ImportData.m_BlockDepth / 8) * ImportData.m_BlockSize.x * ImportData.m_BlockSize.y;
			u16 AtlasIndex = (u32)GfxOffset / GfxOffsetScalar;

			pBlockData->Add( AtlasIndex );
		}

		//	add block to block lib
		ImportData.m_BlockLibrary.Add( pBlockData );
	}

	return true;
}

//--------------------------------------------------------------
//	import chunk data - 
//--------------------------------------------------------------
Bool TLFileSys::TFileMappy::ImportAnimationData(TLAsset::TTileMap& TileMap,TBinary& Data)
{

	//	gr: not processing yet, something wrong with reading the struct atm
	return true;

	Mappy::ANISTR AnimData;
	while ( Data.GetSizeUnread() > 0 )
	{
		if ( !Data.Read(AnimData) )
			return false;

#define ANIM_FRAMERATE	(1.f/60.f)
		float FrameRate = (float)AnimData.andelay * ANIM_FRAMERATE;
		u8 FrameCount = AnimData.ancount;
	}

	return true;
}

//--------------------------------------------------------------
//	import chunk data - 
//--------------------------------------------------------------
Bool TLFileSys::TFileMappy::ImportGraphics(TLAsset::TTileMap& TileMap,TBinary& Data,Mappy::TImportData& ImportData)
{
	/*	The raw graphics in whatever format the map is in. Examples: 8bit: mapwidth*mapheight 
		bytes per block, in forward format *numblocks 16bit: mapwidth*mapheight*2 bytes per block, 
		each word contains 5 bits red, 6 bits green, 5 bits blue.
		*/

	if ( ImportData.m_BlockDepth == 8 )
	{
		//	8 bit data is paletted
		while ( Data.GetSizeUnread() > 0 )
		{
			//	read blocks of palette indexes...
			TArray<u8> PaletteGfx;
			if ( !Data.ReadDataIntoArray( PaletteGfx, ImportData.m_BlockSize.x * ImportData.m_BlockSize.y ) )
				return false;
			
			//	convert paletted graphic into a texture format we can use...
			TPtr<TArray<TColour32> > pBlockTextureData = new TArray<TColour32>;
			TArray<TColour32>& BlockTextureData = *pBlockTextureData;
			for ( u32 i=0;	i<PaletteGfx.GetSize();	i++ )
			{
				u8 PaletteIndex = PaletteGfx[i];
				TColour32& PaletteColour = ImportData.m_ColourPalette[PaletteIndex];
				BlockTextureData.Add( PaletteColour );
			}

			ImportData.m_BlockTextures.Add( pBlockTextureData );
		}
		return true;
	}
	
	if ( ImportData.m_BlockDepth == 16 )
	{
		//	16 bit data is colour data; TColour16; R5 G6 B5
		TLDebug_Break("Unhandled format 16 bit");
		return false;
	}
	
	TTempString Debug_String;
	Debug_String << "Unhandled block depth: " << ImportData.m_BlockDepth;
	TLDebug_Break( Debug_String );
	return false;
}

//--------------------------------------------------------------
//	import chunk data - 
//--------------------------------------------------------------
Bool TLFileSys::TFileMappy::ImportLayer(TLAsset::TTileMap& TileMap,TBinary& Data,Mappy::TImportData& ImportData,u8 LayerIndex)
{
	/*
	BODY - An array of short ints containing positive offsets into BKDT, and negative offsets into ANDT.
LYR? - Where ? is an ASCII number form 1 to 7. These are the same size and format as BODY, and allow object layers to be used.
You can add your own chunks to a map file, if you load it into mappy, when you save it, those additional chunks will be saved in the file, but not necessarily in the same place as before.
*/
	//	make up the tile map entries for this layer
	TArray<s16> LayerBlockIndexes;
	if ( !Data.ReadDataIntoArray( LayerBlockIndexes, TileMap.GetWidth() * TileMap.GetHeight() ) )
		return false;

	//	gr: only layer 0 atm until I change the tilemap format to support layers
	if ( LayerIndex != 0 )
		return true;

	//	convert indexes into tiles
	for ( u16 y=0;	y<TileMap.GetHeight();	y++ )
	{
		for ( u16 x=0;	x<TileMap.GetWidth();	x++ )
		{
			//	get tile
			TLAsset::TTile& Tile = TileMap.GetTile( x, y );
			TBinaryTree& TileData = Tile.GetData();

			u32 TileIndex = x + ( y * TileMap.GetWidth() );
			s16 BlockIndex = LayerBlockIndexes[ TileIndex ];
			
			//	todo: animated tiles have negative indexes
			if ( BlockIndex < 0 )
			{
				BlockIndex /= 16;
				BlockIndex = 0;
			}
			else
			{
				BlockIndex /= ImportData.m_BlockDataSize;
			}

			//	get the atlas indexes at this layer's tile
			TFixedArray<s32,4>& BlockAtlasIndexes = *ImportData.m_BlockLibrary[ BlockIndex ];
			for ( u32 Layer=0;	Layer<BlockAtlasIndexes.GetSize();	Layer++ )
			{
				s32 BlockAtlasIndex = BlockAtlasIndexes[Layer];

				//	block atlas index 0 is the transparent one
				//	todo: make this index -1 if it's the mappy transparent one and ditch graphic tile 0 as it's all-transparent.
				if ( BlockAtlasIndex <= 0 )
					continue;

				//	get pointer to data, if it's not the base layer then alloc a layer data
				TBinaryTree* pTileLayerData = (Layer==0) ? &TileData : TileData.AddChild("Layer").GetObjectPointer();
				if ( !pTileLayerData )
					continue;

				TBinaryTree& TileLayerData = *pTileLayerData;

				//	export tile data
				TileLayerData.ExportData("Atlas", ImportData.m_AtlasRef );
				TileLayerData.ExportData("Texture", ImportData.m_TextureRef );

				TFixedArray<u16,100> AtlasFrames;
				AtlasFrames << (u16)BlockAtlasIndex;
				TileLayerData.ExportArray("Frames", AtlasFrames );
			}
		}
	}

	return true;
}

