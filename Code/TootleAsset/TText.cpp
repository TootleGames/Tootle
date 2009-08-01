/*
 *  TText.cpp
 *  TootleAsset
 *
 *  Created by Duane Bradbury on 15/03/2009.
 *  Copyright 2009 Tootle. All rights reserved.
 *
 */

#include "TText.h"




TLAsset::TText::TText(const TRef& AssetRef) :
	TAsset	( GetAssetType_Static(), AssetRef )
{
}




//-------------------------------------------------------
//	save asset data to binary data
//-------------------------------------------------------
SyncBool TLAsset::TText::ExportData(TBinaryTree& Data)				
{
	//	export each language
	for ( u32 i=0;	i<m_LanguageDatabases.GetSize();	i++ )
	{
		TPtr<TBinaryTree>& pLanguageData = Data.AddChild("Language");
		if ( !pLanguageData )
			return SyncFalse;

		TRefRef LanguageRef = m_LanguageDatabases.GetKeyAt(i);
		TPtr<TLanguageDatabase>& pLanguageDb = m_LanguageDatabases.GetItemAt(i);

		//	write the language ref
		pLanguageData->ExportData( "LangRef", LanguageRef );

		if ( !pLanguageDb->ExportData( *pLanguageData ) )
			return SyncFalse;
	}

	Data.Debug_PrintTree();

	return SyncTrue;
}	


//-------------------------------------------------------
//	save asset data to binary data
//-------------------------------------------------------
SyncBool TLAsset::TText::ImportData(TBinaryTree& Data)				
{
	//	get all language databases
	TPtrArray<TBinaryTree> LanguagesData;
	Data.GetChildren("Language", LanguagesData );

	//	import each language
	for ( u32 i=0;	i<LanguagesData.GetSize();	i++ )
	{
		TBinaryTree& LanguageData = *(LanguagesData[i]);
		TRef LanguageRef;
		if ( !LanguageData.ImportData("LangRef", LanguageRef ) )
			return SyncFalse;

		//	get the language database, or create it if it doesnt exist
		TPtr<TLanguageDatabase> pLanguageDb = m_LanguageDatabases.FindPtr( LanguageRef );
		if ( !pLanguageDb )
		{
			TPtr<TLanguageDatabase>* ppLanguageDb = m_LanguageDatabases.Add( LanguageRef, new TLanguageDatabase );
			if ( ppLanguageDb )
			{
				pLanguageDb = *ppLanguageDb;
			}
		}

		//	failed to find/create
		if ( !pLanguageDb )
			return SyncFalse;

		//	import data
		if ( !pLanguageDb->ImportData( LanguageData ) )
			return SyncFalse;
	}

	return SyncTrue;
}	



//-------------------------------------------------------
//	save array of string data to binary data
//-------------------------------------------------------
Bool TLAsset::TText::TLanguageDatabase::ExportData(TBinaryTree& Data)
{
	//	make an entry for each string
	for ( u32 i=0;	i<this->GetSize();	i++ )
	{
		TRefRef StringRef = this->GetKeyAt( i );
		TComplexString& ComplexString = this->GetItemAt( i );
		TPtr<TBinaryTree>& pStringData = Data.AddChild("String");
		if ( !pStringData )
			return FALSE;

		//	write the ref
		pStringData->ExportData( "StringRef", StringRef );

		//	now write more complex data
		if ( !ComplexString.ExportData( *pStringData ) )
			return FALSE;
	}

	return TRUE;
}


//-------------------------------------------------------
//	import array of string data to binary data
//-------------------------------------------------------
Bool TLAsset::TText::TLanguageDatabase::ImportData(TBinaryTree& Data)
{
	//	get all strings
	TPtrArray<TBinaryTree> StringDatas;
	Data.GetChildren("String", StringDatas );

	//	import each string
	for ( u32 i=0;	i<StringDatas.GetSize();	i++ )
	{
		TBinaryTree& StringData = *(StringDatas[i]);
		TRef StringRef;
		if ( !StringData.ImportData("StringRef", StringRef ) )
			return FALSE;

		//	create new entry
		TComplexString* pComplexString = Add( StringRef, TComplexString(), FALSE );
		if ( !pComplexString )
		{
			TTempString Debug_String("Failed to add complex string entry - text ref already exists?");
			StringRef.GetString( Debug_String );
			if ( !TLDebug_Break( Debug_String ) )
				return FALSE;
			else
				continue;
		}

		//	import data
		if ( !pComplexString->ImportData( StringData ) )
			return FALSE;
	}

	return TRUE;
}



//--------------------------------------------------------------
//	construct the complex string from this string (finds string-replace refs)
//--------------------------------------------------------------
TLAsset::TText::TComplexString::TComplexString(const TString& String)
{
	s32 BraceStart = -1;
	s32 BraceEnd = -1;

	//	hello {foo} world

	BraceStart = String.GetCharIndex('{');
	while ( BraceStart != -1 )
	{
		//	copy the string up to this point (hello )
		m_String.Append( String, BraceEnd+1, BraceStart-(BraceEnd+1) );

		//	find the end-brace
		BraceEnd = String.GetCharIndex('}', BraceStart );

		//	really we should always have a matching close-brace
		if ( BraceEnd == -1 )
		{
			TLDebug_Break("Mis-matched braces in string when looking for string-replace refs.");
			break;
		}

		//	get the ref for this replace
		TTempString ReplaceRefString;
		ReplaceRefString.Append( String, BraceStart+1, BraceEnd-1-BraceStart );
		TRef ReplaceRef( ReplaceRefString );

		//	found a pair of braces but no valid ref inside... don't store this
		if ( !ReplaceRef.IsValid() )
		{
			TLDebug_Break("Failed to get replace-ref inside string's braces");

			//	get next starting index
			BraceStart = String.GetCharIndex('{', BraceEnd );
			continue;
		}

		#ifdef _DEBUG
		TTempString Debug_String("Found replace-ref in string: ");
		ReplaceRef.GetString( Debug_String );
		TLDebug_Print( Debug_String );
		#endif

		//	add to table
		m_ReplaceRefIndexes.Add( ReplaceRef, m_String.GetLength() );

		//	get next starting index
		BraceStart = String.GetCharIndex('{', BraceEnd );
	}

	//	copy the end of the string ( world)
	if ( BraceEnd != -1 )
	{
		m_String.Append( String, BraceEnd+1, -1 );
	}
	
	//	no braces found at all, copy the entire string
	if ( BraceStart == -1 && BraceEnd == -1 )
	{
		//	gr: using append to catch bugs.
		m_String.Append( String );
	}

}


//--------------------------------------------------------------
//	
//--------------------------------------------------------------
TLAsset::TText::TComplexString::TComplexString(const TComplexString& String)
{
	m_ReplaceRefIndexes.Copy( String.m_ReplaceRefIndexes );
	m_String.Set( String.m_String );
}



//--------------------------------------------------------------
//	save string data to binary data
//--------------------------------------------------------------
Bool TLAsset::TText::TComplexString::ExportData(TBinaryTree& Data)
{
	//	write the string
	Data.ExportDataString("RawString", m_String );

	//	write all the entries in the string-insertion table
	for ( u32 i=0;	i<m_ReplaceRefIndexes.GetSize();	i++ )
	{
		TPtr<TBinaryTree>& pReplaceEntry = Data.AddChild("Replace");
		if ( !pReplaceEntry )
			return FALSE;

		//	write ref
		pReplaceEntry->ExportData("Ref", m_ReplaceRefIndexes.GetKeyAt(i) );

		//	write insertion index
		pReplaceEntry->ExportData("Index", m_ReplaceRefIndexes.GetItemAt(i) );
	}

	return TRUE;
}


//--------------------------------------------------------------
//	save string data to binary data
//--------------------------------------------------------------
Bool TLAsset::TText::TComplexString::ImportData(TBinaryTree& Data)
{
	//	import the string
	if ( !Data.ImportDataString("RawString", m_String ) )
	{
		Data.Debug_PrintTree();
		return FALSE;
	}

	//	get all the string-insert table entrys
	TPtrArray<TBinaryTree> ReplaceDatas;
	Data.GetChildren("Replace", ReplaceDatas );

	for ( u32 i=0;	i<ReplaceDatas.GetSize();	i++ )
	{
		TBinaryTree& ReplaceData = *(ReplaceDatas[i]);
		TRef ReplaceRef;
		if ( !ReplaceData.ImportData("Ref",ReplaceRef) )
			return FALSE;

		u8 Index = 0;
		if ( !ReplaceData.ImportData("Index", Index) )
			return FALSE;

		//	add table entry
		m_ReplaceRefIndexes.Add( ReplaceRef, Index );
	}

	return TRUE;
}

	

//--------------------------------------------------------------
//	does the string-insertion
//--------------------------------------------------------------
void TLAsset::TText::TComplexString::DoStringReplace(TString& String,const TTextReplaceTable& ReplaceTable)
{
	//	gr: we MUST go through this list IN order due to this;
	//	if we have "{X}{Y}" in a string, (wuthout a gap) they will both have the same insert-index
	//	if we did the insertion out-of-order then we might end up with stuff in the wrong order as both 
	//	will be inserted at the same index.

	//	whenever we insert a string, the original insert-index needs to move along to be in the same place in the string
	u32 IndexOffset = 0;

	//	go through the complex string's list and see what needs replacing
	for ( u32 i=0;	i<m_ReplaceRefIndexes.GetSize();	i++ )
	{
		TRefRef ReplaceRef = m_ReplaceRefIndexes.GetKeyAt(i);

		//	find the string provided for this replacement
		const TString* pInsertString = ReplaceTable.Find( ReplaceRef );

		//	none provided, string unaltered
		if ( !pInsertString )
			continue;

		//	get the index to insert this string at
		u8 Index = m_ReplaceRefIndexes.GetItemAt(i);

		//	insert the replacing string into the resultant string
		String.InsertAt( Index + IndexOffset, *pInsertString );

		//	move the followng insert indexes past the inserted string
		IndexOffset += pInsertString->GetLength();
	}
}


