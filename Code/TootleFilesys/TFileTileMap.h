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
	
	virtual TRef		GetFileExportAssetType() const						{	return TRef_Static(T,i,l,e,M);	}
	virtual SyncBool	ExportAsset(TPtr<TLAsset::TAsset>& pAsset,Bool& Supported);

protected:
	SyncBool			ImportTileMap(TXmlTag& Tag,TLAsset::TTileMap& TileMap);
	SyncBool			ImportTile(TXmlTag& Tag,TLAsset::TTileMap& TileMap);
};


