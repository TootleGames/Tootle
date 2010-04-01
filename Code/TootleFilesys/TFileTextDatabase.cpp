/*
 *  TFileTextDatabase.cpp
 *  TootleFileSys
 *
 *  Created by Duane Bradbury on 15/03/2009.
 *  Copyright 2009 Tootle. All rights reserved.
 *
 */

#include "TFileTextDatabase.h"
#include "TLFile.h"

#include <TootleAsset/TText.h>
#include <TootleGame/TTextManager.h>



TLFileSys::TFileTextDatabase::TFileTextDatabase(TRefRef FileRef,TRefRef FileTypeRef) :
TFileXml			( FileRef, FileTypeRef )
{
}



//--------------------------------------------------------
//	import the XML and convert from SVG to mesh
//--------------------------------------------------------
SyncBool TLFileSys::TFileTextDatabase::ExportAsset(TPtr<TLAsset::TAsset>& pAsset,TRefRef ExportAssetType)
{
	if ( pAsset )
	{
		TLDebug_Break("Async export not supported yet. asset should be NULL");
		return SyncFalse;
	}

	//	import xml
	SyncBool ImportResult = TFileXml::Import();
	if ( ImportResult != SyncTrue )
		return ImportResult;

	//	get the root tag
	TPtr<TXmlTag> pTtdTag = m_XmlData.GetChild("textdatabase");

	//	malformed AssetScript
	if ( !pTtdTag )
	{
		TLDebug_Print("TTD file missing root <textdatabase> tag");
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

	//	temporary string to put a cleaner data stirng into
	TString TempString;	

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
		const TString& DataString = pTextTag->GetDataString();

		//	if the data string is dirty (needs line feed conversion etc) then convert it into a new string
		Bool IsDataStringDirty = TLString::IsStringDirty( DataString );
		if ( IsDataStringDirty )
		{
			TempString = DataString;
			TLString::CleanString( TempString );
		}

		//	final [clean] string
		const TString& TextString = IsDataStringDirty ? TempString : DataString;

		// And add to the text object
		if(!pText->AddText(LanguageRef, TextRef, TextString))
		{
			TLDebug_Break("Failed to add text string to text object");
		}
	}


	return SyncTrue;
}



SyncBool TLFileSys::TFileTextDatabase::GenerateTextAllLanguages(TPtr<TLAsset::TText>&pText, TRefRef TextRef)
{
	// Add the text ref as text for each language
	TArray<TRef>& SupportedLanguages = TLText::g_pTextManager->GetSupportedLanguages();

	// Get the ref as a string
	TString TextString;
	TextRef.GetString(TextString);

	for(u32 uIndex = 0; uIndex < SupportedLanguages.GetSize(); uIndex++)
	{
		// Get the language
		
		TRef LanguageRef = SupportedLanguages.ElementAt(uIndex);
		
		// Add to the text object
		if(!pText->AddText(LanguageRef, TextRef, TextString))
		{
			TLDebug_Print("Failed to add text string to text object");
		}
	}

	return SyncTrue;
}