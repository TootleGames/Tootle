/*
 *  TFileTextDatabase.h
 *  TootleFileSys
 *
 *  Created by Duane Bradbury on 15/03/2009.
 *  Copyright 2009 Tootle. All rights reserved.
 *
 */

#pragma once

#include "TFile.h"
#include "TFileXml.h"


namespace TLFileSys
{
	class TFileTextDatabase;
};

namespace TLAsset
{
	class TText;
}


class TLFileSys::TFileTextDatabase : public TLFileSys::TFileXml
{
public:
	TFileTextDatabase(TRefRef FileRef,TRefRef FileTypeRef);
	
	virtual SyncBool	ExportAsset(TPtr<TLAsset::TAsset>& pAsset,Bool& Supported);			//	import the XML and convert from Collada to mesh
protected:

	SyncBool			ImportText(TPtr<TLAsset::TText> pText, TPtr<TXmlTag>& pTag);
	SyncBool			ImportText_ImportLanguageText(TPtr<TLAsset::TText>& pText, TPtr<TXmlTag>& pImportTag, TRefRef TextRef);

	SyncBool			GenerateTextAllLanguages(TPtr<TLAsset::TText>&pText, TRefRef TextRef);

};
