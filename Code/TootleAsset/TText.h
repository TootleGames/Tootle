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
public:
	typedef TKeyArray<TRef,TBufferString<255> > TTextReplaceTable;

private:
	//	this is just a string, but also includes information on places where we want to do string-insertions (like string-replace)
	class TComplexString
	{
	public:
		TComplexString() 							{}	//	used for import only
		TComplexString(const TString& String);		//	construct the complex string from this string (finds string-replace refs)
		TComplexString(const TComplexString& String);

		FORCEINLINE void		GetString(TString& String) 													{	String.Set( m_String );	}	//	no-string replace
		FORCEINLINE void		GetString(TString& String,const TTextReplaceTable& ReplaceTable);			//	checks to see if we need to do a string replace

		Bool					ImportData(TBinaryTree& Data);	//	load string data out binary data
		Bool					ExportData(TBinaryTree& Data);	//	save string data to binary data

	private:
		void					DoStringReplace(TString& String,const TTextReplaceTable& ReplaceTable);		//	does the string-insertion

	public:
		TKeyArray<TRef,u8>		m_ReplaceRefIndexes;	//	this is a list of where in the string to insert another string
		TBufferString<255>		m_String;				//	original string from the database. gr: I've limited it to a smaller length to make the class less complex. If we need longer strings then just need to change this type and it will all be backwards compatible still
	};


	// The language database is a key array of text refrences to strings
	class TLanguageDatabase : public TKeyArray<TRef, TComplexString>
	{
	public:
		FORCEINLINE Bool	AddText(TRefRef TextRef, const TString& TextString)		{	return (Add(TextRef, TComplexString(TextString) ) != NULL) ;}
		FORCEINLINE Bool	GetText(TRefRef TextRef, TString& TextString);
		FORCEINLINE Bool	GetText(TRefRef TextRef, TString& TextString,const TTextReplaceTable& ReplaceTable);
	
		Bool				ImportData(TBinaryTree& Data);	//	load array of string data out binary data
		Bool				ExportData(TBinaryTree& Data);	//	save array of string data to binary data
	};

public:
	TText(TRefRef AssetRef);

	static TRef				GetAssetType_Static()										{	return TRef_Static4(T,e,x,t);	}
	
	FORCEINLINE Bool		AddText(TRefRef LanguageRef, TRefRef TextRef, const TString& TextString);
	FORCEINLINE Bool		GetText(TRefRef LanguageRef, TRefRef TextRef, TString& TextString);
	FORCEINLINE Bool		GetText(TRefRef LanguageRef, TRefRef TextRef, TString& TextString,const TTextReplaceTable& ReplaceTable);

protected:
	virtual SyncBool		ImportData(TBinaryTree& Data);	//	load asset data out binary data
	virtual SyncBool		ExportData(TBinaryTree& Data);	//	save asset data to binary data
	
private:

	TPtrKeyArray<TRef, TLanguageDatabase>		m_LanguageDatabases;	// key array using the language ref as a key as all language refs must be unique anyway
};




FORCEINLINE Bool TLAsset::TText::AddText(TRefRef LanguageRef, TRefRef TextRef, const TString& TextString)	
{ 
	TPtr<TLanguageDatabase> pTextDB = m_LanguageDatabases.FindPtr(LanguageRef); 
	
	if(pTextDB.IsValid()) 
		return pTextDB->AddText(TextRef, TextString);	
	
	// No database with the language ref yet so add a new one and add the text to the new database
	pTextDB = new TLanguageDatabase();

	if(!pTextDB.IsValid())
		return FALSE;

	if(!m_LanguageDatabases.Add(LanguageRef, pTextDB))
		return FALSE;

	return pTextDB->AddText(TextRef, TextString);	
}


FORCEINLINE Bool TLAsset::TText::GetText(TRefRef LanguageRef, TRefRef TextRef, TString& TextString)			
{	
	//	gr: cache the current language if this is slow, we're never going to want to change language mid-game
	TPtr<TLanguageDatabase>& pTextDB = m_LanguageDatabases.FindPtr(LanguageRef); 
	if( !pTextDB.IsValid() ) 
		return FALSE;
	
	return pTextDB->GetText(TextRef, TextString);
}


FORCEINLINE Bool TLAsset::TText::GetText(TRefRef LanguageRef, TRefRef TextRef, TString& TextString,const TTextReplaceTable& ReplaceTable)			
{	
	//	gr: cache the current language if this is slow, we're never going to want to change language mid-game
	TPtr<TLanguageDatabase>& pTextDB = m_LanguageDatabases.FindPtr(LanguageRef); 
	if( !pTextDB.IsValid() ) 
		return FALSE;
	
	return pTextDB->GetText(TextRef, TextString,ReplaceTable);
}



FORCEINLINE Bool TLAsset::TText::TLanguageDatabase::GetText(TRefRef TextRef, TString& TextString)			
{
	TComplexString* pComplexString = Find(TextRef);	
	if ( !pComplexString )
		return FALSE;
	
	pComplexString->GetString( TextString );
	return TRUE;
}


FORCEINLINE Bool TLAsset::TText::TLanguageDatabase::GetText(TRefRef TextRef, TString& TextString,const TTextReplaceTable& ReplaceTable)			
{
	//	find string with this ref
	TComplexString* pComplexString = Find(TextRef);	
	if ( !pComplexString )
		return FALSE;
	
	//	fetch the string
	pComplexString->GetString( TextString, ReplaceTable );
	return TRUE;
}


FORCEINLINE void TLAsset::TText::TComplexString::GetString(TString& String,const TTextReplaceTable& ReplaceTable)
{
	//	get the string
	GetString( String );	
	
	//	if we have something to replace, and something to replace it with, do the string-insertions
	if ( ReplaceTable.GetSize() > 0 && m_ReplaceRefIndexes.GetSize() > 0 )
	{
		DoStringReplace( String, ReplaceTable );
	}
}

