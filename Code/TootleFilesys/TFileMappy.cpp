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
	TArray<TLAsset::TTile>			m_BlockLibrary;		//	blocks by their index in the form of a tile
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
SyncBool TLFileSys::TFileMappy::ExportAsset(TPtr<TLAsset::TAsset>& pAsset,Bool& Supported)
{
	Supported = true;
	
	TPtr<TLAsset::TTileMap> pTilemap = new TLAsset::TTileMap( this->GetFileRef() );
	TPtr<TLAsset::TTexture> pTexture = new TLAsset::TTexture( this->GetFileRef() );

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
		case TRef_Static4(A,T,H,R):	Result = ImportAuthor( *pTilemap, Chunk.m_Data );	break;
		case TRef_Static4(M,P,H,D):	Result = ImportMapHeader( *pTilemap, Chunk.m_Data, ImportData );	break;
		case TRef_Static4(E,D,H,D):	Result = ImportEditorInfo( *pTilemap, Chunk.m_Data );	break;
		case TRef_Static4(C,M,A,P):	Result = ImportColourPalette( *pTilemap, Chunk.m_Data, ImportData );	break;
		case TRef_Static4(B,K,D,T):	Result = ImportBlockData( *pTilemap, Chunk.m_Data, ImportData );	break;
		case TRef_Static4(A,N,D,T):	Result = ImportAnimationData( *pTilemap, Chunk.m_Data );	break;
		case TRef_Static4(B,G,F,X):	Result = ImportGraphics( *pTilemap, Chunk.m_Data, ImportData );	break;
		case TRef_Static4(B,O,D,Y):	Result = ImportLayer( *pTilemap, Chunk.m_Data, 0 );	break;
		case TRef_Static4(L,Y,R,ONE):	Result = ImportLayer( *pTilemap, Chunk.m_Data, 1 );	break;
		case TRef_Static4(L,Y,R,TWO):	Result = ImportLayer( *pTilemap, Chunk.m_Data, 2 );	break;
		case TRef_Static4(L,Y,R,THREE):	Result = ImportLayer( *pTilemap, Chunk.m_Data, 3 );	break;
		case TRef_Static4(L,Y,R,FOUR):	Result = ImportLayer( *pTilemap, Chunk.m_Data, 4 );	break;
		case TRef_Static4(L,Y,R,FIVE):	Result = ImportLayer( *pTilemap, Chunk.m_Data, 5 );	break;
		case TRef_Static4(L,Y,R,SIX):	Result = ImportLayer( *pTilemap, Chunk.m_Data, 6 );	break;
		case TRef_Static4(L,Y,R,SEVEN):	Result = ImportLayer( *pTilemap, Chunk.m_Data, 7 );	break;
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

			TArray<TColour32>& TextureData = *ImportData.m_BlockTextures[BlockIndex];
			
			if ( !pTexture->PasteTextureData( Type2<u16>( x * BlockSize, y * BlockSize ), TextureData, ImportData.m_BlockSize ) )
				return SyncFalse;
		}
	}
	
	//	return the texture
	//	gr: need to support multiple asset output! (or at least choose what to return)
	pAsset = pTexture;
	
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

	MPHD Header;
	if ( !Data.Read( Header ) )
		return false;

	//	check tilemap size isn't too large
	if ( Header.mapwidth > 0xffff || Header.mapheight > 0xffff )
	{
		TLDebug_Break("Tilemap is too large for our format");
		return false;
	}

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

	//	read block datas and convert them to our type
	BLKSTR TempBlock;
	if ( sizeof(TempBlock) != ImportData.m_BlockDataSize )
		TLDebug_Break("block size is variable...");

	while ( Data.GetSizeUnread() > 0 )
	{
		if ( !Data.Read( TempBlock ) )
			return false;

		//	convert the block layers to a tile
		TPtr<TBinaryTree> pTileData = new TBinaryTree("Lyr0");

		//	todo: support all the bg/fg's as layers
		s32 GfxOffset = TempBlock.bgoff;

		//	gfx 0 is a no-tile tile
		if ( GfxOffset != 0 )
		{
			//	turn the graphic offset to a graphic index (this will become the atlas index)
			TFixedArray<u16,100> AtlasFrames;
			if ( GfxOffset > 0 )
			{
				u32 GfxOffsetScalar = (ImportData.m_BlockDepth / 8) * ImportData.m_BlockSize.x * ImportData.m_BlockSize.y;
				u32 AtlasIndex = GfxOffset / GfxOffsetScalar;
				AtlasFrames.Add( AtlasIndex );
			}
			else if ( GfxOffset < 0 )
			{
				//	negative number points to an animation index
				s32 AnimIndex = -GfxOffset;
				//	todo!
				AtlasFrames.Add( (u16)0 );

				//	todo: get framerate from animation data
				float FrameRate = 0.1f;
				pTileData->ExportData("FrRate", FrameRate);
			}

			//	export atlas frames
			pTileData->ExportArray("Frames", AtlasFrames);

			//	export atlas ref
			pTileData->ExportData("Atlas", ImportData.m_AtlasRef );

			//	export texture ref
			pTileData->ExportData("Texture", ImportData.m_TextureRef );
		}

		//	add block to tile lib
		ImportData.m_BlockLibrary.Add( TLAsset::TTile( pTileData ) );
	}

	return true;
}

//--------------------------------------------------------------
//	import chunk data - 
//--------------------------------------------------------------
Bool TLFileSys::TFileMappy::ImportAnimationData(TLAsset::TTileMap& TileMap,TBinary& Data)
{
	typedef struct { /* Animation control structure */
	signed char antype;	/* Type of anim, AN_? */
	signed char andelay;	/* Frames to go before next frame */
	signed char ancount;	/* Counter, decs each frame, till 0, then resets to andelay */
	signed char anuser;	/* User info */
	long int ancuroff;	/* Points to current offset in list */
	long int anstartoff;	/* Points to start of blkstr offsets list, AFTER ref. blkstr offset */
	long int anendoff;	/* Points to end of blkstr offsets list */
	} ANISTR;

	//	gr: not processing yet, something wrong with reading the struct atm
	return true;

	ANISTR AnimData;
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
Bool TLFileSys::TFileMappy::ImportLayer(TLAsset::TTileMap& TileMap,TBinary& Data,u8 LayerIndex)
{
	return true;
}

