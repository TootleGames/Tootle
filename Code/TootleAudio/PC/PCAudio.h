
#pragma once

#include <TootleCore/TLCore.h>

#include "../TLAudio.h"

// DirectX audio include
#include <xaudio2.h>	

// OpenAL audio include
#include <al.h>			
#include <alc.h>			

// Different audio system types
#define AUDIO_DIRECTX		0
#define AUDIO_OPENAL		1

#define AUDIO_SYSTEM AUDIO_OPENAL

//#if(AUDIO_SYSTEM == AUDIO_DIRECTX)

// DirectX libraries
//#pragma comment( lib, "xaudio2.lib" )
#pragma comment( lib, "ole32.lib" )

//#else if(AUDIO_SYSTEM == AUDIO_OPENAL)

// OpenAL libraries
#pragma comment( lib, "openal32.lib")	

//#endif

namespace TLAudio
{
	namespace Platform
	{
		namespace DirectX
		{
			SyncBool		Init();
			SyncBool		Update();
			SyncBool		Shutdown();

			// Low level audio requests - may be moved onto a low level audio manager at
			// som stage
			Bool			StartAudio();
			Bool			StopAudio();

			// Audio engine callback handling - may get moved onto a low level audio manager at some stage
			class TAudioEngineCallback;
			extern TPtr<TAudioEngineCallback> g_pAudioEngineCallback;
		}

		namespace OpenAL
		{
			class AudioObj
			{
			public:
				TRef	m_AudioObjRef;
				ALuint	m_OpenALID;
				
				inline Bool					operator==(TRefRef AudioObjRef)const		{	return m_AudioObjRef == AudioObjRef;	}
			};
			
			
			SyncBool		Init();
			SyncBool		Update();
			SyncBool		Shutdown();

			void			RemoveAllSources();
			void			RemoveAllBuffers();

			Bool			DetermineFinishedAudio(TArray<TRef>& refArray);

			// Low level audio routines
			// NOTE: May be moved onto a low level audio manager at some stage
			Bool			StartAudio(TRefRef AudioSourceRef);
			Bool			StopAudio(TRefRef AudioSourceRef);
			Bool			PauseAudio(TRefRef AudioSourceRef);
			
			
			TPtr<AudioObj> CreateBuffer(TRefRef AudioAssetRef);
			Bool ReleaseBuffer(TRefRef AudioAssetRef);
			
			TPtr<AudioObj> CreateSource(TRefRef AudioSourceRef);
			Bool ReleaseSource(TRefRef AudioSourceRef);
			
			Bool AttachSourceToBuffer(ALuint& uSource, ALuint& uBuffer, const Bool bStreaming);
			
			// Buffer property access
			
			
			// Source Property manipulation
			Bool	SetPitch(TRefRef AudioSourceRef, const float fPitch);
			Bool	GetPitch(TRefRef AudioSourceRef, float& fPitch);
			
			Bool	SetPosition(TRefRef AudioSourceRef, const float3 vPosition);
			Bool	GetPosition(TRefRef AudioSourceRef, float3& vPosition);

			/*
			Could do with a nice way to set the properties via the correct routine
			 float	- alSourcef
			 float3 - alSourcefv or alSource3f
			 int	- alSourcei
			Bool	SetSourceProperty(TRefRef AudioSourceRef)
			 {
			 }
			 
			 //Same for the get property type routines

			*/



			// Error routines
			TString			GetALErrorString(ALenum err);	
			TString			GetALCErrorString(ALCenum err);
		}

		SyncBool		Init();			
		SyncBool		Update();
		SyncBool		Shutdown();
		
		// Low level audio routines		
		Bool		CreateSource(TRefRef AudioSourceRef);
		Bool		RemoveSource(TRefRef AudioSourceRef);
		
		Bool		CreateBuffer(TRefRef AudioAssetRef);
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


		// OpenAL access
		Bool		GetBufferID(TRefRef AudioAssetRef, ALuint& buffer);
		Bool		GetSourceID(TRefRef AudioSourceRef, ALuint& source);
	}
}


// DirectX audio engine callback
class TLAudio::Platform::DirectX::TAudioEngineCallback : public IXAudio2EngineCallback
{
    void STDMETHODCALLTYPE OnProcessingPassEnd () {}
    void STDMETHODCALLTYPE OnProcessingPassStart() {}
    void STDMETHODCALLTYPE OnCriticalError (HRESULT Error) {}
};





