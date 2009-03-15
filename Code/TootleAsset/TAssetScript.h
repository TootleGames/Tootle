/*
 *  TAssetScript.h
 *  TootleAsset
 *
 *  Created by Duane Bradbury on 15/03/2009.
 *  Copyright 2009 Tootle. All rights reserved.
 *
 */

#pragma once

#include <TootleAsset/TLAsset.h>
#include <TootleAsset/TAsset.h>


namespace TLAsset
{
	class TAssetScript;
};



class TLAsset::TAssetScript : public TLAsset::TAsset
{
public:
	TAssetScript(const TRef& AssetRef);
	
protected:
	virtual SyncBool		ImportData(TBinaryTree& Data);	//	load asset data out binary data
	virtual SyncBool		ExportData(TBinaryTree& Data);	//	save asset data to binary data
	
private:
};