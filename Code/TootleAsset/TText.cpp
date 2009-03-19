/*
 *  TText.cpp
 *  TootleAsset
 *
 *  Created by Duane Bradbury on 15/03/2009.
 *  Copyright 2009 Tootle. All rights reserved.
 *
 */

#include "TText.h"

using namespace TLAsset;

TText::TText(const TRef& AssetRef) :
TAsset	( "Text", AssetRef )
{
}


//-------------------------------------------------------
//	load asset data out binary data
//-------------------------------------------------------
SyncBool TText::ImportData(TBinaryTree& Data)		
{
	u32 uLanguageDBSize;
	
	if(!Data.Read(uLanguageDBSize))
	{
		TLDebug_Break("Failed to read TDD language count");
		return SyncFalse;
	}

	// No items in the database
	if(uLanguageDBSize == 0)
	{
		TLDebug_Warning("No items in the text database for asset import");
		return SyncTrue;
	}

	for(u32 uIndex = 0; uIndex < uLanguageDBSize; uIndex++)
	{
		// Read language key ref
		TRef LanguageRef;
		if(!Data.Read(LanguageRef))
		{
			TLDebug_Break("Failed to read TDD language key");
			return SyncFalse;
		}

		// Read how many text items we have
		u32 uTextDBSize;
		if(!Data.Read(uTextDBSize))
		{
			TLDebug_Break("Failed to read TDD text count");
			return SyncFalse;
		}

		for(u32 uIndex2 = 0; uIndex2 < uTextDBSize; uIndex2++)
		{
			TRef TextRef;
			TString TextString;

			// Read the ref and string
			if(!Data.Read(TextRef))
			{
				// Failed
				TLDebug_Break("Failed to read text ref");
				return SyncFalse;
			}
				
			if(!Data.ReadString(TextString))
			{
				// Failed
				TLDebug_Break("Failed to read text string");
				return SyncFalse;
			}

			// Add the text ref and string
			if(!AddText(LanguageRef, TextRef, TextString))
			{
				TLDebug_Break("Failed to add text ref or string to text database");
			}
		}
	}


	return SyncTrue;
	
}


//-------------------------------------------------------
//	save asset data to binary data
//-------------------------------------------------------
SyncBool TText::ExportData(TBinaryTree& Data)				
{	
	u32 uLanguageDBSize = m_LanguageDatabases.GetSize();

	// No items in the database
	if(uLanguageDBSize == 0)
	{
		TLDebug_Warning("No items in the text database for asset export");
		return SyncTrue;
	}

	// Write how many items we have
	Data.Write(uLanguageDBSize);

	for(u32 uIndex = 0; uIndex < uLanguageDBSize; uIndex++)
	{
		const TLKeyArray::TPair<TRef,TPtr<TLanguageDatabase> >& KeyPair = m_LanguageDatabases.GetPairAt(uIndex);

		// Write the key
		Data.Write(KeyPair.m_Key);

		// Now go through the text within the language database and write out each language object
		const TPtr<TLanguageDatabase>& LDB = KeyPair.m_Item;
		u32 uTextDBSize = LDB->GetSize();

		// Write how many items we have
		Data.Write(uTextDBSize);

		for(u32 uIndex2 = 0; uIndex2 < uTextDBSize; uIndex2++)
		{
			const TLKeyArray::TPair<TRef,TString>& KeyPair2 = LDB->GetPairAt(uIndex2);

			// Write the ref and the string
			Data.Write(KeyPair2.m_Key);
			Data.WriteString(KeyPair2.m_Item);
		}
	}

	return SyncTrue;
}	
