/*------------------------------------------------------
 
 WAV file
 
 -------------------------------------------------------*/
#pragma once

#include <TootleCore/TString.h>
#include "TFile.h"
#include <TootleAsset/TAsset.h>


namespace TLFileSys
{
	class TFileWAV;	
};



//---------------------------------------------------------
//	
//---------------------------------------------------------
class TLFileSys::TFileWAV : public TLFileSys::TFile
{
public:
	TFileWAV(TRefRef FileRef,TRefRef FileTypeRef);
	
	SyncBool			Import();					
	//	SyncBool			Export();					//	export XML to binary data
	
	//	virtual SyncBool	Export(TPtr<TFileAsset>& pAssetFile);	//	turn XML into a more binary format. XML tags become ref's, data and properties become child binarytree's
	
protected:
	
};
