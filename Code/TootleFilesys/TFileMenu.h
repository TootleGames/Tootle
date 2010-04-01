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

	virtual SyncBool	ExportAsset(TPtr<TLAsset::TAsset>& pAsset,TRefRef ExportAssetType);
	virtual void		GetSupportedExportAssetTypes(TArray<TRef>& SupportedTypes) const	{	SupportedTypes << TRef_Static4(M,e,n,u);	}

protected:
	SyncBool			ImportMenu(TPtr<TXmlTag>& pTag,TPtr<TLAsset::TMenu>& pMenu);
	SyncBool			ImportMenuItem(TPtr<TXmlTag>& pTag,TPtr<TLAsset::TMenu>& pMenu);
};

