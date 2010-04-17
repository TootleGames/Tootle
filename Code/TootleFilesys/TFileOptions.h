
#pragma once

#include "TFileXML.h"

#include <TootleAsset/TOptions.h>

namespace TLFileSys
{
	class TFileOptions;
}

//---------------------------------------------------------
//	Options file - xml file that has markup for various user-speified options
//---------------------------------------------------------
class TLFileSys::TFileOptions : public TLFileSys::TFileXml
{
public:
	TFileOptions(TRefRef FileRef,TRefRef FileTypeRef);


	virtual SyncBool	ExportAsset(TPtr<TLAsset::TAsset>& pAsset,TRefRef ExportAssetType);

	virtual void		GetSupportedExportAssetTypes(TArray<TRef>& SupportedTypes) const	{	SupportedTypes << TRef_Static(O,p,t,i,o);	}

	virtual TLAsset::TOptions* CreateAsset() { 	return new TLAsset::TOptions( GetFileRef()); }

protected:
	SyncBool			ImportRoot(TPtr<TXmlTag>& pTag,TLAsset::TOptions& Options);

};
