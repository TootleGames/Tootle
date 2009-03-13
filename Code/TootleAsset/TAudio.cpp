#include "TAudio.h"

#include <TootleAudio/TLAudio.h>

using namespace TLAsset;

TAudio::TAudio(const TRef& AssetRef) :
	TAsset	( "Audio", AssetRef )
{
}

SyncBool TAudio::Shutdown()
{
	// Release the audio buffer
	TLAudio::Platform::RemoveBuffer(GetAssetRef());

	return TAsset::Shutdown();	
}



//-------------------------------------------------------
//	load asset data out binary data
//-------------------------------------------------------
SyncBool TAudio::ImportData(TBinaryTree& Data)		
{
	TPtrArray<TBinaryTree> DataArray;
	
	//TODO: Make async

	// Import Header
	TPtr<TBinaryTree>& pChild = Data.GetChild("Header");
#ifdef _DEBUG
	if(!pChild)
		TLDebug_Break("Unable to import audio header");
#endif	
	pChild->ResetReadPos();
	pChild->Read(m_HeaderData);

	
	// Import audio data
	pChild = Data.GetChild("Audio");
#ifdef _DEBUG
	if(!pChild)
		TLDebug_Break("Unable to import audio header");
#endif	
	pChild->ResetReadPos();
	
	pChild->ReadAll(m_RawAudioData);
	
	
	// Now we have read the data create the audio buffer for this asset
	// in the audio system
	if(!TLAudio::Platform::CreateBuffer(GetAssetRef()))
		return SyncFalse;	
		
	return SyncTrue;

}


//-------------------------------------------------------
//	save asset data to binary data
//-------------------------------------------------------
SyncBool TAudio::ExportData(TBinaryTree& Data)				
{	
	TPtr<TBinaryTree> pChildData = Data.AddChild("Header");
	
#ifdef _DEBUG
	if(!pChildData)
		TLDebug_Break("Failed to create child");
#endif	
	// Write the Header data
	pChildData->Write(m_HeaderData);
	
	pChildData = Data.AddChild("Audio");

#ifdef _DEBUG
	if(!pChildData)
		TLDebug_Break("Failed to create child");
#endif	
	// Write the Audio data
	pChildData->Copy(RawAudioDataBinary());
	
		
	return SyncTrue;
}	
