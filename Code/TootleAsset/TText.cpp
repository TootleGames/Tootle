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
	return SyncTrue;
	
}


//-------------------------------------------------------
//	save asset data to binary data
//-------------------------------------------------------
SyncBool TText::ExportData(TBinaryTree& Data)				
{	
	return SyncTrue;
}	
