/*------------------------------------------------------

	Texture asset

-------------------------------------------------------*/
#pragma once

#include "TLAsset.h"
#include "TAsset.h"
#include <TootleMaths/TBox.h>
#include <TootleMaths/TSphere.h>
#include <TootleMaths/TLine.h>
#include <TootleCore/TBinary.h>
#include <TootleCore/TColour.h>


namespace TLAsset
{
	class TTexture;
};


//--------------------------------------------------
//	network of nodes for a path
//--------------------------------------------------
class TLAsset::TTexture : public TLAsset::TAsset
{
public:
	TTexture(TRefRef AssetRef);

	static TRef						GetAssetType_Static()					{	return TRef_Static(T,e,x,t,u);	}

	Bool							SetSize(const Type2<u16>& NewSize,Bool EnableAlpha, Bool AutoCreateIfNonsquare=FALSE);	//	set a new size, re-alloc data etc - fails if size is incompatbile
	FORCEINLINE const Type2<u16>&	GetSize() const							{	return m_Size;	}
	FORCEINLINE const u16&			GetWidth() const						{	return m_Size.x;	}
	FORCEINLINE const u16&			GetHeight() const						{	return m_Size.y;	}
	FORCEINLINE Bool				HasAlphaChannel() const					{	return m_HasAlpha;	}

	FORCEINLINE Bool				IsMinFilterLinear() const				{	return TRUE;	}
	FORCEINLINE Bool				IsMagFilterLinear() const				{	return TRUE;	}
	FORCEINLINE Bool				IsMipMapEnabled() const					{	return FALSE;	}
	FORCEINLINE Bool				IsClampEnabled() const					{	return true;	}

	FORCEINLINE TColour32*			GetPixelData32(u32 PixelIndex=0)				{	return HasAlphaChannel() ? (TColour32*)GetPixelData( PixelIndex ) : NULL;	}
	FORCEINLINE TColour32*			GetPixelData32At(u32 PixelX,u32 PixelY)			{	return GetPixelData32( GetPixelIndex( PixelX, PixelY ) );	}
	FORCEINLINE TColour32*			GetPixelData32At(const Type2<u16>& PixelXY)		{	return GetPixelData32( GetPixelIndex( PixelXY.x, PixelXY.y ) );	}
	FORCEINLINE TColour24*			GetPixelData24(u32 PixelIndex=0)				{	return HasAlphaChannel() ? NULL : (TColour24*)GetPixelData( PixelIndex );	}
	FORCEINLINE TColour24*			GetPixelData24At(u32 PixelX,u32 PixelY)			{	return GetPixelData24( GetPixelIndex( PixelX, PixelY ) );	}
	FORCEINLINE TColour24*			GetPixelData24At(const Type2<u16>& PixelXY)		{	return GetPixelData24( GetPixelIndex( PixelXY.x, PixelXY.y ) );	}

	FORCEINLINE u32					GetPixelDataSize() const					{	return HasAlphaChannel() ? 4 : 3;	}
	FORCEINLINE TBinary&			GetPixelData()								{	return m_TextureData;	}
	FORCEINLINE const TBinary&		GetPixelData() const						{	return m_TextureData;	}
	FORCEINLINE u8*					GetPixelData(u32 PixelIndex) 				{	return m_TextureData.GetData( PixelIndex * GetPixelDataSize() );	}
	FORCEINLINE const u8*			GetPixelData(u32 PixelIndex) const			{	return m_TextureData.GetData( PixelIndex * GetPixelDataSize() );	}
	FORCEINLINE u8*					GetPixelData(u32 PixelX,u32 PixelY) 		{	return GetPixelData( GetPixelIndex( PixelX, PixelY ) );	}
	FORCEINLINE const u8*			GetPixelData(u32 PixelX,u32 PixelY) const	{	return GetPixelData( GetPixelIndex( PixelX, PixelY ) );	}
	FORCEINLINE u32					GetPixelIndex(u32 PixelX,u32 PixelY) const	{	return (PixelY * GetWidth() ) + PixelX;	}

	Bool							SetTextureData(const Type2<u16>& ImageSize, TBinary& ImageData);

	//	insert texture data into the texture at a certain point
	//	pass a Box2D to get the UV's of the pasted region to put into an atlas or similar
	bool							PasteTextureData(const Type2<u16>& PasteAt,const TArray<TColour24>& TextureData,const Type2<u16>& DataSize,TLMaths::TBox2D* pGlyphBox=NULL);
	bool							PasteTextureData(const Type2<u16>& PasteAt,const TArray<TColour32>& TextureData,const Type2<u16>& DataSize,TLMaths::TBox2D* pGlyphBox=NULL);

protected:
	virtual SyncBool				ImportData(TBinaryTree& Data);		//	load asset data out binary data
	virtual SyncBool				ExportData(TBinaryTree& Data);		//	save asset data to binary data

protected:
	TBinary						m_TextureData;	//	binary texture data
	Type2<u16>					m_Size;
	Bool						m_HasAlpha;		//	data has/hasnt got an alpha channel
};


