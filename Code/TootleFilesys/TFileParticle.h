/*------------------------------------------------------
	
	Particle markup (XML)

-------------------------------------------------------*/
#pragma once

#include "TFile.h"
#include "TFileXml.h"


namespace TLAsset
{
	class TParticle;
}

namespace TLFileSys
{
	class TFileParticle;
}



//---------------------------------------------------------
//	
//---------------------------------------------------------
class TLFileSys::TFileParticle : public TLFileSys::TFileXml
{
public:
	TFileParticle(TRefRef FileRef,TRefRef FileTypeRef);

	virtual TRef		GetFileExportAssetType() const										{	return TRef_Static(P,a,r,t,i);	}
	virtual SyncBool	ExportAsset(TPtr<TLAsset::TAsset>& pAsset,Bool& Supported);			//	import the XML and convert from SVG to mesh

protected:
	SyncBool			ImportRoot(TPtr<TXmlTag>& pTag,TLAsset::TParticle& Particle);
};

