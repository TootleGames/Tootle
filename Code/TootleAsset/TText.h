/*
 *  TText.h
 *  TootleAsset
 *
 *  Created by Duane Bradbury on 15/03/2009.
 *  Copyright 2009 Tootle. All rights reserved.
 *
 */

#pragma once

#include <TootleAsset/TLAsset.h>
#include <TootleAsset/TAsset.h>
#include <TootleCore/TKeyArray.h>

namespace TLAsset
{
	class TText;
};




class TLAsset::TText : public TLAsset::TAsset
{
	// The language database is a key array of text refrences to strings
	class TLanguageDatabase : public TKeyArray<TRef, TString>
	{
	public:
		FORCEINLINE Bool	AddText(TRefRef TextRef, const TString& TextString)		{ return (Add(TextRef, TextString) != NULL) ;}
		FORCEINLINE Bool	GetText(TRefRef TextRef, TString& TextString)			{ TString* pString = Find(TextRef);  if(!pString) return FALSE; TextString = *pString; return TRUE;  }
	};

public:
	TText(const TRef& AssetRef);
	
	FORCEINLINE Bool	AddText(TRefRef LanguageRef, TRefRef TextRef, const TString& TextString)	
	{ 
		TPtr<TLanguageDatabase> pDB = m_LanguageDatabases.FindPtr(LanguageRef); 
		
		if(pDB.IsValid()) 
			return pDB->AddText(TextRef, TextString);	
		else
		{
			// No database with the language ref yet so add a new one and add the text to the new database
			pDB = new TLanguageDatabase();

			if(pDB.IsValid())
			{
				if(m_LanguageDatabases.Add(LanguageRef, pDB))
					return pDB->AddText(TextRef, TextString);	
			}
		}

		return FALSE;
		
	}

	FORCEINLINE Bool	GetText(TRefRef LanguageRef, TRefRef TextRef, TString& TextString)			{ TPtr<TLanguageDatabase> pDB = m_LanguageDatabases.FindPtr(LanguageRef); if(!pDB.IsValid()) return FALSE; return pDB->GetText(TextRef, TextString);	}

protected:
	virtual SyncBool		ImportData(TBinaryTree& Data);	//	load asset data out binary data
	virtual SyncBool		ExportData(TBinaryTree& Data);	//	save asset data to binary data
	
private:

	TPtrKeyArray<TRef, TLanguageDatabase>		m_LanguageDatabases;	// key array using the language ref as a key as all language refs must be unique anyway
};