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
	~TFileWAV();
	
	virtual TRef				GetFileExportAssetType() const										{	return TRef_Static(A,u,d,i,o);	}
	virtual SyncBool			ExportAsset(TPtr<TLAsset::TAsset>& pAsset,Bool& Supported);
	
protected:
	Bool						InitExport(TPtr<TLAsset::TAsset>& pAsset);
	SyncBool					UpdateExport(TPtr<TLAsset::TAsset>& pAsset);
	void						ShutdownExport(Bool DeleteFont);
	
	Bool						LoadHeader(TPtr<TLAsset::TAsset>& pAsset);

	// Header data
	struct descriptor
	{
		char	riff[4];	// 'RIFF' tag
		u32		uSize;		// 36 + SubChunk2Size, or more precisely:
							// 4 + (8 + (formatheader)uSize) + (8 + (audioheader)uSize)
							// This is the size of the rest of the chunk 
							// following this number.  This is the size of the 
							// entire file in bytes minus 8 bytes for the
							// two fields not included in this count:
							// riff[4] and uSize.
			
		char	wave[4];	// 'WAVE' tag
	};
	
	struct formatdataheader
	{
		char	fmt[4];		// 'FMT ' tag (NOTE the space at the end)
		u32		uSize;		// Size of format data chunk - should be equal to sizeof(formatdata) *but* can be larger
	};

	struct formatdata
	{
		u16		uFormat;			// Format info - tells you the compression if any.  Should be 1 for PCM i.e. no compression
		u16		uNumChannels;		// Number of channels
		u32		uSampleBitrate;		// Sample bitrate
		u32		uByteRate;			// Average byte rate == uSampleBitrate * uNumChannels * uBitsPerSample/8
		u16		uBlockalign;		// Block align value == uNumChannels * uBitsPerSample/8
		u16		uBitsPerSample;		// Bits per sample
	};
	
	struct audiodataheader
	{
		char	data[4];			// 'DATA' tag
		u32		uSize;				// Size of audio data chunk
	};
	
private:
	Bool						m_bHeaderLoaded;
};
