/*------------------------------------------------------
	
	XML file - parses on import to an XML tree type

-------------------------------------------------------*/
#pragma once

#include <TootleCore/TString.h>
#include "TFile.h"
#include "TXml.h"
#include <TootleAsset/TAsset.h>


namespace TLFileSys
{
	class TFileXml;	
};



//---------------------------------------------------------
//	
//---------------------------------------------------------
class TLFileSys::TFileXml : public TLFileSys::TFile
{
public:
	TFileXml(TRefRef FileRef,TRefRef FileTypeRef);

	SyncBool			Import();					//	turn this TFile into XML
//	SyncBool			Export();					//	export XML to binary data

//	virtual SyncBool	Export(TPtr<TFileAsset>& pAssetFile);	//	turn XML into a more binary format. XML tags become ref's, data and properties become child binarytree's

protected:
	TXml				m_XmlData;	//	parsed xml
};


