/*------------------------------------------------------
 
	proprietry tilemap format

 -------------------------------------------------------*/
#pragma once

#include "TFile.h"
#include "TFileXml.h"


namespace TLAsset
{
	class TTileMap;
}


namespace TLFileSys
{
	class TFileTileMap;	
};



//---------------------------------------------------------
//	
//---------------------------------------------------------
class TLFileSys::TFileTileMap : public TLFileSys::TFileXml
{
public:
	TFileTileMap(TRefRef FileRef,TRefRef FileTypeRef);
	
	virtual SyncBool	ExportAsset(TPtr<TLAsset::TAsset>& pAsset,TRefRef ExportAssetType);
	virtual void		GetSupportedExportAssetTypes(TArray<TRef>& SupportedTypes) const	{	SupportedTypes << TRef_Static(T,i,l,e,M);	}

protected:
	SyncBool			ImportTileMap(TXmlTag& Tag,TLAsset::TTileMap& TileMap);
	SyncBool			ImportTile(TXmlTag& Tag,TLAsset::TTileMap& TileMap);
};


