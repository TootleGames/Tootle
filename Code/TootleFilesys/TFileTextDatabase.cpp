/*
 *  TFileTextDatabase.cpp
 *  TootleFileSys
 *
 *  Created by Duane Bradbury on 15/03/2009.
 *  Copyright 2009 Tootle. All rights reserved.
 *
 */

#include "TFileTextDatabase.h"

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
	
	//	todo
	return SyncFalse;
}
