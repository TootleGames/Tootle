/*------------------------------------------------------
	
	Scheme markup (xml)

-------------------------------------------------------*/
#pragma once

#include "TFile.h"
#include "TFileXml.h"


namespace TLAsset
{
	class TScheme;
	class TSchemeNode;
}

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

	virtual SyncBool	ExportAsset(TPtr<TLAsset::TAsset>& pAsset,Bool& Supported);			//	import the XML and convert from SVG to mesh

protected:
	SyncBool			ImportScheme(TPtr<TXmlTag>& pTag,TPtr<TLAsset::TScheme>& pScheme);
	SyncBool			ImportGraph(TPtr<TXmlTag>& pTag,TPtr<TLAsset::TScheme>& pScheme);
	SyncBool			ImportNode(TPtr<TXmlTag>& pTag,TRefRef GraphRef,TPtr<TLAsset::TScheme>& pScheme,TPtr<TLAsset::TSchemeNode> pParentNode);
	SyncBool			ImportNode_Data(TPtr<TXmlTag>& pTag,TPtr<TLAsset::TSchemeNode>& pNode);
};

