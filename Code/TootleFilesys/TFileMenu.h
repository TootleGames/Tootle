/*------------------------------------------------------
	
	Menu asset markup (xml)

-------------------------------------------------------*/
#pragma once

#include "TFile.h"
#include "TFileXml.h"


namespace TLAsset
{
	class TMenu;
}

namespace TLFileSys
{
	class TFileMenu;
}



//---------------------------------------------------------
//	xml file that has markup for a menu asset
//---------------------------------------------------------
class TLFileSys::TFileMenu : public TLFileSys::TFileXml
{
public:
	TFileMenu(TRefRef FileRef,TRefRef FileTypeRef);

	virtual TRef		GetFileExportAssetType() const										{	return TRef_Static4(M,e,n,u);	}
	virtual SyncBool	ExportAsset(TPtr<TLAsset::TAsset>& pAsset,Bool& Supported);			//	import the XML and convert from SVG to mesh

protected:
	SyncBool			ImportMenu(TPtr<TXmlTag>& pTag,TPtr<TLAsset::TMenu>& pMenu);
	SyncBool			ImportMenuItem(TPtr<TXmlTag>& pTag,TPtr<TLAsset::TMenu>& pMenu);
};

