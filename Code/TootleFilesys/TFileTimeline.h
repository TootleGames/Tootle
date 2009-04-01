/*
 *  TFileTimeline.h
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
	class TFileTimeline;
};

namespace TLAsset
{
	class TKeyframe;
	class TAssetTimeline;
	class TAssetTimelineCommand;
	class TAssetTimelineCommandList;
}

class TLFileSys::TFileTimeline : public TLFileSys::TFileXml
{
public:
	TFileTimeline(TRefRef FileRef,TRefRef FileTypeRef);
	
	virtual SyncBool	ExportAsset(TPtr<TLAsset::TAsset>& pAsset,Bool& Supported);			//	import the XML and convert from Collada to mesh
	
protected:

	SyncBool			ImporTAssetTimeline(TPtr<TLAsset::TAssetTimeline> pAssetTimeline,TPtr<TXmlTag>& pTag);
	SyncBool			ImporTAssetTimeline_ImportKeyframeTag(TPtr<TLAsset::TAssetTimeline>& pAssetTimeline, TPtr<TXmlTag>& pImportTag);
	SyncBool			ImporTAssetTimeline_ImportCommandTag(TPtr<TLAsset::TAssetTimeline>& pAssetTimeline, TLAsset::TKeyframe* pKeyframe, TPtr<TXmlTag>& pImportTag);
	SyncBool			ImporTAssetTimeline_ImportCommandData(TPtr<TLAsset::TAssetTimeline>& pAssetTimeline, TLAsset::TAssetTimelineCommand* pTimelineCommand, TPtr<TXmlTag>& pImportTag);
};
