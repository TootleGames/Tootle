/*------------------------------------------------------
 
	PNG file loader that uses libpng
	
	http://www.libpng.org/pub/png/libpng-1.2.5-manual.html#section-3.1


 -------------------------------------------------------*/
#pragma once

#include "TFile.h"
#include <TootleAsset/TAsset.h>


namespace TLFileSys
{
	class TFilePng;	
};



//---------------------------------------------------------
//	
//---------------------------------------------------------
class TLFileSys::TFilePng : public TLFileSys::TFile
{
public:
	TFilePng(TRefRef FileRef,TRefRef FileTypeRef);
	
	virtual TRef				GetFileExportAssetType() const										{	return TRef_Static(T,e,x,t,u);	}
	virtual SyncBool			ExportAsset(TPtr<TLAsset::TAsset>& pAsset,Bool& Supported);

};


