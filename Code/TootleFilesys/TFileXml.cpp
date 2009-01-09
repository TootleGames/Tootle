#include "TFileXml.h"





//---------------------------------------------------------
//	
//---------------------------------------------------------
TLFileSys::TFileXml::TFileXml(TRefRef FileRef,TRefRef FileTypeRef) :
	TFile	( FileRef, FileTypeRef )
{
}


//---------------------------------------------------------
//	turn this TFile into XML
//---------------------------------------------------------
SyncBool TLFileSys::TFileXml::Import()
{
	//	clean out existing XML
	m_XmlData.Empty();

	//	turn our data into a string
	TBinary& Data = GetData();
	Data.ResetReadPos();
	TArray<u8>& DataArray = Data.GetDataArray();
	TString DataString;
	DataString.Append( (char*)DataArray.GetData(), DataArray.GetSize() );

	//	parse XML string
	SyncBool ImportResult = m_XmlData.Import( DataString );

	if ( ImportResult != SyncTrue )
		return ImportResult;

	//	print out parsed xml
	//m_XmlData.Debug_PrintTree( TFile::GetFilename() );

	return SyncTrue;
}

