#include "TTexture.h"





TLAsset::TTexture::TTexture(TRefRef AssetRef) :
	TAsset		( GetAssetType_Static(), AssetRef ),
	m_Size		( 0, 0 ),
	m_HasAlpha	( FALSE ),
	m_HasChanged(FALSE)
{
}


//-------------------------------------------------------
//	set a new size, re-alloc data etc - fails if size is incompatbile
//-------------------------------------------------------
Bool TLAsset::TTexture::SetSize(const Type2<u16>& NewSize,Bool EnableAlpha, Bool bAutoCreateIfNonsquare)
{
	// [09/03/10] DB - Added non-const local rather than altering the function for now
	Type2<u16> FinalSize = NewSize;
		
	//	check is a square size (opengl ES can't do rectangluar textures)
	if ( FinalSize.x != FinalSize.y )
	{
		if(bAutoCreateIfNonsquare)
		{
			u16 WidthPower2 = TLMaths::GetNextPowerOf2(NewSize.x);
			u16 HeightPower2 = TLMaths::GetNextPowerOf2(NewSize.y);
			
			if(WidthPower2 > HeightPower2)
			{
				FinalSize.x = WidthPower2;
				FinalSize.y = WidthPower2;
			}
			else
			{
				FinalSize.x = HeightPower2;
				FinalSize.y = HeightPower2;
			}
		}
		else 
		{
			TLDebug_Break("Texture size must be square; eg. 256x256, NOT 256x128");
			return FALSE;
		}
	}

	// sizes are the same, so can just check x or y is power of 2 and less than max texture size
	// TODO: max texture size may be platform specific so may need a platform specific function
	// to get this information rather than assuming a set size
	//	gr: for portability, we have a max of 1024 which is the iphone's limit.
	if ( FinalSize.x > 1024 || !TLMaths::IsPowerOf2(FinalSize.x) )
	{
		TLDebug_Break("Texture size MUST be power of 2; 2,4,8...1024 etc and MUST be 1024 or less");
		return FALSE;
	}
	
	// Already the same size?  No need to change size
	if(FinalSize == m_Size)
		return TRUE;

	//	gr: todo; if data already exists, crop/expand image

	//	set alpha
	m_HasAlpha = EnableAlpha;

	//	alloc data
	m_Size = FinalSize;
	u32 DataSize = GetPixelDataSize() * ( GetWidth() * GetHeight() );
	m_TextureData.SetSize( DataSize );

	//	initialise to all-white-full-alpha (easiest way :)
	m_TextureData.GetDataArray().SetAll( 255 );

	SetHasChanged(TRUE);
	
	return TRUE;
}


Bool TLAsset::TTexture::SetTextureData(const Type2<u16>& ImageSize, TBinary& ImageData)
{
	// Image data bigger than the texture data?
	if(ImageSize.x > m_Size.x || ImageSize.y > m_Size.y)
		return FALSE;
	
	u32 uPixelOffset = 0;
	u32 uImageOffset = 0;

	u32 uSize = ImageSize.x * sizeof(u32);

	// Copy data in rows.  By doing this we can 'fit' an image into a larger texture
	// as we need to use square textures but may need to use non-square so to get around that
	// put the non-square image into a square texture and utilise the UV mapping to get the right
	// effect.  Very wastefull unfortunately so where possible use square textures.
	for(u32 uIndex = 0; uIndex < ImageSize.y; uIndex++)
	{
		u8* pPixelData = GetPixelData(uPixelOffset);
		u8* pImageData = ImageData.GetData(uImageOffset);
		
		// Copy image data into the texture
		TLMemory::CopyData(pPixelData, pImageData, uSize);
									 
		uPixelOffset += m_Size.x;
		uImageOffset += ImageSize.x * 4;
	}

	GetPixelData().ResetReadPos();

	SetHasChanged(TRUE);
	
	return TRUE;
}

//-------------------------------------------------------
//	load asset data out binary data
//-------------------------------------------------------
SyncBool TLAsset::TTexture::ImportData(TBinaryTree& Data)
{
	if ( !Data.ImportData("Alpha", m_HasAlpha ) )	
		return SyncFalse;
	if ( !Data.ImportData("Width", m_Size.x ) )	
		return SyncFalse;
	if ( !Data.ImportData("Height", m_Size.y ) )	
		return SyncFalse;
	
	if ( !Data.ImportArrays("TexData", m_TextureData.GetDataArray() ) )	
		return SyncFalse;
	
	SetHasChanged(TRUE);
	
	m_TextureData.ResetReadPos();

	return SyncTrue;
}

//-------------------------------------------------------
//	save asset data to binary data
//-------------------------------------------------------
SyncBool TLAsset::TTexture::ExportData(TBinaryTree& Data)
{
	Data.ExportData("Alpha", m_HasAlpha );
	Data.ExportData("Width", m_Size.x );
	Data.ExportData("Height", m_Size.y );
	Data.ExportArray("TexData", m_TextureData.GetDataArray() );

	return SyncTrue;
}


//-------------------------------------------------------
//	insert texture data into the texture at a certain point
//-------------------------------------------------------
bool TLAsset::TTexture::PasteTextureData(const Type2<u16>& PasteAt,const TArray<TColour24>& TextureData,const Type2<u16>& DataSize)
{
	//	check this fits in
	Type2<u16> BottomRight = PasteAt + DataSize;
	if ( BottomRight.x >= GetWidth() || BottomRight.y >= GetHeight() )
	{
		TTempString Debug_String;
		Debug_String << "Data being pasted into texture at (" << PasteAt.x << "," << PasteAt.y << " -> " << BottomRight.x << "," << BottomRight.y << ")"
					<< " does not fit into texture sized (" << GetWidth() << "," << GetHeight() << ")";
		TLDebug_Break( Debug_String );
		return false;
	}

	//	make sure we're pasting into the right format
	//	todo: convert as we paste
	TColour24* pTextureRowData = GetPixelData24At( PasteAt );
	if ( !pTextureRowData )
	{
		TLDebug_Break("Missing colour data in texture... wrong format? (24bit into 32bit texture?");
		return false;
	}

	//	write data in, row-by-row
	for ( u32 y=0;	y<DataSize.y;	y++ )
	{
		pTextureRowData = GetPixelData24At( PasteAt.x, PasteAt.y + y );
		const TColour24* pPasteRowData = &TextureData[ 0 + (y * DataSize.y) ];
		TLMemory::CopyData( pTextureRowData, pPasteRowData, DataSize.x );
	}

	return true;
}


//-------------------------------------------------------
//	insert texture data into the texture at a certain point
//-------------------------------------------------------
bool TLAsset::TTexture::PasteTextureData(const Type2<u16>& PasteAt,const TArray<TColour32>& TextureData,const Type2<u16>& DataSize)
{
	//	check this fits in
	Type2<u16> BottomRight = PasteAt + DataSize;
	if ( BottomRight.x >= GetWidth() || BottomRight.y >= GetHeight() )
	{
		TTempString Debug_String;
		Debug_String << "Data being pasted into texture at (" << PasteAt.x << "," << PasteAt.y << " -> " << BottomRight.x << "," << BottomRight.y << ")"
					<< " does not fit into texture sized (" << GetWidth() << "," << GetHeight() << ")";
		TLDebug_Break( Debug_String );
		return false;
	}

	//	make sure we're pasting into the right format
	//	todo: convert as we paste
	TColour32* pTextureRowData = GetPixelData32At( PasteAt );
	if ( !pTextureRowData )
	{
		TLDebug_Break("Missing colour data in texture... wrong format? (24bit into 32bit texture?");
		return false;
	}

	//	write data in, row-by-row
	for ( u32 y=0;	y<DataSize.y;	y++ )
	{
		pTextureRowData = GetPixelData32At( PasteAt.x, PasteAt.y + y );
		const TColour32* pPasteRowData = &TextureData[ 0 + (y * DataSize.y) ];
		TLMemory::CopyData( pTextureRowData, pPasteRowData, DataSize.x );
	}

	return true;
}
