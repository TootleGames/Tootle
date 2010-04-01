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
//	use this class to parse XML file types by deriving from it, but
//	it can be used just to read XML files anyway (just doesn't support any exporting)
//---------------------------------------------------------
class TLFileSys::TFileXml : public TLFileSys::TFile
{
public:
	TFileXml(TRefRef FileRef,TRefRef FileTypeRef);

	SyncBool			Import();					//	turn this TFile into XML

	TXml&				GetXmlData()				{	return m_XmlData;	}

	//	xml file doesnt support any asset exporting
	virtual SyncBool	ExportAsset(TPtr<TLAsset::TAsset>& pAsset,TRefRef ExportAssetType)	{	return SyncFalse;	}
	virtual void		GetSupportedExportAssetTypes(TArray<TRef>& SupportedTypes) const	{	}

protected:
	TXml				m_XmlData;	//	parsed xml
};


