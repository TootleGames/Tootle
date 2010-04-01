/*------------------------------------------------------
 
 support for Mappy FMP files containing tile map and bitmap data
 
 -------------------------------------------------------*/
#pragma once

#include <TootleCore/TString.h>
#include "TFile.h"
#include <TootleAsset/TAsset.h>


namespace TLFileSys
{
	class TFileMappy;
};

namespace TLAsset
{
	class TTileMap;
	class TTexture;
}

namespace Mappy
{
	class TImportData;
}

//---------------------------------------------------------
//	
//---------------------------------------------------------
class TLFileSys::TFileMappy : public TLFileSys::TFile
{
public:
	TFileMappy(TRefRef FileRef,TRefRef FileTypeRef);
	~TFileMappy();
	
	virtual SyncBool	ExportAsset(TPtr<TLAsset::TAsset>& pAsset,TRefRef ExportAssetType);
	virtual void		GetSupportedExportAssetTypes(TArray<TRef>& SupportedTypes) const	{	SupportedTypes << TRef_Static(T,e,x,t,u) << TRef_Static(T,i,l,e,M) << TRef_Static5(A,t,l,a,s);	}
	
protected:
	Bool	ImportAuthor(TLAsset::TTileMap& TileMap,TBinary& Data);
	Bool	ImportMapHeader(TLAsset::TTileMap& TileMap,TBinary& Data,Mappy::TImportData& ImportData);
	Bool	ImportEditorInfo(TLAsset::TTileMap& TileMap,TBinary& Data);
	Bool	ImportColourPalette(TLAsset::TTileMap& TileMap,TBinary& Data,Mappy::TImportData& ImportData);
	Bool	ImportBlockData(TLAsset::TTileMap& TileMap,TBinary& Data,Mappy::TImportData& ImportData);
	Bool	ImportAnimationData(TLAsset::TTileMap& TileMap,TBinary& Data);
	Bool	ImportGraphics(TLAsset::TTileMap& TileMap,TBinary& Data,Mappy::TImportData& ImportData);
	Bool	ImportLayer(TLAsset::TTileMap& TileMap,TBinary& Data,Mappy::TImportData& ImportData,u8 LayerIndex);



};
