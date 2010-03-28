/*------------------------------------------------------
 
	proprietry Texture atlas format. I didn't find a generic 
	format so this seems fine :)

	The xml just defines individual glyphs

 -------------------------------------------------------*/
#pragma once

#include "TFile.h"
#include "TFileXml.h"
#include <TootleAsset/TAtlas.h>



namespace TLFileSys
{
	class TFileAtlas;	
};



//---------------------------------------------------------
//	
//---------------------------------------------------------
class TLFileSys::TFileAtlas : public TLFileSys::TFileXml
{
public:
	TFileAtlas(TRefRef FileRef,TRefRef FileTypeRef);
	
	virtual TRef		GetFileExportAssetType() const										{	return TRef_Static(A,t,l,a,s);	}
	virtual SyncBool	ExportAsset(TPtr<TLAsset::TAsset>& pAsset,Bool& Supported);

protected:
	SyncBool			ImportAtlas(TXmlTag& Tag,TLAsset::TAtlas& Atlas);
	SyncBool			ImportGlyph(TXmlTag& Tag,TLAsset::TAtlas& Atlas);
};


