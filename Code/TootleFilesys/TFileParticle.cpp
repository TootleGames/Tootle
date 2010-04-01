#include "TFileParticle.h"
#include <TootleAsset/TParticle.h>
#include "TLFile.h"



TLFileSys::TFileParticle::TFileParticle(TRefRef FileRef,TRefRef FileTypeRef) :
	TFileXml				( FileRef, FileTypeRef )
{
}



//--------------------------------------------------------
//	import the XML
//--------------------------------------------------------
SyncBool TLFileSys::TFileParticle::ExportAsset(TPtr<TLAsset::TAsset>& pAsset,TRefRef ExportAssetType)
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
	TPtr<TXmlTag> pRootTag = m_XmlData.GetChild("Particle");

	//	malformed XML
	if ( !pRootTag )
	{
		TLDebug_Print("Scheme file missing root <Particle> tag");
		return SyncFalse;
	}

	//	make up new storage asset type
	TPtr<TLAsset::TParticle> pNewAsset = new TLAsset::TParticle( GetFileRef() );
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
SyncBool TLFileSys::TFileParticle::ImportRoot(TPtr<TXmlTag>& pTag,TLAsset::TParticle& Particle)
{
	/*
	<Particle>
		<Data DataRef=TRef_Static(T,r,a,n,s)><float3>0,40,0</float3></Data>
		<Data DataRef="MeshRef"><TRef>LeftArrow</TRef></Data>
	</Particle>
	*/
	
	//	deal with child tags
	for ( u32 c=0;	c<pTag->GetChildren().GetSize();	c++ )
	{
		TPtr<TXmlTag>& pChildTag = pTag->GetChildren().ElementAt(c);

		SyncBool TagImportResult = SyncFalse;
		if ( pChildTag->GetTagName() == "Data" )
		{
			//	read-in data and add to the asset's generic data
			if ( TLFile::ParseXMLDataTree( *pChildTag, Particle.GetData() ) )
				TagImportResult = SyncTrue;
			else
				TagImportResult = SyncFalse;
		}
		else
		{
			TLDebug_Break("Unsupported tag in Particle import");
		}

		//	failed
		if ( TagImportResult == SyncFalse )
			return SyncFalse;

		//	async
		if ( TagImportResult == SyncWait )
		{
			TLDebug_Break("todo: async Scheme import");
			return SyncFalse;
		}
	}
	
	return SyncTrue;
}









