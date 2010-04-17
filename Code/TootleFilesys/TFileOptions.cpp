#include "TFileOptions.h"
#include <TootleAsset/TOptions.h>
#include "TLFile.h"



TLFileSys::TFileOptions::TFileOptions(TRefRef FileRef,TRefRef FileTypeRef) :
	TFileXml				( FileRef, FileTypeRef )
{
}



//--------------------------------------------------------
//	import the XML
//--------------------------------------------------------
SyncBool TLFileSys::TFileOptions::ExportAsset(TPtr<TLAsset::TAsset>& pAsset,TRefRef ExportAssetType)
{
	if ( pAsset )
	{
		TLDebug_Break("Async export not supported yet. asset should be NULL");
		return SyncFalse;
	}

	//	import xml
	SyncBool ImportResult = TFileXml::Import();
	if ( ImportResult != SyncTrue )
	{
		if ( ImportResult == SyncFalse )
		{
			TLDebug_Break("Failed to parse xml file");
		}
		return ImportResult;
	}

	//	get the root tag
	TPtr<TXmlTag> pRootTag = m_XmlData.GetChild("Options");

	//	malformed XML
	if ( !pRootTag )
	{
		TLDebug_Print("Scheme file missing root <Options> tag");
		return SyncFalse;
	}

	//	make up new storage asset type
	TPtr<TLAsset::TOptions> pNewAsset = CreateAsset( );

	ImportResult = ImportRoot( pRootTag, *pNewAsset );

	//	failed to import
	if ( ImportResult != SyncTrue )
	{
		return SyncFalse;
	}

	//	assign resulting asset
	pAsset = pNewAsset;

	return SyncTrue;
}




//--------------------------------------------------------
//	
//--------------------------------------------------------
SyncBool TLFileSys::TFileOptions::ImportRoot(TPtr<TXmlTag>& pTag,TLAsset::TOptions& Options)
{
	/*
	<Options>

		// Game specific options
		<Data DataRef="Game">
		</Data>

		// Audio options
		<Data DataRef="Audio">

			<Data DataRef="Enable"><bool>TRUE</bool></Data>

			<Data DataRef="Volume">
				<Data DataRef="Effects"><flt>1.0f</flt></Data>
				<Data DataRef="Music"><flt>1.0f</flt></Data>
			</Data>

		</Data>

	</Options>
	*/
	
	//	deal with child tags
	for ( u32 c=0;	c<pTag->GetChildren().GetSize();	c++ )
	{
		TPtr<TXmlTag>& pChildTag = pTag->GetChildren().ElementAt(c);

		SyncBool TagImportResult = SyncFalse;
		if ( pChildTag->GetTagName() == "Data" )
		{
			//	read-in data and add to the asset's generic data
			if ( TLFile::ParseXMLDataTree( *pChildTag, Options.GetData() ) )
				TagImportResult = SyncTrue;
			else
				TagImportResult = SyncFalse;
		}
		else
		{
			TLDebug_Break("Unsupported tag in Options import");
		}

		//	failed
		if ( TagImportResult == SyncFalse )
			return SyncFalse;

		//	async
		if ( TagImportResult == SyncWait )
		{
			TLDebug_Break("todo: async Options import");
			return SyncFalse;
		}
	}
	
	return SyncTrue;
}
