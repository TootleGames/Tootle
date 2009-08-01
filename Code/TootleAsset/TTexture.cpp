#include "TTexture.h"





TLAsset::TTexture::TTexture(TRefRef AssetRef) :
	TAsset		( GetAssetType_Static(), AssetRef ),
	m_Size		( 0, 0 ),
	m_HasAlpha	( FALSE )
{
}


//-------------------------------------------------------
//	set a new size, re-alloc data etc - fails if size is incompatbile
//-------------------------------------------------------
Bool TLAsset::TTexture::SetSize(const Type2<u16>& NewSize,Bool EnableAlpha)
{
	//	check is a square size (opengl ES can't do rectangluar textures)
	if ( NewSize.x != NewSize.y )
	{
		TLDebug_Break("Texture size must be square; 256x256, NOT 256x128");
		return FALSE;
	}

	//	sizes are the same, so can just check x or y is power of 2
	if (	NewSize.x != 2 && 
			NewSize.x != 4 && 
			NewSize.x != 8 && 
			NewSize.x != 16 && 
			NewSize.x != 32 && 
			NewSize.x != 64 && 
			NewSize.x != 128 && 
			NewSize.x != 256 && 
			NewSize.x != 512 && 
			NewSize.x != 1024 )
	{
		TLDebug_Break("Texture size must be power of 2; 2,4,8...1024 etc");
		return FALSE;
	}

	//	gr: todo; if data already exists, crop/expand image

	//	set alpha
	m_HasAlpha = EnableAlpha;

	//	alloc data
	m_Size = NewSize;
	u32 DataSize = GetPixelDataSize() * ( GetWidth() * GetHeight() );
	m_TextureData.SetSize( DataSize );

	//	initialise to all-white-full-alpha (easiest way :)
	m_TextureData.GetDataArray().SetAll( 255 );

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




