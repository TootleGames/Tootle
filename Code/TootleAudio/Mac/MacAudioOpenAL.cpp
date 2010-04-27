#include "MacAudio.h"
#include "MacAudioOpenAL.h"

#include <TootleAsset/TAsset.h>
#include <TootleAsset/TAudio.h>


//#define AUDIO_DISABLED

// Define to enable some extra debug traces
#define ENABLE_AUDIO_TRACE

namespace TLAudio
{
	namespace Platform
	{
		namespace OpenAL
		{
			ALCcontext*					g_pContext = NULL;
			ALboolean					g_bEAX = FALSE;
						
			TArray<AudioObj> g_Sources;
			TArray<AudioObj> g_Buffers;
		}
	}
}


using namespace TLAudio;

SyncBool Platform::OpenAL::Init()
{
	//Enable();

	return SyncTrue;
}

SyncBool Platform::OpenAL::Update()
{
	if(g_pContext)
		alcMakeContextCurrent(g_pContext);

	return SyncTrue;
}

void Platform::OpenAL::SetListener(const TListenerProperties& Props)
{
#ifndef AUDIO_DISABLED
	
	// The orientation will use 6 consecutive floats which is the look at and up vectors
	alListenerfv(AL_ORIENTATION, (ALfloat*)&Props.m_vLookAt);

	// Set listener position
	alListenerfv(AL_POSITION, (ALfloat*)&Props.m_vPosition);

#ifdef ENABLE_AUDIO_TRACE
	TString	str = "Set Listener Pos: ";
	str.Appendf("(%.2f, %.2f, %.2f)", Props.m_vPosition.x, Props.m_vPosition.y, Props.m_vPosition.z);
	TLDebug_Print(str);
#endif

	// Set listener velocity
	alListenerfv(AL_VELOCITY, (ALfloat*)&Props.m_vVelocity);
	
#endif
}

Bool Platform::OpenAL::DetermineFinishedAudio(TArray<TRef>& refArray)
{
	if(g_Sources.GetSize() == 0)
		return FALSE;

	// Need to go through the list of audio objects and monitor for when they have finished.
	// Once finished remove the audio and add the ref to a list so it can be sent out as 
	// notification of the audio finishing
	for(u32 uIndex = 0; uIndex < g_Sources.GetSize(); uIndex++)
	{
		AudioObj& AO = g_Sources.ElementAt(uIndex);

		ALuint objid = AO.m_OpenALID;

		ALint state;
		alGetSourcei(objid, AL_SOURCE_STATE, &state);

		ALenum error;
		if ((error = alGetError()) != AL_NO_ERROR)
		{
			TString strerr = GetALErrorString(error);
			TLDebug_Break(strerr);
		}
		else
		{
			// Check the state
			if(state == AL_STOPPED)
			{
				refArray.Add(AO.m_AudioObjRef);
			}
		}
	}

	return (refArray.GetSize() > 0);
}


Bool Platform::OpenAL::CreateBuffer(TLAsset::TAudio& AudioAsset)
{
#ifdef AUDIO_DISABLED
	return FALSE;
#else
	// Test to see if we already have a buffer for the audio asset
	// If so return that
	AudioObj* pAO = g_Buffers.Find( AudioAsset.GetAssetRef() );
	
	if(pAO)
		return TRUE;
	
	ALuint uBufferID;
	
	// Create a new audio buffer	
	alGenBuffers(1, &uBufferID);
	
	ALenum error;
	if ((error = alGetError()) != AL_NO_ERROR)
	{
		TString strerr = GetALErrorString(error);
		TLDebug_Print(strerr);
		
		// Failed
		return FALSE;
	}
	
	
	/////////////////////////////////////////////////////////
	// Map the asset data into the audio buffer
	// Essentially this is a copy so not sure if we want to unload 
	// the asset afterwards or not?
	// We *may* end up wasting memory if not...
	/////////////////////////////////////////////////////////

	ALenum  format;

	// Determine format of data
	if(AudioAsset.GetNumberOfChannels() > 1)
		format = (AudioAsset.GetBitsPerSample()==16 ? AL_FORMAT_STEREO16 : AL_FORMAT_STEREO8 );
	else
		format = (AudioAsset.GetBitsPerSample()==16 ? AL_FORMAT_MONO16 : AL_FORMAT_MONO8 );

	// Audio data
	ALvoid* data = (ALvoid*)AudioAsset.RawAudioDataBinary().GetData();
	
	// Size of the audio data
	ALsizei size = AudioAsset.GetSize();	
	ALsizei freq = AudioAsset.GetSampleRate();
	
	alBufferData(uBufferID, format, data, size, freq);
	
	if ((error = alGetError()) != AL_NO_ERROR)
	{
		TString strerr = GetALErrorString(error);
		TLDebug_Break(strerr);
		
		// Failed
		return FALSE;
	}

 
	/////////////////////////////////////////////////////////

	
	
	// Success - add to the array
	AudioObj AO(AudioAsset.GetAssetRef(), uBufferID);
		
	g_Buffers.Add(AO);
	
#ifdef ENABLE_AUDIO_TRACE
	TLDebug_Print("Audio buffer created");
#endif
		
	return TRUE;
#endif
}


Bool Platform::OpenAL::ReleaseBuffer(TRefRef AudioAssetRef)
{
#ifdef AUDIO_DISABLED
	return TRUE;
#else

	s32 sIndex = g_Buffers.FindIndex(AudioAssetRef);

	if(sIndex == -1)
	{	
		TLDebug_Print("Failed to find audio buffer for release");
		return FALSE;
	}
	
	AudioObj& AO = g_Buffers.ElementAt(sIndex);

	alDeleteBuffers(1, &AO.m_OpenALID);

	ALenum error;
	if ((error = alGetError()) != AL_NO_ERROR)
	{
		TLDebug_Print("alDeleteBuffers error: ");

		TString strerr = GetALErrorString(error);
		TLDebug_Print(strerr);

		// Failed to remove a buffer that should exist!!!?
		TLDebug_Break("Failed to delete audio buffer");

		return FALSE;
	}
	
#ifdef ENABLE_AUDIO_TRACE
	TLDebug_Print("Removing audio buffer");
#endif
	
	// Remove the buffer from the array
	return g_Buffers.RemoveAt(sIndex);
#endif
}

Bool Platform::OpenAL::CreateSource(TRefRef AudioSourceRef)
{
	
#ifdef AUDIO_DISABLED
	return FALSE;

#else

	// Check to see if the source already exists and return that if so
	AudioObj* pAO = g_Sources.Find(AudioSourceRef);
	
	if(pAO)
		return TRUE;

	// Create the source
	ALuint uSourceID;
	alGenSources(1, &uSourceID);
	
	ALenum error;
	if ((error = alGetError()) != AL_NO_ERROR)
	{
		TString strerr = GetALErrorString(error);
		TLDebug_Break(strerr);
		
		// Failed
		return FALSE;
	}
	
	// Success add to the array
	AudioObj AO(AudioSourceRef, uSourceID);
	
	g_Sources.Add(AO);
	
#ifdef ENABLE_AUDIO_TRACE
	TLDebug_Print("Audio source created");
#endif
	
	return TRUE;
#endif
}


Bool Platform::OpenAL::AttachSourceToBuffer(ALuint& uSource, ALuint& uBuffer, const Bool bStreaming)
{
#ifdef AUDIO_DISABLED
	return FALSE;	
#else
	
	if(bStreaming)
		alSourceQueueBuffers(uSource, 1, &uBuffer);
	else
		alSourcei(uSource, AL_BUFFER, uBuffer);

	ALenum error;
	if ((error = alGetError()) != AL_NO_ERROR)
	{
		TLDebug_Print("Failed to attach source to buffer");
		TString strerr = GetALErrorString(error);
		TLDebug_Break(strerr);
		
		return FALSE;
	}
	
	// All done
	return TRUE;
#endif
}



Bool Platform::OpenAL::ReleaseSource(TRefRef AudioSourceRef)
{
#ifdef AUDIO_DISABLED
	return TRUE;	
#else
	
	s32 sIndex = g_Sources.FindIndex(AudioSourceRef);

	if(sIndex == -1)
	{
		TLDebug_Print("Failed to find audio source for release");
		return FALSE;
	}

	AudioObj& AO = g_Sources.ElementAt(sIndex);
	
	// Delete the source from the OpenAL system
	alDeleteSources(1, &AO.m_OpenALID);

	ALenum error;
	if ((error = alGetError()) != AL_NO_ERROR)
	{
		TLDebug_Print("Failed to delete source");
		TString strerr = GetALErrorString(error);
		TLDebug_Print(strerr);

		//NOTE: This WILL leave audio objects in memory
		TLDebug_Break("Audio Source Delete failed");
		return FALSE;
	}

#ifdef ENABLE_AUDIO_TRACE
	TLDebug_Print("Removing audio source");
#endif
	
	// Remove the source object from the array
	return g_Sources.RemoveAt(sIndex);	
#endif
}


SyncBool Platform::OpenAL::Shutdown()
{
	if(IsEnabled())
		Disable();

	return SyncTrue;
}


void Platform::OpenAL::RemoveAllSources()
{
	for(u32 uIndex = 0; uIndex < g_Sources.GetSize(); uIndex++)
	{
		AudioObj& AO = g_Sources.ElementAt(uIndex);

		alDeleteSources(1, &AO.m_OpenALID);

		ALenum error;
		if ((error = alGetError()) != AL_NO_ERROR)
		{
			TLDebug_Print("alDeleteSources error: ");

			TString strerr = GetALErrorString(error);
			TLDebug_Print(strerr);

			// Failed to remove a source that should exist!!!?
			TLDebug_Break("Failed to delete audio source");
		}
	}

	g_Sources.Empty(TRUE);
}

void Platform::OpenAL::RemoveAllBuffers()
{
	for(u32 uIndex = 0; uIndex < g_Buffers.GetSize(); uIndex++)
	{
		AudioObj& AO = g_Buffers.ElementAt(uIndex);

		alDeleteBuffers(1, &AO.m_OpenALID);

		ALenum error;
		if ((error = alGetError()) != AL_NO_ERROR)
		{
			TLDebug_Print("alDeleteBuffers error: ");

			TString strerr = GetALErrorString(error);
			TLDebug_Print(strerr);

			// Failed to remove a buffer that should exist!!!?
			TLDebug_Break("Failed to delete audio buffer");
		}
	}

	g_Buffers.Empty(TRUE);
}



Bool Platform::OpenAL::StartAudio(TRefRef AudioSourceRef)
{
	AudioObj* pAO = g_Sources.Find(AudioSourceRef);
	
	if(!pAO)
	{
		TLDebug_Print("Failed to find audio source for playback");
		return FALSE;
	}
			
	alSourcePlay(pAO->m_OpenALID);
	
	ALenum error;
	if ((error = alGetError()) != AL_NO_ERROR)
	{
		TLDebug_Print("alSourcePlay error: ");

		TString strerr = GetALErrorString(error);
		TLDebug_Break(strerr);

		return FALSE;
	}
	
	return TRUE;
}

Bool Platform::OpenAL::StopAudio(TRefRef AudioSourceRef)
{
	AudioObj* pAO = g_Sources.Find(AudioSourceRef);
	
	if(!pAO)
	{
		TLDebug_Print("Failed to find audio source for stop request");
		return FALSE;
	}
	
	alSourceStop(pAO->m_OpenALID);
	
	ALenum error;
	if ((error = alGetError()) != AL_NO_ERROR)
	{
		TLDebug_Print("alSourceStop error: ");
		
		TString strerr = GetALErrorString(error);
		TLDebug_Break(strerr);
		
		return FALSE;
	}
	
	return TRUE;
}	


Bool Platform::OpenAL::PauseAudio(TRefRef AudioSourceRef)
{
	AudioObj* pAO = g_Sources.Find(AudioSourceRef);
	
	if(!pAO)
	{
		TLDebug_Print("Failed to find audio source for pause request");
		return FALSE;
	}
	
	alSourcePause(pAO->m_OpenALID);
	
	ALenum error;
	if ((error = alGetError()) != AL_NO_ERROR)
	{
		TLDebug_Print("alSourcePause error: ");
		
		TString strerr = GetALErrorString(error);
		TLDebug_Break(strerr);
		
		return FALSE;
	}
	
	return TRUE;
}	


// Pitch manipulation
Bool Platform::OpenAL::SetPitch(TRefRef AudioSourceRef, const float fPitch)
{
	AudioObj* pAO = g_Sources.Find(AudioSourceRef);
	
	if(!pAO)
	{
		TLDebug_Print("Failed to find audio source for setpitch request");
		return FALSE;
	}
	
	alSourcef(pAO->m_OpenALID,AL_PITCH,fPitch);
	
	ALenum error;
	if ((error = alGetError()) != AL_NO_ERROR)
	{
		TLDebug_Print("setpitch alSourcef error: ");
		
		TString strerr = GetALErrorString(error);
		TLDebug_Break(strerr);
		
		return FALSE;
	}
	
	return TRUE;	
}


Bool Platform::OpenAL::GetPitch(TRefRef AudioSourceRef, float& fPitch)
{
	AudioObj* pAO = g_Sources.Find(AudioSourceRef);
	
	if(!pAO)
	{
		TLDebug_Print("Failed to find audio source for getpitch request");
		return FALSE;
	}
	
	// Attempt to get the value
	ALfloat fResult;
	alGetSourcef(pAO->m_OpenALID,AL_PITCH, &fResult);
	
	ALenum error;
	if ((error = alGetError()) != AL_NO_ERROR)
	{
		TLDebug_Print("getpitch alGetSourcef error: ");
		
		TString strerr = GetALErrorString(error);
		TLDebug_Break(strerr);
		
		return FALSE;
	}

	// Success - write the value to the variable
	fPitch = fResult;
	
	return TRUE;	
}


Bool Platform::OpenAL::SetVolume(TRefRef AudioSourceRef, const float fVolume)
{
	AudioObj* pAO = g_Sources.Find(AudioSourceRef);
	
	if(!pAO)
	{
		TLDebug_Print("Failed to find audio source for setvolume request");
		return FALSE;
	}
	
	alSourcef(pAO->m_OpenALID,AL_GAIN,fVolume);
	
	ALenum error;
	if ((error = alGetError()) != AL_NO_ERROR)
	{
		TLDebug_Print("setvolume alSourcef error: ");
		
		TString strerr = GetALErrorString(error);
		TLDebug_Break(strerr);
		
		return FALSE;
	}
	
	return TRUE;	
}


Bool Platform::OpenAL::GetVolume(TRefRef AudioSourceRef, float& fVolume)
{
	AudioObj* pAO = g_Sources.Find(AudioSourceRef);
	
	if(!pAO)
	{
		TLDebug_Print("Failed to find audio source for getvolume request");
		return FALSE;
	}
	
	// Attempt to get the value
	ALfloat fResult;
	alGetSourcef(pAO->m_OpenALID,AL_GAIN, &fResult);
	
	ALenum error;
	if ((error = alGetError()) != AL_NO_ERROR)
	{
		TLDebug_Print("getvolume alGetSourcef error: ");
		
		TString strerr = GetALErrorString(error);
		TLDebug_Break(strerr);
		
		return FALSE;
	}
	
	// Success - write the value to the variable
	fVolume = fResult;
	
	return TRUE;	
}


Bool Platform::OpenAL::SetLooping(TRefRef AudioSourceRef, const Bool bLooping)
{
	AudioObj* pAO = g_Sources.Find(AudioSourceRef);
	
	if(!pAO)
	{
		TLDebug_Print("Failed to find audio source for setlooping request");
		return FALSE;
	}
	
	alSourcei(pAO->m_OpenALID,AL_LOOPING,bLooping);
	
	ALenum error;
	if ((error = alGetError()) != AL_NO_ERROR)
	{
		TLDebug_Print("setlooping alSourcei error: ");
		
		TString strerr = GetALErrorString(error);
		TLDebug_Break(strerr);
		
		return FALSE;
	}
	
	return TRUE;	
}


Bool Platform::OpenAL::GetIsLooping(TRefRef AudioSourceRef, Bool& bLooping)
{
	AudioObj* pAO = g_Sources.Find(AudioSourceRef);
	
	if(!pAO)
	{
		TLDebug_Print("Failed to find audio source for getislooping request");
		return FALSE;
	}
	
	// Attempt to get the value
	ALint bResult;
	alGetSourcei(pAO->m_OpenALID,AL_LOOPING, &bResult);
	
	ALenum error;
	if ((error = alGetError()) != AL_NO_ERROR)
	{
		TLDebug_Print("getislooping alGetSourcef error: ");
		
		TString strerr = GetALErrorString(error);
		TLDebug_Break(strerr);
		
		return FALSE;
	}
	
	// Success - write the value to the variable
	bLooping = (bResult==1);
	
	return TRUE;	
}


Bool Platform::OpenAL::SetRelative(TRefRef AudioSourceRef, const Bool bRelative)
{
	AudioObj* pAO = g_Sources.Find(AudioSourceRef);
	
	if(!pAO)
	{
		TLDebug_Print("Failed to find audio source for setrelative request");
		return FALSE;
	}
	
	alSourcei(pAO->m_OpenALID,AL_SOURCE_RELATIVE,bRelative);
	
	ALenum error;
	if ((error = alGetError()) != AL_NO_ERROR)
	{
		TLDebug_Print("setrelative alSourcei error: ");
		
		TString strerr = GetALErrorString(error);
		TLDebug_Break(strerr);
		
		return FALSE;
	}
	
	return TRUE;	
}


Bool Platform::OpenAL::GetIsRelative(TRefRef AudioSourceRef, Bool& bRelative)
{
	AudioObj* pAO = g_Sources.Find(AudioSourceRef);
	
	if(!pAO)
	{
		TLDebug_Print("Failed to find audio source for getisrelative request");
		return FALSE;
	}
	
	// Attempt to get the value
	ALint bResult;
	alGetSourcei(pAO->m_OpenALID,AL_SOURCE_RELATIVE, &bResult);
	
	ALenum error;
	if ((error = alGetError()) != AL_NO_ERROR)
	{
		TLDebug_Print("getisrelative alGetSourcef error: ");
		
		TString strerr = GetALErrorString(error);
		TLDebug_Break(strerr);
		
		return FALSE;
	}
	
	// Success - write the value to the variable
	bRelative = (bResult==1);
	
	return TRUE;	
}


Bool Platform::OpenAL::SetPosition(TRefRef AudioSourceRef, const float3 vPosition)
{
	AudioObj* pAO = g_Sources.Find(AudioSourceRef);
	
	if(!pAO)
	{
		TLDebug_Print("Failed to find audio source for setposition request");
		return FALSE;
	}
	
	alSourcefv(pAO->m_OpenALID,AL_POSITION,vPosition);	
	
	ALenum error;
	if ((error = alGetError()) != AL_NO_ERROR)
	{
		TLDebug_Print("setposition alSourcefv error: ");
		
		TString strerr = GetALErrorString(error);
		TLDebug_Break(strerr);
		
		return FALSE;
	}

#ifdef ENABLE_AUDIO_TRACE
	TString	str = "Set Audio Pos: ";
	str.Appendf("%d (%.2f, %.2f, %.2f)", pAO->m_OpenALID, vPosition.x, vPosition.y, vPosition.z);
	TLDebug_Print(str);
#endif
		
	return TRUE;	
}


Bool Platform::OpenAL::GetPosition(TRefRef AudioSourceRef, float3& vPosition)
{
	AudioObj* pAO = g_Sources.Find(AudioSourceRef);
	
	if(!pAO)
	{
		TLDebug_Print("Failed to find audio source for getposition request");
		return FALSE;
	}
	
	// Attempt to get the values
	ALfloat vResult[3];
	alGetSourcefv(pAO->m_OpenALID,AL_POSITION,&vResult[0]);	
	
	ALenum error;
	if ((error = alGetError()) != AL_NO_ERROR)
	{
		TLDebug_Print("getposition alGetSourcefv error: ");
		
		TString strerr = GetALErrorString(error);
		TLDebug_Break(strerr);
		
		return FALSE;
	}
	
	// Success - write the values to the variable
	vPosition.Set(vResult[0], vResult[1], vResult[2]);
	
	return TRUE;	
}


Bool Platform::OpenAL::SetVelocity(TRefRef AudioSourceRef, const float3 vVelocity)
{
	AudioObj* pAO = g_Sources.Find(AudioSourceRef);
	
	if(!pAO)
	{
		TLDebug_Print("Failed to find audio source for setvelocity request");
		return FALSE;
	}
	
	alSourcefv(pAO->m_OpenALID,AL_VELOCITY,vVelocity);	
	
	ALenum error;
	if ((error = alGetError()) != AL_NO_ERROR)
	{
		TLDebug_Print("setvelocity alSourcefv error: ");
		
		TString strerr = GetALErrorString(error);
		TLDebug_Break(strerr);
		
		return FALSE;
	}
		
	return TRUE;	
}


Bool Platform::OpenAL::GetVelocity(TRefRef AudioSourceRef, float3& vVelocity)
{
	AudioObj* pAO = g_Sources.Find(AudioSourceRef);
	
	if(!pAO)
	{
		TLDebug_Print("Failed to find audio source for getvelocity request");
		return FALSE;
	}
	
	// Attempt to get the values
	ALfloat vResult[3];
	alGetSourcefv(pAO->m_OpenALID,AL_VELOCITY,&vResult[0]);	
	
	ALenum error;
	if ((error = alGetError()) != AL_NO_ERROR)
	{
		TLDebug_Print("getvelocity alGetSourcefv error: ");
		
		TString strerr = GetALErrorString(error);
		TLDebug_Break(strerr);
		
		return FALSE;
	}
	
	// Success - write the values to the variable
	vVelocity.Set(vResult[0], vResult[1], vResult[2]);
	
	return TRUE;	
}


Bool Platform::OpenAL::SetReferenceDistance(TRefRef AudioSourceRef, const float fDistance)
{
	AudioObj* pAO = g_Sources.Find(AudioSourceRef);
	
	if(!pAO)
	{
		TLDebug_Print("Failed to find audio source for setreferencedistance request");
		return FALSE;
	}
	
	alSourcef(pAO->m_OpenALID,AL_REFERENCE_DISTANCE, fDistance);	
	
	ALenum error;
	if ((error = alGetError()) != AL_NO_ERROR)
	{
		TLDebug_Print("setreferencedistance alSourcef error: ");
		
		TString strerr = GetALErrorString(error);
		TLDebug_Break(strerr);
		
		return FALSE;
	}
		
	return TRUE;	
}

Bool Platform::OpenAL::SetMaxDistance(TRefRef AudioSourceRef, const float fDistance)
{
	AudioObj* pAO = g_Sources.Find(AudioSourceRef);
	
	if(!pAO)
	{
		TLDebug_Print("Failed to find audio source for setmaxdistance request");
		return FALSE;
	}
	
	alSourcef(pAO->m_OpenALID,AL_MAX_DISTANCE, fDistance);	
	
	ALenum error;
	if ((error = alGetError()) != AL_NO_ERROR)
	{
		TLDebug_Print("setmaxdistance alSourcef error: ");
		
		TString strerr = GetALErrorString(error);
		TLDebug_Break(strerr);
		
		return FALSE;
	}
		
	return TRUE;	

}

Bool Platform::OpenAL::SetRollOffFactor(TRefRef AudioSourceRef, const float fRollOffFactor)
{
	AudioObj* pAO = g_Sources.Find(AudioSourceRef);
	
	if(!pAO)
	{
		TLDebug_Print("Failed to find audio source for setrollofffactor request");
		return FALSE;
	}
	
	alSourcef(pAO->m_OpenALID,AL_ROLLOFF_FACTOR, fRollOffFactor);	
	
	ALenum error;
	if ((error = alGetError()) != AL_NO_ERROR)
	{
		TLDebug_Print("setrollofffactor alSourcef error: ");
		
		TString strerr = GetALErrorString(error);
		TLDebug_Break(strerr);
		
		return FALSE;
	}
		
	return TRUE;	
}




Bool Platform::OpenAL::HasBuffer(TRefRef AudioAssetRef)
{
	AudioObj* pAO = g_Buffers.Find(AudioAssetRef);
	
	if(pAO)
		return TRUE;
	
	return FALSE;		
}

Bool Platform::OpenAL::HasSource(TRefRef AudioSourceRef)
{
	AudioObj* pAO = g_Sources.Find(AudioSourceRef);
	
	if(pAO)
		return TRUE;

	return FALSE;		
}


Bool Platform::OpenAL::GetBufferID(TRefRef AudioAssetRef, ALuint& buffer)
{
	AudioObj* pAO = g_Buffers.Find(AudioAssetRef);
	
	if(pAO)
	{
		buffer = pAO->m_OpenALID;
		return TRUE;
	}

	// Not found
	return FALSE;	
}


Bool Platform::OpenAL::GetSourceID(TRefRef AudioSourceRef, ALuint& source)
{
	AudioObj* pAO = g_Sources.Find(AudioSourceRef);
	
	if(pAO)
	{
		source = pAO->m_OpenALID;
		return TRUE;
	}
	
	// Not found
	return FALSE;	
}



TString Platform::OpenAL::GetALErrorString(ALenum err)
{
    switch(err)
    {
        case AL_NO_ERROR:
            return TString("AL_NO_ERROR");

        case AL_INVALID_NAME:
            return TString("AL_INVALID_NAME");
			
        case AL_INVALID_ENUM:
            return TString("AL_INVALID_ENUM");
			
        case AL_INVALID_VALUE:
            return TString("AL_INVALID_VALUE");
			
        case AL_INVALID_OPERATION:
            return TString("AL_INVALID_OPERATION");
			
        case AL_OUT_OF_MEMORY:
            return TString("AL_OUT_OF_MEMORY");			
    };
	
	// Unknown
	return TString("AL_UNKNOWN_ERROR");
}

TString Platform::OpenAL::GetALCErrorString(ALCenum err)
{
    switch(err)
    {
        case ALC_NO_ERROR:
            return TString("AL_NO_ERROR");
			
        case ALC_INVALID_DEVICE:
            return TString("ALC_INVALID_DEVICE");
			
        case ALC_INVALID_CONTEXT:
            return TString("ALC_INVALID_CONTEXT");
			
        case ALC_INVALID_ENUM:
            return TString("ALC_INVALID_ENUM");
			
        case ALC_INVALID_VALUE:
            return TString("ALC_INVALID_VALUE");
			
        case ALC_OUT_OF_MEMORY:
            return TString("ALC_OUT_OF_MEMORY");
    };
	
	// Unknown
	return TString("AL_UNKNOWN_ERROR");
}


Bool Platform::OpenAL::IsEnabled()
{
	return (g_pContext != NULL);
}


Bool Platform::OpenAL::Enable()
{
	// Context exists therefore already enabled
	if(g_pContext)
		return TRUE;

	ALCenum alcerror;
	ALenum alerror;

	TLDebug_Print("Checking persistent OpenAL errors");
	// [27/04/10] DB - not sure why but the OpenAL system sometimes has errors from previous instances
	// which alGetError will remove when it's called.
	// Check for errors still in the system
	if ((alerror = alGetError()) != AL_NO_ERROR)
	{
		TString strerr = GetALErrorString(alerror);
		TLDebug_Print(strerr);
	}
	else 
	{
		TLDebug_Print("No persistent OpenAL errors");
		
	}

	
	
	// Initialization
	ALCdevice* pDevice = alcOpenDevice(NULL); // select the "preferred device"

	if(!pDevice)
	{
		TLDebug_Print("Unable to create OpenAL device");
		return FALSE;
	}

	const ALCchar* actualDeviceName = alcGetString(pDevice, ALC_DEVICE_SPECIFIER);
	
	TTempString devicename("OpenAL Audio Device: ");
	devicename.Appendf("%s", actualDeviceName);
	TLDebug_Print("OpenAL Audio Device opened successfully");
	TLDebug_Print(devicename);

	g_pContext = alcCreateContext(pDevice,NULL);

	// If the audio fails to create a context then something is wrong and we need to bail out
	if(g_pContext == NULL)
	{
		TLDebug_Print("Unable to create OpenAL context");

		
		if((alcerror = alcGetError(pDevice)) != AL_NO_ERROR)
		{
			TString strerr = GetALCErrorString(alcerror);
			TLDebug_Print(strerr);
		}
		
		// Destroy the device
		alcCloseDevice(pDevice);

		return FALSE;
	}

	
	ALCboolean bSuccess = alcMakeContextCurrent(g_pContext);

	
	// Failed?
	if(bSuccess == ALC_FALSE)
	{
		TLDebug_Print("Faied to set OpenAL context");

		//TString strerr = GetALCErrorString(alcerror);
		//TLDebug_Print(strerr);

		alcMakeContextCurrent(NULL);
		alcDestroyContext(g_pContext);
		g_pContext = NULL;
		alcCloseDevice(pDevice);

		return FALSE;
	}
	
	
	//TODO: Test to see if extensions are supported
	
	// Check for EAX 2.0 support
	Platform::OpenAL::g_bEAX = alIsExtensionPresent("EAX2.0");
	
	if ((alerror = alGetError()) != AL_NO_ERROR)
	{
		TTempString str("OpenAL Error: ");
		TTempString errstr = GetALErrorString(alerror);
		str << errstr;
		TLDebug_Print(str);
		TLDebug_Print("Faied to check EAX2.0");
	}
	

	//Set the default distance model to use
	//SetDistanceModel(Type2D);
	//SetDistanceModel(Type3DLinearClamped);
	
	// Setup doppler shift
	alDopplerFactor(1.0f);			// Set to 0.0f to switch off the doppler effect
	alDopplerVelocity(1.0f);

	/*
	alEnable(ALC_CONVERT_DATA_UPON_LOADING);

	if ((error = alGetError()) != AL_NO_ERROR)
	{
		TLDebug_Print("Faied to set convert data parameter");
	}

	// Set low quality audio rendering
	alSetInteger(ALC_SPATIAL_RENDERING_QUALITY, ALC_SPATIAL_RENDERING_QUALITY_LOW);

	if ((error = alGetError()) != AL_NO_ERROR)
	{
		TLDebug_Print("Faied to set spatial quality");
	}

	alSetInteger(ALC_RENDER_CHANNEL_COUNT, ALC_RENDER_CHANNEL_COUNT_STEREO);

	if ((error = alGetError()) != AL_NO_ERROR)
	{
		TLDebug_Print("Faied to set channel count");
	}
	*/


	/*
	 // Done via the audiograph when the system is enabled
	ALfloat data[3] = {0,0,0};
	// Set the listener
	alListenerfv(AL_POSITION,    data);
	
	if ((alerror = alGetError()) != AL_NO_ERROR)
	{
		TString strerr = GetALErrorString(alerror);
		TLDebug_Break(strerr);
	}
	
    alListenerfv(AL_VELOCITY,    data);
   
	// Orientation uses 6 consecutive floats
	//alListenerfv(AL_ORIENTATION, data);
	*/

	if ((alerror = alGetError()) != AL_NO_ERROR)
	{
		TString strerr = GetALErrorString(alerror);
		TLDebug_Break(strerr);
	}
	
	if ((alcerror = alcGetError(pDevice)) != ALC_NO_ERROR)
	{
		TString strerr = GetALCErrorString(alcerror);
		TLDebug_Break(strerr);
	}
	
		
	return TRUE;
}


Bool Platform::OpenAL::SetDistanceModel(TLAudio::DistanceModel uDistanceModel)
{
	if(!g_pContext)
		return FALSE;
	
	if(uDistanceModel == Type2D)
	{
		alDistanceModel(AL_NONE);
	}
	else if(uDistanceModel == Type3DLinear)
	{
		alDistanceModel(AL_LINEAR_DISTANCE);
	}
	else if(uDistanceModel == Type3DLinearClamped)
	{
		alDistanceModel(AL_LINEAR_DISTANCE_CLAMPED);
	}
	else if(uDistanceModel == Type3DInverse)
	{
		alDistanceModel(AL_INVERSE_DISTANCE);
	}
	else if(uDistanceModel == Type3DInverseClamped)
	{
		alDistanceModel(AL_INVERSE_DISTANCE_CLAMPED);
	}

	else 
	{
		TLDebug_Break("Invalid distance model");
	}

	ALenum error;
	
	if ((error = alGetError()) != AL_NO_ERROR)
	{
		TString strerr = GetALErrorString(error);
		TLDebug_Break(strerr);
		
		return FALSE;
	}
	
	return TRUE;
	
}


Bool Platform::OpenAL::SetDopplerEffect(float fFactor, float fVelocity)
{
	if(!g_pContext)
		return FALSE;
	
	ALenum error;
	
	alDopplerFactor(fFactor);			// Set to 0.0f to switch off the doppler effect
	
	if ((error = alGetError()) != AL_NO_ERROR)
	{
		TString strerr = GetALErrorString(error);
		TLDebug_Break(strerr);
		
		return FALSE;
	}
	
	alDopplerVelocity(fVelocity);
	
	if ((error = alGetError()) != AL_NO_ERROR)
	{
		TString strerr = GetALErrorString(error);
		TLDebug_Break(strerr);
		
		return FALSE;
	}
	
	return TRUE;
}

Bool Platform::OpenAL::Disable()
{	
	// TODO: The removal of the sources will be done when the nodes shutdown
	//		 The removal of the buffers will be done when the file assets are shutdown
	
	// Delete the Sources
	RemoveAllSources();
	
	// Delete the Buffers	
	RemoveAllBuffers();
	
	if(g_pContext)
	{
		alcMakeContextCurrent(NULL);
		
		ALCdevice* pDevice = alcGetContextsDevice(g_pContext);

		alcDestroyContext(g_pContext);
		g_pContext = NULL;
	
		if(pDevice)
		{
			const ALCchar* actualDeviceName = alcGetString(pDevice, ALC_DEVICE_SPECIFIER);
			
			TTempString devicename("Closing OpenAL Audio Device: ");
			devicename.Appendf("%s", actualDeviceName);
			TLDebug_Print(devicename);
			
			alcCloseDevice(pDevice);
			TLDebug_Print("OpenAL Audio Device closed successfully");

		}
	}


	return TRUE;
}

Bool Platform::OpenAL::Activate()
{
	if(g_pContext)
	{
		ALCdevice* pDevice = alcGetContextsDevice(g_pContext);
		
		if(pDevice)
		{
			alcProcessContext(g_pContext);
			
			// Success?
			ALCenum error;
			if ((error = alcGetError(pDevice)) == ALC_NO_ERROR)
				return TRUE;
		}
	}
	
	return FALSE;
}

Bool Platform::OpenAL::Deactivate()
{
	if(g_pContext)
	{
		ALCdevice* pDevice = alcGetContextsDevice(g_pContext);
		
		if(pDevice)
		{
			alcSuspendContext(g_pContext);
			
			ALCenum error;
			if ((error = alcGetError(pDevice)) == ALC_NO_ERROR)
				return TRUE;
		}
	}
	
	return FALSE;
	
}
