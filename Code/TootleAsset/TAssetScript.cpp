/*
 *  TAssetScript.cpp
 *  TootleAsset
 *
 *  Created by Duane Bradbury on 15/03/2009.
 *  Copyright 2009 Tootle. All rights reserved.
 *
 */

#include "TAssetScript.h"


using namespace TLAsset;

TAssetScript::TAssetScript(const TRef& AssetRef) :
TAsset	( "AScript", AssetRef )
{
}



//-------------------------------------------------------
//	load asset data out binary data
//-------------------------------------------------------
SyncBool TAssetScript::ImportData(TBinaryTree& Data)		
{
	return SyncTrue;
	
}


//-------------------------------------------------------
//	save asset data to binary data
//-------------------------------------------------------
SyncBool TAssetScript::ExportData(TBinaryTree& Data)				
{	
	return SyncTrue;
}	
