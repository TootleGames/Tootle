#include "TFileWAV.h"





//---------------------------------------------------------
//	
//---------------------------------------------------------
TLFileSys::TFileWAV::TFileWAV(TRefRef FileRef,TRefRef FileTypeRef) :
TFile	( FileRef, FileTypeRef )
{
}


//---------------------------------------------------------
//	Import WAV audio data
//---------------------------------------------------------
SyncBool TLFileSys::TFileWAV::Import()
{
	
	return SyncTrue;
}

