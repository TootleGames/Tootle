#include "TAudio.h"

namespace TLAudio
{
	namespace Platform
	{
		Bool CreateBuffer(TLAsset::TAudio&);
		Bool RemoveBuffer(TRefRef);
	}
}

using namespace TLAsset;

TAudio::TAudio(const TRef& AssetRef) :
	TAsset	( GetAssetType_Static(), AssetRef )
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
	
	//	Now we have read the data create the audio buffer for this asset
	//	in the audio system
	//	gr: use the asset directly, if we use the method with fetches the asset
	//	it causes a recursing load and corrupts the state of the asset/load task
	//	note, at this point the asset is still considered "not loaded".
	//	this kind of runtime-buffer thing should really be implemented on the 
	//	thing that USES the asset, not on the asset itself
	//	I assume there may be openAL issues if we load 1000 audio assets (even if we intend to only play 1)
	if( !TLAudio::Platform::CreateBuffer(*this) )
		return SyncFalse;	
		
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
