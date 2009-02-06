/*------------------------------------------------------
	
	Collada file format (simple vector graphics)
	Converts to mesh

-------------------------------------------------------*/
#pragma once

#include "TFile.h"
#include "TFileXml.h"


namespace TLFileSys
{
	class TFileCollada;
};

namespace TLAsset
{
	class TMesh;
}



//---------------------------------------------------------
//	collada xml file
//---------------------------------------------------------
class TLFileSys::TFileCollada : public TLFileSys::TFileXml
{
public:
	TFileCollada(TRefRef FileRef,TRefRef FileTypeRef);

	virtual SyncBool	ExportAsset(TPtr<TLAsset::TAsset>& pAsset,Bool& Supported);			//	import the XML and convert from Collada to mesh

protected:
};

