/*------------------------------------------------------
	
	Scheme markup (xml)

-------------------------------------------------------*/
#pragma once

#include "TFile.h"
#include "TFileXml.h"

#include <TootleAsset/TScheme.h>

namespace TLFileSys
{
	class TFileScheme;
}



//---------------------------------------------------------
//	SVG xml file that has markup for nodes
//---------------------------------------------------------
class TLFileSys::TFileScheme : public TLFileSys::TFileXml
{
public:
	TFileScheme(TRefRef FileRef,TRefRef FileTypeRef);

	virtual SyncBool	ExportAsset(TPtr<TLAsset::TAsset>& pAsset,TRefRef ExportAssetType);
	virtual void		GetSupportedExportAssetTypes(TArray<TRef>& SupportedTypes) const	{	SupportedTypes << TRef_Static(S,c,h,e,m);	}

	virtual TLAsset::TScheme* CreateAsset() { 	return new TLAsset::TScheme( GetFileRef()); }

protected:
	SyncBool			ImportScheme(TPtr<TXmlTag>& pTag,TPtr<TLAsset::TScheme>& pScheme);
	SyncBool			ImportGraph(TPtr<TXmlTag>& pTag,TPtr<TLAsset::TScheme>& pScheme);
	SyncBool			ImportNode(TPtr<TXmlTag>& pTag,TRefRef GraphRef,TPtr<TLAsset::TScheme>& pScheme,TPtr<TLAsset::TSchemeNode> pParentNode);
	SyncBool			ImportNode_Data(TPtr<TXmlTag>& pTag,TPtr<TLAsset::TSchemeNode>& pNode);
};

