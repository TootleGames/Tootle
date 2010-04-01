#include "TFilePng.h"
#include <TootleAsset/TTexture.h>

namespace libpng
{
	#include <libpng/png.h>
}

namespace TLFilePng
{
	void	ReadData(libpng::png_structp png_ptr,libpng::png_bytep data, libpng::png_size_t length);
	void	OnReadRow(libpng::png_structp png_ptr,libpng::png_uint_32 row,int pass);

	//	wrappers for the PNG reading as it's stupid long jump system won't allow us to use returns properly
	void	PngReadHeader(libpng::png_structp png_ptr,libpng::png_infop info_ptr,Bool& Success);
	void	PngReadImage(libpng::png_structp png_ptr,TArray<u8*>& RowData,Bool& Success);
	void	PngReadEnd(libpng::png_structp png_ptr,Bool& Success);

	void	AllocRowData(TArray<u8*>& RowData,libpng::png_infop info_ptr,u32& RowDataSize,u32& TotalDataSize);
	void	DeleteRowData(TArray<u8*>& RowData);
}



void TLFilePng::AllocRowData(TArray<u8*>& RowData,libpng::png_infop info_ptr,u32& RowDataSize,u32& TotalDataSize)
{
	RowDataSize = info_ptr->rowbytes;
//	RowDataSize = info_ptr->width * info_ptr->channels;
	TotalDataSize = 0;

	//	alloc rows of data
	for ( u32 y=0;	y<info_ptr->height;	y++ )
	{
		RowData.Add( new u8[RowDataSize] );

		//	count data
		TotalDataSize += RowDataSize;
	}
}


void TLFilePng::DeleteRowData(TArray<u8*>& RowData)
{
	for ( u32 i=0;	i<RowData.GetSize();	i++ )
		TLMemory::DeleteArray( RowData[i] );

	RowData.Empty();
}


//-------------------------------------------------------
//	read header info
//-------------------------------------------------------
void TLFilePng::PngReadHeader(libpng::png_structp png_ptr,libpng::png_infop info_ptr,Bool& Success)
{
	if (libpng::setjmp(png_jmpbuf(png_ptr)))
		return;

	libpng::png_read_info(png_ptr, info_ptr);

	//	if we got here and the longjump shit didnt get used, the set success to true
	Success = TRUE;
}


//-------------------------------------------------------
//	read image data
//-------------------------------------------------------
void TLFilePng::PngReadImage(libpng::png_structp png_ptr,TArray<u8*>& RowData,Bool& Success)
{
	if (libpng::setjmp(png_jmpbuf(png_ptr)))
		return;

	libpng::png_read_image(png_ptr, RowData.GetData() );

	//	if we got here and the longjump shit didnt get used, the set success to true
	Success = TRUE;
}



//-------------------------------------------------------
//	read end of file
//-------------------------------------------------------
void TLFilePng::PngReadEnd(libpng::png_structp png_ptr,Bool& Success)
{
	if (libpng::setjmp(png_jmpbuf(png_ptr)))
		return;

	libpng::png_read_end(png_ptr, NULL );

	//	if we got here and the longjump shit didnt get used, the set success to true
	Success = TRUE;
}



void TLFilePng::ReadData(libpng::png_structp png_ptr,libpng::png_bytep data, libpng::png_size_t length)
{
	/*

	//	error
	if (libpng::setjmp(png_jmpbuf(png_ptr)))
    {
        TLDebug_Break("errrrr");
    }
*/
	//	get pointer back to our object
	TLFileSys::TFilePng* pFilePng = (TLFileSys::TFilePng*)png_ptr->io_ptr;
	
	//	read X bytes of data out of the file
	pFilePng->GetData().ReadData( data, length );
}

void TLFilePng::OnReadRow(libpng::png_structp png_ptr,libpng::png_uint_32 row,int pass)
{
/*
//	error
	if (libpng::setjmp(png_jmpbuf(png_ptr)))
    {
        TLDebug_Break("errrrr");
    }
*/
	TLDebug_Break("todo");
}





//---------------------------------------------------------
//	
//---------------------------------------------------------
TLFileSys::TFilePng::TFilePng(TRefRef FileRef,TRefRef FileTypeRef) :
	TFile	( FileRef, FileTypeRef )
{
}


//--------------------------------------------------------------
//	parse PNG file and turn it into a texture asset
//--------------------------------------------------------------	
SyncBool TLFileSys::TFilePng::ExportAsset(TPtr<TLAsset::TAsset>& pAsset,TRefRef ExportAssetType)
{
	if ( pAsset )
	{
		TLDebug_Break("Async export not supported yet. asset should be NULL");
		return SyncFalse;
	}

	//	check first 8 bytes for validity
	TFixedArray<u8,8> Header(8);

	//	gr: use the TBinary functions to move the read pos along
	GetData().ResetReadPos();
	GetData().ReadData( Header.GetData(), Header.GetSize() );

	//	png header check
	Bool HeaderValid = libpng::png_sig_cmp( Header.GetData(), 0, Header.GetSize() ) == 0;
	if ( !HeaderValid )
	{
		TLDebug_Break("png has invalid header - not a png?");
		return SyncFalse;
	}

	//	alloc some internal structures needed for processing - no custom error/warning handlers
	libpng::png_structp png_ptr = libpng::png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL );
	libpng::png_infop info_ptr = png_ptr ? libpng::png_create_info_struct(png_ptr) : NULL;
	libpng::png_infop end_info = png_ptr ? libpng::png_create_info_struct(png_ptr) : NULL;

	if ( !png_ptr || !info_ptr || !end_info )
	{
		TLDebug_Break("error allocating libpng structs");
		libpng::png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
		return SyncFalse;
    }

	//	use our own i/o functions to read from our binary data
	png_set_read_fn( png_ptr, this, &TLFilePng::ReadData );

	if (libpng::setjmp(png_jmpbuf(png_ptr)))
    {
		TLDebug_Break("Error setting libpng jump");
        libpng::png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
        return SyncFalse;
    }

	//	already read header so tell libpng how much we've read
	png_set_sig_bytes(png_ptr, Header.GetSize() );

	//	wrapper for the PNG reading as it's stupid long jump system won't allow us to use returns properly
	Bool ReadSuccess = FALSE;
	TLFilePng::PngReadHeader( png_ptr, info_ptr, ReadSuccess );
	if ( !ReadSuccess )
	{
		TLDebug_Break("error reading PNG header");
		libpng::png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
		return SyncFalse;
	}

	if (libpng::setjmp(png_jmpbuf(png_ptr)))
    {
		TLDebug_Break("Error setting libpng jump");
        libpng::png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
        return SyncFalse;
    }

	//	palette -> rgb
	if (info_ptr->color_type == PNG_COLOR_TYPE_PALETTE)
		png_set_palette_to_rgb(png_ptr);

	//	less than 8 bit grey scale -> rgb
	if (info_ptr->color_type == PNG_COLOR_TYPE_GRAY && info_ptr->bit_depth < 8) 
		png_set_gray_1_2_4_to_8(png_ptr);

	//	something about alpha channel
//	if (png_get_valid(png_ptr, info_ptr,PNG_INFO_tRNS)) 
//		png_set_tRNS_to_alpha(png_ptr);

	//	dont combine alpha with main image
//	if (info_ptr->color_type & PNG_COLOR_MASK_ALPHA)
 //       png_set_strip_alpha(png_ptr);

	//	expand small bit depths to 8bit
	if (info_ptr->bit_depth < 8)
        png_set_packing(png_ptr);

	//	reduce 16 bit colours to 8bit
    if (info_ptr->bit_depth == 16)
        png_set_strip_16(png_ptr);

	//	grey scale -> rgb
	if (info_ptr->color_type == PNG_COLOR_TYPE_GRAY || info_ptr->color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
		png_set_gray_to_rgb(png_ptr);

	//	endian swap
	//if (bit_depth == 16)
	//	png_set_swap(png_ptr);

	//	create new texture asset
	TPtr<TLAsset::TTexture> pTexture = new TLAsset::TTexture( GetFileRef() );
	if ( !pTexture )
	{
		TLDebug_Break("failed to alloc texture asset");
		libpng::png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
		return SyncFalse;
	}

	//	check to see if it has an alpha channel or not
	Bool HasAlphaChannel = (info_ptr->color_type & PNG_COLOR_MASK_ALPHA) != 0x0;

	//	setup texture
	u16 width = (u16)info_ptr->width;
	u16 height = (u16)info_ptr->height;
	if ( !pTexture->SetSize( Type2<u16>( width, height ), HasAlphaChannel ) )
	{
		TLDebug_Break("failed to set texture size - non-square?");
		libpng::png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
		return SyncFalse;
	}

	//	alloc data for image
	TArray<u8*> RowData;
	u32 RowDataSize = 0;
	u32 TotalDataSize = 0;
	TLFilePng::AllocRowData( RowData, info_ptr, RowDataSize, TotalDataSize );

	//	assume sizes should match...
	if ( TotalDataSize != pTexture->GetPixelData().GetSize() )
	{
		if ( !TLDebug_Break("expected data sizes to match...") )
		{
			libpng::png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
			return SyncFalse;
		}
	}

	ReadSuccess = FALSE;
	TLFilePng::PngReadImage( png_ptr, RowData, ReadSuccess );
	if ( !ReadSuccess )
	{
		TLDebug_Break("error reading PNG image data");
		TLFilePng::DeleteRowData( RowData );
		libpng::png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
		return SyncFalse;
	}

	ReadSuccess = FALSE;
	TLFilePng::PngReadEnd(png_ptr, ReadSuccess );
	if ( !ReadSuccess )
	{
		TLDebug_Break("error reading PNG end data");
		TLFilePng::DeleteRowData( RowData );
		libpng::png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
		return SyncFalse;
	}

	//	put row data into our texture data
	TArray<u8>& PixelDataArray = pTexture->GetPixelData().GetDataArray();
	PixelDataArray.SetSize(0);
	pTexture->GetPixelData().GetDataArray().SetSize(0);
	for ( u32 r=0;	r<RowData.GetSize();	r++ )
	{
		//	add data to texture data
		PixelDataArray.Add( RowData[r], RowDataSize );
	}
	
	//	assign texture to the output asset ptr to complete export
	pTexture->SetLoadingState( TLAsset::LoadingState_Loaded );
	pAsset = pTexture;

	//	cleanup
	TLFilePng::DeleteRowData( RowData );
	libpng::png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);

	return SyncTrue;	
}	

