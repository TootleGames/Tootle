
#pragma once

#include <TootleCore/TLCore.h>
#include <TootleCore/TLTypes.h>
#include <TootleCore/TRef.h>

#include "../TAudioListener.h"
//#include "../TLAudio.h"

#include "../TAudioDistance.h"

#include <TootleAsset/TAudio.h>


// Different audio system types
#define AUDIO_DIRECTX		0
#define AUDIO_OPENAL		1

#define AUDIO_SYSTEM AUDIO_OPENAL

namespace TLAudio
{
	namespace Platform
	{
		SyncBool		Init();			
		SyncBool		Update();
		SyncBool		Shutdown();
		
		// Low level audio routines		
		Bool		CreateSource(TRefRef AudioSourceRef);
		Bool		RemoveSource(TRefRef AudioSourceRef);
		
		Bool		CreateBuffer(TLAsset::TAudio& AudioAsset);
		Bool		RemoveBuffer(TRefRef AudioAssetRef);
		
		Bool		HasSource(TRefRef AudioSourceRef);
		Bool		HasBuffer(TRefRef AudioAssetRef);
		
		Bool		AttachSourceToBuffer(TRefRef AudioSourceRef, TRefRef AudioAssetRef, Bool bStreaming);		
		
		//Audio control
		Bool		StartAudio(TRefRef AudioSourceRef);
		Bool		StopAudio(TRefRef AudioSourceRef);
		Bool		PauseAudio(TRefRef AudioSourceRef);

		Bool		DetermineFinishedAudio(TArray<TRef>& refArray);


		// Audio properties
		Bool		SetPitch(TRefRef AudioSourceRef, const float fPitch);
		Bool		GetPitch(TRefRef AudioSourceRef, float& fPitch);

		Bool		SetVolume(TRefRef AudioSourceRef, const float fVolume);
		Bool		GetVolume(TRefRef AudioSourceRef, float& fVolume);
		
		Bool		SetLooping(TRefRef AudioSourceRef, const Bool bLooping);
		Bool		GetIsLooping(TRefRef AudioSourceRef, Bool& bLooping);

		Bool		SetRelative(TRefRef AudioSourceRef, const Bool bRelative);
		Bool		GetIsRelative(TRefRef AudioSourceRef, Bool& bRelative);

		Bool		SetPosition(TRefRef AudioSourceRef, const float3 vPosition);
		Bool		GetPosition(TRefRef AudioSourceRef, float3& vPosition);

		Bool		SetVelocity(TRefRef AudioSourceRef, const float3 vVelocity);
		Bool		GetVelocity(TRefRef AudioSourceRef, float3& vVelocity);

		Bool		SetMinRange(TRefRef AudioSourceRef, const float fDistance);
		Bool		SetMaxRange(TRefRef AudioSourceRef, const float fDistance);
		Bool		SetRateOfDecay(TRefRef AudioSourceRef, const float fRateOfDecay);


		// Audio system listener (aka a virtual microphone)	
		void SetListener(const TListenerProperties& Props);

		Bool Enable();
		Bool Disable();
		
		Bool Activate();
		Bool Deactivate();
		
		Bool	SetDistanceModel(TLAudio::DistanceModel uDistanceModel);
		Bool	SetDopplerEffect(float fFactor, float fVelocity);				

	}
}





