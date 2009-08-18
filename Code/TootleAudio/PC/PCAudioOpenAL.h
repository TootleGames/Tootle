
#pragma once

// OpenAL audio include
#include <al.h>			
#include <alc.h>			

// OpenAL libraries
#pragma comment( lib, "openal32.lib")	


namespace TLAudio
{
	namespace Platform
	{
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
			
			
			Bool CreateBuffer(TLAsset::TAudio& AudioAsset);
			Bool ReleaseBuffer(TRefRef AudioAssetRef);
			
			Bool CreateSource(TRefRef AudioSourceRef);
			Bool ReleaseSource(TRefRef AudioSourceRef);
			
			Bool AttachSourceToBuffer(ALuint& uSource, ALuint& uBuffer, const Bool bStreaming);
			
			// Buffer property access
			
			
			// Source Property manipulation
			Bool	SetPitch(TRefRef AudioSourceRef, const float fPitch);
			Bool	GetPitch(TRefRef AudioSourceRef, float& fPitch);

			Bool	SetVolume(TRefRef AudioSourceRef, const float fVolume);
			Bool	GetVolume(TRefRef AudioSourceRef, float& fVolume);

			Bool	SetLooping(TRefRef AudioSourceRef, const Bool bLooping);
			Bool	GetIsLooping(TRefRef AudioSourceRef, Bool& bLooping);

			Bool	SetRelative(TRefRef AudioSourceRef, const Bool bRelative);
			Bool	GetIsRelative(TRefRef AudioSourceRef, Bool& bRelative);

			Bool	SetPosition(TRefRef AudioSourceRef, const float3 vPosition);
			Bool	GetPosition(TRefRef AudioSourceRef, float3& vPosition);

			Bool	SetVelocity(TRefRef AudioSourceRef, const float3 vPosition);
			Bool	GetVelocity(TRefRef AudioSourceRef, float3& vPosition);

			Bool	SetReferenceDistance(TRefRef AudioSourceRef, const float fDistance);
			Bool	SetMaxDistance(TRefRef AudioSourceRef, const float fDistance);
			Bool	SetRollOffFactor(TRefRef AudioSourceRef, const float fRollOffFactor);

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

			Bool		HasSource(TRefRef AudioSourceRef);
			Bool		HasBuffer(TRefRef AudioAssetRef);

			// Buffer/Source ID access
			Bool		GetBufferID(TRefRef AudioAssetRef, ALuint& buffer);
			Bool		GetSourceID(TRefRef AudioSourceRef, ALuint& source);


			// Error routines
			TString			GetALErrorString(ALenum err);	
			TString			GetALCErrorString(ALCenum err);

			// Set listener details - in OpenAL there is only ever one listener
			void SetListener(const TListenerProperties& Props);

			Bool Enable();
			Bool Disable();
		}
	}
}


