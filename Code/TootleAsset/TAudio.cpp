#include "TAudio.h"

using namespace TLAsset;

TAudio::TAudio(const TRef& AssetRef) :
	TAsset	( GetAssetType_Static(), AssetRef )
{
}

SyncBool TAudio::Shutdown()
{
	return TAsset::Shutdown();	
}



//-------------------------------------------------------
//	load asset data out binary data
//-------------------------------------------------------
SyncBool TAudio::ImportData(TBinaryTree& Data)		
{
	// Import Header
	Data.ImportData("Header", m_HeaderData );

	// Import audio data
	TPtr<TBinaryTree>& pAudioData = Data.GetChild("Audio");
	if(!pAudioData)
	{
		TLDebug_Break("Unable to import audio header");
		return SyncFalse;
	}

	pAudioData->ResetReadPos();
	pAudioData->ReadAll(m_RawAudioData);
			
	return SyncTrue;
}


//-------------------------------------------------------
//	save asset data to binary data
//-------------------------------------------------------
SyncBool TAudio::ExportData(TBinaryTree& Data)				
{	
	//	write header
	Data.ExportData("Header", m_HeaderData);
	
	//	write audio data
	TPtr<TBinaryTree>& pAudioData = Data.AddChild("Audio");
	if ( !pAudioData )
		return SyncFalse;
	pAudioData->Copy(RawAudioDataBinary());
			
	return SyncTrue;
}	
