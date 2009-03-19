/*
 *  TFileTextDatabase.cpp
 *  TootleFileSys
 *
 *  Created by Duane Bradbury on 15/03/2009.
 *  Copyright 2009 Tootle. All rights reserved.
 *
 */

#include "TFileTextDatabase.h"

#include <TootleAsset/TText.h>

TLFileSys::TFileTextDatabase::TFileTextDatabase(TRefRef FileRef,TRefRef FileTypeRef) :
TFileXml			( FileRef, FileTypeRef )
{
}



//--------------------------------------------------------
//	import the XML and convert from SVG to mesh
//--------------------------------------------------------
SyncBool TLFileSys::TFileTextDatabase::ExportAsset(TPtr<TLAsset::TAsset>& pAsset,Bool& Supported)
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
	TPtr<TXmlTag> pTtdTag = m_XmlData.GetChild("textdatabase");

	//	malformed AssetScript
	if ( !pTtdTag )
	{
		TLDebug_Print("TAS file missing root <textdatabase> tag");
		return SyncFalse;
	}

	//	do specific importing
	TPtr<TLAsset::TAsset> pNewAsset = new TLAsset::TText( GetFileRef() );

	ImportResult = ImportText( pNewAsset, pTtdTag );

	//	failed to import
	if ( ImportResult != SyncTrue )
	{
		return SyncFalse;
	}

	//	assign resulting asset
	pAsset = pNewAsset;

	return SyncTrue;
}


SyncBool TLFileSys::TFileTextDatabase::ImportText(TPtr<TLAsset::TText> pText, TPtr<TXmlTag>& pTag)
{
/*
	<TextDatabase>
		<Text TextRef="VolSFX">
			<eng>Effects Volume</eng>		
			<usa>Effects Volume</usa>		
			<fre>Effects Volume</fre>		
		</Text>
		<Text TextRef="VolMus">
			<eng>Music Volume</eng>		
			<usa>Music Volume</usa>		
			<fre>Music Volume</fre>		
		</Text>
		<Text TextRef="NewGame" />
		<Text TextRef="Options" />
		<Text TextRef="Pause" />
		<Text TextRef="Quit" />
	</TextDatabase>
*/
	//	deal with child tags
	for ( u32 c=0;	c<pTag->GetChildren().GetSize();	c++ )
	{
		TPtr<TXmlTag>& pChildTag = pTag->GetChildren().ElementAt(c);

		SyncBool TagImportResult = SyncFalse;
		if ( pChildTag->GetTagName() == "Text" )
		{
			TRef TextRef;
			const TString* pImportString = pChildTag->GetProperty("TextRef");
			if ( pImportString )
				TextRef = (*pImportString);

			if(!TextRef.IsValid())
			{
				TLDebug_Print("Failed to get valid text ref from TTD file");
				return SyncFalse;
			}

			// Now add the text
			if(pChildTag->GetChildren().GetSize())
			{
				TagImportResult = ImportText_ImportLanguageText( pText, pChildTag, TextRef);
			}
			else
			{
				// If no text has been specified then we generate the text ref for each language 
				TagImportResult = GenerateTextAllLanguages(pText, TextRef);
			}
		}
		else
		{
			TLDebug_Break("Unsupported tag in text import");
		}

		//	failed
		if ( TagImportResult == SyncFalse )
			return SyncFalse;

		//	async
		if ( TagImportResult == SyncWait )
		{
			TLDebug_Break("todo: async TTD import");
			return SyncFalse;
		}
	}
	
	return SyncTrue;
}

SyncBool TLFileSys::TFileTextDatabase::ImportText_ImportLanguageText(TPtr<TLAsset::TText>& pText, TPtr<TXmlTag>& pImportTag, TRefRef TextRef)
{
/*
	<Text TextRef="VolMus">
		<eng>Music Volume</eng>		
		<usa>Music Volume</usa>		
		<fre>Music Volume</fre>		
	</Text>
*/

	// Now get the children of the text tag and process them
	for ( u32 index=0;	index< pImportTag->GetChildren().GetSize();	index++ )
	{
		TPtr<TXmlTag>& pTextTag = pImportTag->GetChildren().ElementAt(index);
		
		// The child tag name is the language ref
		TRef LanguageRef(pTextTag->GetTagName());

		if(!LanguageRef.IsValid())
		{
			TLDebug_Print("Failed to get valid language ref from TTD file");
			return SyncFalse;
		}

		// Now get the text string
		const TString& TextString = pTextTag->GetDataString();

		// And add to the text object
		if(!pText->AddText(LanguageRef, TextRef, TextString))
		{
			TLDebug_Print("Failed to add text string to text object");
		}
	}


	return SyncTrue;
}



SyncBool TLFileSys::TFileTextDatabase::GenerateTextAllLanguages(TPtr<TLAsset::TText>&pText, TRefRef TextRef)
{
	// Add the text ref as text for each language

	return SyncTrue;
}