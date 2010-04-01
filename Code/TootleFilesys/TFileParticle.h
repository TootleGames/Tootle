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

	virtual SyncBool	ExportAsset(TPtr<TLAsset::TAsset>& pAsset,TRefRef ExportAssetType);
	virtual void		GetSupportedExportAssetTypes(TArray<TRef>& SupportedTypes) const	{	SupportedTypes << TRef_Static(P,a,r,t,i);	}

protected:
	SyncBool			ImportRoot(TPtr<TXmlTag>& pTag,TLAsset::TParticle& Particle);
};

