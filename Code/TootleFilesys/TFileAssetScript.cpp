/*
 *  TFileAssetScript.cpp
 *  TootleFileSys
 *
 *  Created by Duane Bradbury on 15/03/2009.
 *  Copyright 2009 Tootle. All rights reserved.
 *
 */

#include "TFileAssetScript.h"
#include <TootleAsset/TAssetScript.h>
#include "TLFile.h"



TLFileSys::TFileAssetScript::TFileAssetScript(TRefRef FileRef,TRefRef FileTypeRef) :
TFileXml			( FileRef, FileTypeRef )
{
}



//--------------------------------------------------------
//	import the XML and convert from SVG to mesh
//--------------------------------------------------------
SyncBool TLFileSys::TFileAssetScript::ExportAsset(TPtr<TLAsset::TAsset>& pAsset,Bool& Supported)
{
	if ( pAsset )
	{
		TLDebug_Break("Async export not supported yet. asset should be NULL");
		return SyncFalse;
	}

	Supported = TRUE;

	//	import xml
	SyncBool ImportResult = TFileXml::Import();
	if ( ImportResult != SyncTrue )
		return ImportResult;

	//	get the root tag
	TPtr<TXmlTag> pTasTag = m_XmlData.GetChild("assetscript");

	//	malformed AssetScript
	if ( !pTasTag )
	{
		TLDebug_Print("TAS file missing root <AssetScript> tag");
		return SyncFalse;
	}

	//	do specific importing
	TPtr<TLAsset::TAsset> pNewAsset = new TLAsset::TAssetScript( GetFileRef() );

	ImportResult = ImportAssetScript( pNewAsset, pTasTag );

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
SyncBool TLFileSys::TFileAssetScript::ImportAssetScript(TPtr<TLAsset::TAssetScript> pAssetScript, TPtr<TXmlTag>& pTag)
{
	/*
	<assetscript>
		<keyframe time="0.0">
			<Node NodeRef="rarm1">		
				<Data DataRef="Rotate" Interp="TRUE" Method="Linear"><float3>0,0,0</float3></Data>
			</Node>
		</keyframe>
	</assetscript>
	*/
	//	deal with child tags
	for ( u32 c=0;	c<pTag->GetChildren().GetSize();	c++ )
	{
		TPtr<TXmlTag>& pChildTag = pTag->GetChildren().ElementAt(c);

		SyncBool TagImportResult = SyncFalse;
		if ( pChildTag->GetTagName() == "keyframe" )
		{
			TagImportResult = ImportAssetScript_ImportKeyframeTag( pAssetScript, pChildTag );
		}
		else
		{
			TLDebug_Break("Unsupported tag in asset script import");
		}

		//	failed
		if ( TagImportResult == SyncFalse )
			return SyncFalse;

		//	async
		if ( TagImportResult == SyncWait )
		{
			TLDebug_Break("todo: async TAS import");
			return SyncFalse;
		}
	}
	
	return SyncTrue;
}


//--------------------------------------------------------
//	generate mesh TAM tag
//--------------------------------------------------------
SyncBool TLFileSys::TFileAssetScript::ImportAssetScript_ImportKeyframeTag(TPtr<TLAsset::TAssetScript>& pAssetScript,TPtr<TXmlTag>& pImportTag)
{
	/*
	<keyframe time="0.0">
		<Node NodeRef="rarm1">		
			<Data DataRef="Rotate" Interp="TRUE" Method="Linear"><float3>0,0,0</float3></Data>
		</Node>
	</keyframe>
	*/
	float keyframetime;
	const TString* pImportkeyframeString = pImportTag->GetProperty("Time");
	if ( pImportkeyframeString )
	{
		if(!pImportkeyframeString->GetFloat(keyframetime))
		{
			return SyncFalse;
		}
	}
	else
	{
		TLDebug_Break("gr: use of uninitialised keyframe time! - keyframe property is required I assume?");
		return SyncFalse;
	}

	// Create a new keyframe
	TLAsset::TKeyframe* pKeyframe = pAssetScript->AddKeyframe(keyframetime);

	if(!pKeyframe)
	{
		TLDebug_Print("Unable to create keyframe for TAS asset");
		return SyncFalse;
	}

	//	find out what we need to do
	for ( u32 c=0;	c<pImportTag->GetChildren().GetSize();	c++ )
	{
		TPtr<TXmlTag>& pChildTag = pImportTag->GetChildren().ElementAt(c);
		
		SyncBool TagImportResult = SyncFalse;

		// Deal with "node" tags
		if ( pChildTag->GetTagName() == "node" )
		{
			TagImportResult = ImportAssetScript_ImportNodeTag( pAssetScript, pKeyframe, pChildTag );
		}

		//	failed
		if ( TagImportResult == SyncFalse )
			return SyncFalse;

		//	async
		if ( TagImportResult == SyncWait )
		{
			TLDebug_Break("todo: async TAS import");
			return SyncFalse;
		}
	}
	
	return SyncTrue;
}

SyncBool TLFileSys::TFileAssetScript::ImportAssetScript_ImportNodeTag(TPtr<TLAsset::TAssetScript>& pAssetScript, TLAsset::TKeyframe* pKeyframe, TPtr<TXmlTag>& pImportTag)
{
	/*
	<Node NodeRef="rarm1">		
		<Data DataRef="Rotate" Interp="TRUE" Method="Linear"><float3>0,0,0</float3></Data>
	</Node>
	*/

	TRef NodeRef, NodeGraphRef;
	const TString* pImportkeyframeString = pImportTag->GetProperty("NodeRef");
	if ( pImportkeyframeString )
		NodeRef.Set(*pImportkeyframeString);

	if(!NodeRef.IsValid())
	{
		TLDebug_Print("Failed to get valid node ref from TAS file");
		return SyncFalse;
	}

	pImportkeyframeString = pImportTag->GetProperty("NodeGraphRef");
	if ( pImportkeyframeString )
		NodeGraphRef.Set(*pImportkeyframeString);

	if(!NodeGraphRef.IsValid())
	{
		TLDebug_Print("Failed to get valid node graph ref from TAS file");
		return SyncFalse;
	}


	// Create the asset script command list object
	TPtr<TLAsset::TAssetScriptCommandList> pScriptCommandList = new TLAsset::TAssetScriptCommandList(NodeRef, NodeGraphRef);

	if(!pScriptCommandList || (pKeyframe->Add(pScriptCommandList) == -1))
	{
		TLDebug_Print("Failed to add new script command list");
		return SyncFalse;
	}


	//	find out what we need to do
	for ( u32 c=0;	c<pImportTag->GetChildren().GetSize();	c++ )
	{
		TPtr<TXmlTag>& pChildTag = pImportTag->GetChildren().ElementAt(c);
		
		SyncBool TagImportResult = ImportAssetScript_ImportCommandTag( pAssetScript, pScriptCommandList, pChildTag);

		//	failed
		if ( TagImportResult == SyncFalse )
			return SyncFalse;

		//	async
		if ( TagImportResult == SyncWait )
		{
			TLDebug_Break("todo: async TAS import");
			return SyncFalse;
		}
	}

	return SyncTrue;

}


SyncBool TLFileSys::TFileAssetScript::ImportAssetScript_ImportCommandTag(TPtr<TLAsset::TAssetScript>& pAssetScript, TPtr<TLAsset::TAssetScriptCommandList>& pScriptCommandList, TPtr<TXmlTag>& pImportTag)
{
	TLAsset::TAssetScriptCommand* pCommand = NULL;

	// Get command and all data properties
	for ( u32 c=0;	c<pImportTag->GetPropertyCount();	c++ )
	{
		const TXmlTag::TProperty& Property = pImportTag->GetPropertyAt(c);

		const TStringLowercase<TTempString>& PropertyName = Property.m_Key;
		const TString& PropertyData = Property.m_Item;

		if ( PropertyName == "DataRef" )
		{
			pCommand = pScriptCommandList->AddCommand(PropertyData);

			if(!pCommand)
			{
				TLDebug_Print("Failed to create command for TAS file");
				return SyncFalse;
			}
		}
		else if(PropertyName == "InterpMethod")
		{
			if(PropertyData == "Linear")
			{
				pCommand->SetInterpMethod(TLAsset::TAssetScriptCommand::Linear);
			}
			else if(PropertyData == "SLERP")
			{
				pCommand->SetInterpMethod(TLAsset::TAssetScriptCommand::SLERP);
			}
		}

		// Now get any extra data once we reach the end of the tag info
		if((pCommand != NULL) && (c == (pImportTag->GetPropertyCount() - 1)))
		{
			TPtr<TXmlTag>& pChildTag = pImportTag->GetChildren().ElementAt(0);

			if(pChildTag)
			{
				TRef DataTypeRef = TLFile::GetDataTypeFromString( pChildTag->GetTagName() );

				//	update type of data
				pCommand->SetDataTypeHint( DataTypeRef );

				SyncBool TagImportResult = TLFile::ImportBinaryData( pChildTag, *pCommand, DataTypeRef );


				if(TagImportResult != SyncTrue)
				{
					// Failed to copy the data form the XML file
					TLDebug_Print("Failed to get command data from TAS file");
					return SyncFalse;
				}
			}
		}
	}


	
	return SyncTrue;
}


