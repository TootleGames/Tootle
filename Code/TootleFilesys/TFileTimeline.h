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
	class TTimeline;
	class TTimelineCommand;
	class TTimelineCommandList;
}

class TLFileSys::TFileTimeline : public TLFileSys::TFileXml
{
public:
	TFileTimeline(TRefRef FileRef,TRefRef FileTypeRef);
	
	virtual SyncBool	ExportAsset(TPtr<TLAsset::TAsset>& pAsset,Bool& Supported);			//	import the XML and convert from Collada to mesh
	
protected:

	SyncBool			ImporTTimeline(TPtr<TLAsset::TTimeline> pAssetTimeline,TPtr<TXmlTag>& pTag);
	SyncBool			ImporTTimeline_ImportKeyframeTag(TPtr<TLAsset::TTimeline>& pAssetTimeline, TPtr<TXmlTag>& pImportTag);
	SyncBool			ImporTTimeline_ImportCommandTag(TPtr<TLAsset::TTimeline>& pAssetTimeline, TLAsset::TKeyframe* pKeyframe, TPtr<TXmlTag>& pImportTag);
	SyncBool			ImporTTimeline_ImportCommandData(TPtr<TLAsset::TTimeline>& pAssetTimeline, TLAsset::TTimelineCommand* pTimelineCommand, TPtr<TXmlTag>& pImportTag);
};
