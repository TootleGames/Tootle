
#include "PCAudio.h"
#include "PCAudioDirectX.h"
#include "PCAudioOpenAL.h"


using namespace TLAudio;

SyncBool Platform::Init()			
{	
	#if(AUDIO_SYSTEM == AUDIO_DIRECTX)
		return DirectX::Init();	
	#else if(AUDIO_SYSTEM == AUDIO_OPENAL)
		return OpenAL::Init();	
	#endif
}

SyncBool Platform::Update()		
{	
#if(AUDIO_SYSTEM == AUDIO_DIRECTX)
	return DirectX::Update();	
#else if(AUDIO_SYSTEM == AUDIO_OPENAL)
	return OpenAL::Update();	
#endif

	return SyncTrue;	
}


SyncBool Platform::Shutdown()
{
	#if(AUDIO_SYSTEM == AUDIO_DIRECTX)
		return DirectX::Shutdown();	
	#else if(AUDIO_SYSTEM == AUDIO_OPENAL)
		return OpenAL::Shutdown();	
	#endif
}


Bool Platform::CreateSource(TRefRef AudioSourceRef)
{
#if(AUDIO_SYSTEM == AUDIO_OPENAL)
	if(!OpenAL::CreateSource(AudioSourceRef))
	{
		TLDebug_Print("Failed to create source for audio");	
		return FALSE;
	}
	return TRUE;
#endif
	return FALSE;
}

Bool Platform::RemoveSource(TRefRef AudioSourceRef)
{	
#if(AUDIO_SYSTEM == AUDIO_OPENAL)

	// Release the platform specific buffer data	
	if(!Platform::OpenAL::ReleaseSource(AudioSourceRef))
	{
		// Failed? No low level buffer? 
		TLDebug_Print("Failed to release low level audio source");
		return FALSE;
	}
#endif

	return TRUE;
}

Bool Platform::CreateBuffer(TLAsset::TAudio& AudioAsset)
{
#if(AUDIO_SYSTEM == AUDIO_OPENAL)
	if(!OpenAL::CreateBuffer(AudioAsset))
	{
		TLDebug_Print("Failed to create buffer for audio");	
		return FALSE;
	}
	return TRUE;
#endif

	return FALSE;
}

Bool Platform::RemoveBuffer(TRefRef AudioAssetRef)
{
#if(AUDIO_SYSTEM == AUDIO_OPENAL)

	// Release the platform specific buffer data		
	if(Platform::OpenAL::ReleaseBuffer(AudioAssetRef))
		return TRUE;
#endif	

	// Failed? No low level buffer? 
	TLDebug_Print("Failed to release low level audio buffer");
	return FALSE;
}

Bool Platform::HasBuffer(TRefRef AudioAssetRef)
{
#if(AUDIO_SYSTEM == AUDIO_OPENAL)
	return OpenAL::HasBuffer(AudioAssetRef);
#endif
	
	return FALSE;		
}

Bool Platform::HasSource(TRefRef AudioSourceRef)
{
#if(AUDIO_SYSTEM == AUDIO_OPENAL)
	return OpenAL::HasSource(AudioSourceRef);
#endif

	return FALSE;		
}



Bool Platform::StartAudio(TRefRef AudioSourceRef)
{
#if(AUDIO_SYSTEM == AUDIO_OPENAL)
	// Play the source
	return OpenAL::StartAudio(AudioSourceRef);
#endif

	TLDebug_Break("TODO");
	return FALSE;
}

Bool Platform::StopAudio(TRefRef AudioSourceRef)
{
#if(AUDIO_SYSTEM == AUDIO_OPENAL)
	// Stop the source
	return OpenAL::StopAudio(AudioSourceRef);
#endif

	TLDebug_Break("TODO");
	return FALSE;
}

Bool Platform::PauseAudio(TRefRef AudioSourceRef)
{
#if(AUDIO_SYSTEM == AUDIO_OPENAL)
	// Stop the source
	return OpenAL::PauseAudio(AudioSourceRef);
#endif

	TLDebug_Break("TODO");
	return FALSE;
}


Bool Platform::SetPitch(TRefRef AudioSourceRef, const float fPitch)
{
#if(AUDIO_SYSTEM == AUDIO_OPENAL)
	// Stop the source
	return OpenAL::SetPitch(AudioSourceRef, fPitch);
#endif

	TLDebug_Break("TODO");
	return FALSE;
}

Bool Platform::GetPitch(TRefRef AudioSourceRef, float& fPitch)
{
	TLDebug_Break("TODO");
	return FALSE;
}


Bool Platform::SetVolume(TRefRef AudioSourceRef, const float fVolume)
{
#if(AUDIO_SYSTEM == AUDIO_OPENAL)
	return OpenAL::SetVolume(AudioSourceRef, fVolume);
#endif

	return FALSE;
}

Bool Platform::GetVolume(TRefRef AudioSourceRef, float& fVolume)
{
#if(AUDIO_SYSTEM == AUDIO_OPENAL)
	return OpenAL::GetVolume(AudioSourceRef, fVolume);	
#endif

	return FALSE;
}


Bool Platform::SetLooping(TRefRef AudioSourceRef, const Bool bLooping)
{
#if(AUDIO_SYSTEM == AUDIO_OPENAL)
	return OpenAL::SetLooping(AudioSourceRef, bLooping);
#endif

	return FALSE;
}

Bool Platform::GetIsLooping(TRefRef AudioSourceRef, Bool& bLooping)
{
#if(AUDIO_SYSTEM == AUDIO_OPENAL)
	return OpenAL::GetIsLooping(AudioSourceRef, bLooping);	
#endif

	return FALSE;
}

Bool Platform::SetRelative(TRefRef AudioSourceRef, const Bool bRelative)
{
#if(AUDIO_SYSTEM == AUDIO_OPENAL)
	return OpenAL::SetRelative(AudioSourceRef, bRelative);
#endif
	return FALSE;
}

Bool Platform::GetIsRelative(TRefRef AudioSourceRef, Bool& bRelative)
{
#if(AUDIO_SYSTEM == AUDIO_OPENAL)
	return OpenAL::GetIsLooping(AudioSourceRef, bRelative);	
#endif
	return FALSE;
}



Bool Platform::SetPosition(TRefRef AudioSourceRef, const float3 vPosition)
{
#if(AUDIO_SYSTEM == AUDIO_OPENAL)
	return OpenAL::SetPosition(AudioSourceRef, vPosition);
#endif
	return FALSE;
}

Bool Platform::GetPosition(TRefRef AudioSourceRef, float3& vPosition)
{
#if(AUDIO_SYSTEM == AUDIO_OPENAL)
	return OpenAL::GetPosition(AudioSourceRef, vPosition);
#endif
	return FALSE;
}

Bool Platform::SetVelocity(TRefRef AudioSourceRef, const float3 vVelocity)
{
#if(AUDIO_SYSTEM == AUDIO_OPENAL)
	return OpenAL::SetVelocity(AudioSourceRef, vVelocity);

#endif
	return FALSE;
}

Bool Platform::GetVelocity(TRefRef AudioSourceRef, float3& vVelocity)
{
#if(AUDIO_SYSTEM == AUDIO_OPENAL)
	return OpenAL::GetVelocity(AudioSourceRef, vVelocity);
#endif
	return FALSE;
}

Bool Platform::SetMinRange(TRefRef AudioSourceRef, const float fDistance)
{
#if(AUDIO_SYSTEM == AUDIO_OPENAL)
	return OpenAL::SetReferenceDistance(AudioSourceRef, fDistance);
#endif

	return FALSE;
}


Bool Platform::SetMaxRange(TRefRef AudioSourceRef, const float fDistance)
{
#if(AUDIO_SYSTEM == AUDIO_OPENAL)
	return OpenAL::SetMaxDistance(AudioSourceRef, fDistance);
#endif

	return FALSE;
}

Bool Platform::SetRateOfDecay(TRefRef AudioSourceRef, const float fRateOfDecay)
{
#if(AUDIO_SYSTEM == AUDIO_OPENAL)
	return OpenAL::SetRollOffFactor(AudioSourceRef, fRateOfDecay);
#endif

	return FALSE;
}

Bool Platform::AttachSourceToBuffer(TRefRef AudioSourceRef, TRefRef AudioAssetRef, Bool bStreaming)
{
#if(AUDIO_SYSTEM == AUDIO_OPENAL)

	// Get the buffer and source OpenAL ID's
	ALuint uBuffer, uSource;
	
	if(!OpenAL::GetBufferID(AudioAssetRef, uBuffer))
	{
		TLDebug_Print("Failed to find audio buffer ID");
		return FALSE;
	}
	
	if(!OpenAL::GetSourceID(AudioSourceRef, uSource))
	{
		TLDebug_Print("Failed to find audio source ID");
		return FALSE;
	}
	
	// Got both buffer and source ID
	// Now attach the source ot the buffer	
	return OpenAL::AttachSourceToBuffer(uSource, uBuffer, bStreaming);	
#endif

	return FALSE;
}


Bool Platform::DetermineFinishedAudio(TArray<TRef>& refArray)
{
#if(AUDIO_SYSTEM == AUDIO_OPENAL)
	return OpenAL::DetermineFinishedAudio(refArray);
#endif
	return FALSE;
}


void Platform::SetListener(const TListenerProperties& Props)
{
#if(AUDIO_SYSTEM == AUDIO_OPENAL)
	OpenAL::SetListener(Props);
#endif
}


Bool Platform::Enable()
{
#if(AUDIO_SYSTEM == AUDIO_OPENAL)
	return OpenAL::Enable();
#endif

	return FALSE;
}

Bool Platform::Disable()
{
#if(AUDIO_SYSTEM == AUDIO_OPENAL)
	return OpenAL::Disable();
#endif

	return FALSE;
}
