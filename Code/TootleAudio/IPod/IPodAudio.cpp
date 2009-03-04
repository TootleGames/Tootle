#include "IPodAudio.h"
#include "IPodAudioOpenAL.h"

#include <TootleAsset/TAsset.h>
#include <TootleAsset/TAudio.h>


using namespace TLAudio;

SyncBool Platform::Init()			
{	
	return OpenAL::Init();	
}

SyncBool Platform::Update()		
{	
	return OpenAL::Update();	
}


SyncBool Platform::Shutdown()
{
	return OpenAL::Shutdown();	
}


Bool Platform::CreateSource(TRefRef AudioSourceRef)
{
	TPtr<OpenAL::AudioObj> pAO = OpenAL::CreateSource(AudioSourceRef);
	
	if(pAO)
	{
		TLDebug_Print("Audio source created successfully");	
		return TRUE;
	}
	
	
	TLDebug_Print("Failed to create source for audio");	
	return FALSE;
}

Bool Platform::RemoveSource(TRefRef AudioSourceRef)
{	
	// Release the platform specific buffer data	
	if(!Platform::OpenAL::ReleaseSource(AudioSourceRef))
	{
		// Failed? No low level buffer? 
		TLDebug_Print("Failed to release low level audio source");
		return FALSE;
	}
	
	return TRUE;
}

Bool Platform::CreateBuffer(TRefRef AudioAssetRef)
{
	TPtr<OpenAL::AudioObj> pAO = OpenAL::CreateBuffer(AudioAssetRef);
	
	if(pAO)
	{
		return TRUE;
	}


	TLDebug_Print("Failed to create buffer for audio");	
	return FALSE;
}

Bool Platform::RemoveBuffer(TRefRef AudioAssetRef)
{
	// Release the platform specific buffer data		
	if(!Platform::OpenAL::ReleaseBuffer(AudioAssetRef))
	{
		// Failed? No low level buffer? 
		TLDebug_Print("Failed to release low level audio buffer");
		return FALSE;
	}
	
	return TRUE;
}

Bool Platform::HasBuffer(TRefRef AudioAssetRef)
{
	TPtr<OpenAL::AudioObj> pAO = OpenAL::g_Buffers.FindPtr(AudioAssetRef);
	
	if(pAO)
		return TRUE;
	
	return FALSE;		
}

Bool Platform::HasSource(TRefRef AudioSourceRef)
{
	TPtr<OpenAL::AudioObj> pAO = OpenAL::g_Sources.FindPtr(AudioSourceRef);
	
	if(pAO)
		return TRUE;
	
	return FALSE;		
}



Bool Platform::StartAudio(TRefRef AudioSourceRef)
{
	// Play the source
	return OpenAL::StartAudio(AudioSourceRef);
}

Bool Platform::StopAudio(TRefRef AudioSourceRef)
{
	// Stop the source
	return OpenAL::StopAudio(AudioSourceRef);
}

Bool Platform::PauseAudio(TRefRef AudioSourceRef)
{
	// Pause the source
	return OpenAL::PauseAudio(AudioSourceRef);
}

Bool Platform::SetPitch(TRefRef AudioSourceRef, const float fPitch)
{
	return OpenAL::SetPitch(AudioSourceRef, fPitch);
}

Bool Platform::GetPitch(TRefRef AudioSourceRef, float& fPitch)
{
	return OpenAL::GetPitch(AudioSourceRef, fPitch);	
}


Bool Platform::SetVolume(TRefRef AudioSourceRef, const float fVolume)
{
	return OpenAL::SetVolume(AudioSourceRef, fVolume);
}

Bool Platform::GetVolume(TRefRef AudioSourceRef, float& fVolume)
{
	return OpenAL::GetVolume(AudioSourceRef, fVolume);	
}


Bool Platform::SetLooping(TRefRef AudioSourceRef, const Bool bLooping)
{
	return OpenAL::SetLooping(AudioSourceRef, bLooping);
}

Bool Platform::GetIsLooping(TRefRef AudioSourceRef, Bool& bLooping)
{
	return OpenAL::GetIsLooping(AudioSourceRef, bLooping);	
}


Bool Platform::SetPosition(TRefRef AudioSourceRef, const float3 vPosition)
{
	return OpenAL::SetPosition(AudioSourceRef, vPosition);
}

Bool Platform::GetPosition(TRefRef AudioSourceRef, float3& vPosition)
{
	return OpenAL::GetPosition(AudioSourceRef, vPosition);
}

Bool Platform::SetVelocity(TRefRef AudioSourceRef, const float3 vVelocity)
{
	return OpenAL::SetVelocity(AudioSourceRef, vVelocity);
}

Bool Platform::GetVelocity(TRefRef AudioSourceRef, float3& vVelocity)
{
	return OpenAL::GetVelocity(AudioSourceRef, vVelocity);
}

Bool Platform::SetMinRange(TRefRef AudioSourceRef, const float fDistance)
{
	return OpenAL::SetReferenceDistance(AudioSourceRef, fDistance);
}

Bool Platform::SetRateOfDecay(TRefRef AudioSourceRef, const float fRateOfDecay)
{
	return OpenAL::SetRollOffFactor(AudioSourceRef, fRateOfDecay);
}


Bool Platform::AttachSourceToBuffer(TRefRef AudioSourceRef, TRefRef AudioAssetRef, Bool bStreaming)
{
	// Get the buffer and source OpenAL ID's
	ALuint uBuffer, uSource;
	
	if(!GetBufferID(AudioAssetRef, uBuffer))
	{
		TLDebug_Print("Failed to find audio buffer ID");
		return FALSE;
	}
	
	if(!GetSourceID(AudioSourceRef, uSource))
	{
		TLDebug_Print("Failed to find audio source ID");
		return FALSE;
	}
	
	// Got both buffer and source ID
	// Now attach the source ot the buffer	
	return OpenAL::AttachSourceToBuffer(uSource, uBuffer, bStreaming);	
}


Bool Platform::DetermineFinishedAudio(TArray<TRef>& refArray)
{
	return OpenAL::DetermineFinishedAudio(refArray);
}

void Platform::SetListener(const TListenerProperties& Props)
{
	OpenAL::SetListener(Props);
}


