
#include "PCAudio.h"
#include "PCAudioOpenAL.h"

#include <TootleCore/TPtrArray.h>

namespace TLAudio
{
	namespace Platform
	{
		namespace OpenAL
		{
			ALboolean					g_bEAX = FALSE;
						
			TPtrArray<AudioObj> g_Sources;
			TPtrArray<AudioObj> g_Buffers;
		}
	}
}

// Define to enable some extra debug traces
//#define ENABLE_AUDIO_TRACE

using namespace TLAudio;

SyncBool Platform::OpenAL::Init()
{
	// Initialization
	ALCdevice* pDevice = alcOpenDevice(NULL); // select the "preferred device"

	if(!pDevice)
	{
		TLDebug_Print("Unable to create OpenAL device");
		return SyncFalse;
	}


	ALCcontext* pContext = alcCreateContext(pDevice,NULL);

	if(pContext == NULL)
	{
		TLDebug_Print("Unable to create OpenAL context");
		
		ALCenum alcerror;

		if((alcerror = alcGetError(pDevice)) != AL_NO_ERROR)
		{
			TString strerr = GetALCErrorString(alcerror);
			TLDebug_Print(strerr);
		}
		
		// Destroy the device
		alcCloseDevice(pDevice);

		return SyncFalse;
	}

	
	ALCboolean bSuccess = alcMakeContextCurrent(pContext);

	
	// Failed?
	if(bSuccess == ALC_FALSE)
	{
		TLDebug_Print("Faied to set OpenAL context");

		//TString strerr = GetALCErrorString(alcerror);
		//TLDebug_Print(strerr);

		alcMakeContextCurrent(NULL);
		alcDestroyContext(pContext);
		alcCloseDevice(pDevice);

		return SyncFalse;
	}
	
	// Check for EAX 2.0 support
	Platform::OpenAL::g_bEAX = alIsExtensionPresent("EAX2.0");
	
	ALenum error;

	if ((error = alGetError()) != AL_NO_ERROR)
	{
		TLDebug_Print("Faied to check EAX2.0");
		
		alcMakeContextCurrent(NULL);
		alcDestroyContext(pContext);
		alcCloseDevice(pDevice);

		return SyncFalse;
	}

	//Set the default distance model to use
	//alDistanceModel(AL_NONE);
	alDistanceModel(AL_INVERSE_DISTANCE_CLAMPED);
	//alDistanceModel(AL_INVERSE_DISTANCE);
	//alDistanceModel(AL_LINEAR_DISTANCE);
	
	// Setup doppler shift
	alDopplerFactor(1.0f);			// Set to 0.0f to switch off the doppler effect
	alDopplerVelocity(1000.0f);

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


	ALfloat data[3] = {0,0,0};
	// Set the listener
	alListenerfv(AL_POSITION,    data);
    alListenerfv(AL_VELOCITY,    data);
   
	// Orientation uses 6 consecutive floats
	//alListenerfv(AL_ORIENTATION, data);

	return SyncTrue;
}

SyncBool Platform::OpenAL::Update()
{
	return SyncTrue;
}

void Platform::OpenAL::SetListener(const TListenerProperties& Props)
{
	// The orientation will use 6 consecutive floats which is the look at and up vectors
	alListenerfv(AL_ORIENTATION, (ALfloat*)&Props.m_vLookAt);

	float3 vPos;

	vPos.x = -Props.m_vPosition.x;
	vPos.y = -Props.m_vPosition.y;
	vPos.z = 5.0f;

	// Set listener position
	alListenerfv(AL_POSITION, (ALfloat*)&vPos);

#ifdef ENABLE_AUDIO_TRACE
	TString	str = "Set Listener Pos: ";
	str.Appendf("(%.2f, %.2f, %.2f)", vPos.x, vPos.y, vPos.z);
	TLDebug_Print(str);
#endif

	// Set listener velocity
	alListenerfv(AL_VELOCITY, (ALfloat*)&Props.m_vVelocity);
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
		TPtr<AudioObj> pAO = g_Sources.ElementAt(uIndex);

		ALuint objid = pAO->m_OpenALID;

		ALint state;
		alGetSourcei(objid, AL_SOURCE_STATE, &state);

		ALenum error;
		if ((error = alGetError()) != AL_NO_ERROR)
		{
			TString strerr = GetALErrorString(error);
			TLDebug_Print(strerr);
		}
		else
		{
			// Check the state
			if(state == AL_STOPPED)
			{
				refArray.Add(pAO->m_AudioObjRef);
			}
		}
	}

	return (refArray.GetSize() > 0);
}


TPtr<Platform::OpenAL::AudioObj> Platform::OpenAL::CreateBuffer(TRefRef AudioAssetRef)
{
	// Test to see if we already have a buffer for the audio asset
	// If so return that
	TPtr<AudioObj> pAO = g_Buffers.FindPtr(AudioAssetRef);
	
	if(pAO)
		return pAO;
	
	ALuint uBufferID;
	
	// Create a new audio buffer	
	alGenBuffers(1, &uBufferID);
	
	ALenum error;
	if ((error = alGetError()) != AL_NO_ERROR)
	{
		TString strerr = GetALErrorString(error);
		TLDebug_Print(strerr);
		
		// Failed
		return TPtr<AudioObj>(NULL);
	}
	
	
	/////////////////////////////////////////////////////////
	// Map the asset data into the audio buffer
	// Essentially this is a copy so not sure if we want to unload 
	// the asset afterwards or not?
	// We *may* end up wasting memory if not...
	/////////////////////////////////////////////////////////

	// Get the audio details from the asset
	TPtr<TLAsset::TAsset> pAsset = TLAsset::GetAsset(AudioAssetRef);
	
	if(!pAsset.IsValid())
	{
		TLDebug_Print("Failed to find audio asset for buffer creation");
		return TPtr<AudioObj>(NULL);
	}
	
	TLAsset::TAudio* pAudioAsset = static_cast<TLAsset::TAudio*>(pAsset.GetObject());

	ALenum  format;

	// Determine format of data
	if(pAudioAsset->GetNumberOfChannels() > 1)
		format = (pAudioAsset->GetBitsPerSample()==16 ? AL_FORMAT_STEREO16 : AL_FORMAT_STEREO8 );
	else
		format = (pAudioAsset->GetBitsPerSample()==16 ? AL_FORMAT_MONO16 : AL_FORMAT_MONO8 );

	// Audio data
	ALvoid* data = (ALvoid*) pAudioAsset->RawAudioDataBinary().GetData();
	
	// Size of the audio data
	ALsizei size = pAudioAsset->GetSize();	
	ALsizei freq = pAudioAsset->GetSampleRate();
	
	alBufferData(uBufferID, format, data, size, freq);
	
	if ((error = alGetError()) != AL_NO_ERROR)
	{
		TString strerr = GetALErrorString(error);
		TLDebug_Print(strerr);
		
		// Failed
		return TPtr<AudioObj>(NULL);
	}

 
	/////////////////////////////////////////////////////////

	
	
	// Success - add to the array
	pAO = new AudioObj;
	
	if(pAO)
	{
		pAO->m_AudioObjRef = AudioAssetRef;
		pAO->m_OpenALID = uBufferID;
		
		g_Buffers.Add(pAO);
	}
	
	return pAO;
}


Bool Platform::OpenAL::ReleaseBuffer(TRefRef AudioAssetRef)
{
	
	TPtr<AudioObj> pAO = g_Buffers.FindPtr(AudioAssetRef);
	
	if(!pAO)
	{
		TLDebug_Print("Failed to find audio buffer for release");
		return FALSE;
	}
	
	// Remove the buffer from the array
	return g_Buffers.Remove(pAO);
}

TPtr<Platform::OpenAL::AudioObj> Platform::OpenAL::CreateSource(TRefRef AudioSourceRef)
{
	// Check to see if the source already exists and return that if so
	TPtr<AudioObj> pAO = g_Sources.FindPtr(AudioSourceRef);
	
	if(pAO)
		return pAO;

	// Create the source
	ALuint uSourceID;
	alGenSources(1, &uSourceID);
	
	ALenum error;
	if ((error = alGetError()) != AL_NO_ERROR)
	{
		TString strerr = GetALErrorString(error);
		TLDebug_Print(strerr);
		
		// Failed
		return TPtr<AudioObj>(NULL);
	}
	
	// Success add to the array
	pAO = new AudioObj;
	
	if(pAO)
	{
		pAO->m_AudioObjRef = AudioSourceRef;
		pAO->m_OpenALID = uSourceID;

		g_Sources.Add(pAO);
	}
	
	return pAO;
}


Bool Platform::OpenAL::AttachSourceToBuffer(ALuint& uSource, ALuint& uBuffer, const Bool bStreaming)
{
	if(bStreaming)
		alSourceQueueBuffers(uSource, 1, &uBuffer);
	else
		alSourcei(uSource, AL_BUFFER, uBuffer);

	ALenum error;
	if ((error = alGetError()) != AL_NO_ERROR)
	{
		TLDebug_Print("Failed to attach source to buffer");
		TString strerr = GetALErrorString(error);
		TLDebug_Print(strerr);
		
		return FALSE;
	}
	
	// All done
	return TRUE;
}



Bool Platform::OpenAL::ReleaseSource(TRefRef AudioSourceRef)
{
	TPtr<AudioObj> pAO = g_Sources.FindPtr(AudioSourceRef);
	
	if(!pAO)
	{
		TLDebug_Print("Failed to find audio source for release");
		return FALSE;
	}

	// Delete the source from the OpenAL system
	alDeleteSources(1, &pAO->m_OpenALID);

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

	// Remove the source object from the array
	return g_Sources.Remove(pAO);	
}


SyncBool Platform::OpenAL::Shutdown()
{
	
	// TODO: The removal of the sources will be done when the nodes shutdown
	//		 The removal of the buffers will be done when the file assets are shutdown
	
	// Delete the Sources
	RemoveAllSources();
	
	// Delete the Buffers	
	RemoveAllBuffers();
	
	ALCcontext* pContext = alcGetCurrentContext();

	ALCdevice* pDevice = alcGetContextsDevice(pContext);

	alcMakeContextCurrent(NULL);

	alcDestroyContext(pContext);
	alcCloseDevice(pDevice);

/*
	// Delete the callback handler
	if(Platform::g_pAudioEngineCallback)
	{
		Platform::g_pAudioEngineCallback = NULL;
	}
*/

	return SyncTrue;
}


void Platform::OpenAL::RemoveAllSources()
{
	for(u32 uIndex = 0; uIndex < g_Sources.GetSize(); uIndex++)
	{
		TPtr<AudioObj> pAO = g_Sources.ElementAt(uIndex);

		alDeleteSources(1, &pAO->m_OpenALID);

		ALenum error;
		if ((error = alGetError()) != AL_NO_ERROR)
		{
			TLDebug_Print("alDeleteSources error: ");

			TString strerr = GetALErrorString(error);
			TLDebug_Print(strerr);

			// Failed to remove a source that should exist!!!?
			TLDebug_Break("Failed to delete audio source");
		}

		// Delete the array element
		g_Sources.ElementAt(uIndex) = NULL;
	}

	g_Sources.Empty(TRUE);
}

void Platform::OpenAL::RemoveAllBuffers()
{
	for(u32 uIndex = 0; uIndex < g_Buffers.GetSize(); uIndex++)
	{
		TPtr<AudioObj> pAO = g_Buffers.ElementAt(uIndex);

		alDeleteBuffers(1, &pAO->m_OpenALID);

		ALenum error;
		if ((error = alGetError()) != AL_NO_ERROR)
		{
			TLDebug_Print("alDeleteBuffers error: ");

			TString strerr = GetALErrorString(error);
			TLDebug_Print(strerr);

			// Failed to remove a buffer that should exist!!!?
			TLDebug_Break("Failed to delete audio buffer");
		}

		// Delete the array element
		g_Buffers.ElementAt(uIndex) = NULL;
	}

	g_Buffers.Empty(TRUE);
}



Bool Platform::OpenAL::StartAudio(TRefRef AudioSourceRef)
{
	TPtr<AudioObj> pAO = g_Sources.FindPtr(AudioSourceRef);
	
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
		TLDebug_Print(strerr);

		return FALSE;
	}
	
	return TRUE;
}

Bool Platform::OpenAL::StopAudio(TRefRef AudioSourceRef)
{
	TPtr<AudioObj> pAO = g_Sources.FindPtr(AudioSourceRef);
	
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
		TLDebug_Print(strerr);
		
		return FALSE;
	}
	
	return TRUE;
}	


Bool Platform::OpenAL::PauseAudio(TRefRef AudioSourceRef)
{
	TPtr<AudioObj> pAO = g_Sources.FindPtr(AudioSourceRef);
	
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
		TLDebug_Print(strerr);
		
		return FALSE;
	}
	
	return TRUE;
}	


// Pitch manipulation
Bool Platform::OpenAL::SetPitch(TRefRef AudioSourceRef, const float fPitch)
{
	TPtr<AudioObj> pAO = g_Sources.FindPtr(AudioSourceRef);
	
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
		TLDebug_Print(strerr);
		
		return FALSE;
	}
	
	return TRUE;	
}


Bool Platform::OpenAL::GetPitch(TRefRef AudioSourceRef, float& fPitch)
{
	TPtr<AudioObj> pAO = g_Sources.FindPtr(AudioSourceRef);
	
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
		TLDebug_Print(strerr);
		
		return FALSE;
	}

	// Success - write the value to the variable
	fPitch = fResult;
	
	return TRUE;	
}


Bool Platform::OpenAL::SetVolume(TRefRef AudioSourceRef, const float fVolume)
{
	TPtr<AudioObj> pAO = g_Sources.FindPtr(AudioSourceRef);
	
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
		TLDebug_Print(strerr);
		
		return FALSE;
	}
	
	return TRUE;	
}


Bool Platform::OpenAL::GetVolume(TRefRef AudioSourceRef, float& fVolume)
{
	TPtr<AudioObj> pAO = g_Sources.FindPtr(AudioSourceRef);
	
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
		TLDebug_Print(strerr);
		
		return FALSE;
	}
	
	// Success - write the value to the variable
	fVolume = fResult;
	
	return TRUE;	
}


Bool Platform::OpenAL::SetLooping(TRefRef AudioSourceRef, const Bool bLooping)
{
	TPtr<AudioObj> pAO = g_Sources.FindPtr(AudioSourceRef);
	
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
		TLDebug_Print(strerr);
		
		return FALSE;
	}
	
	return TRUE;	
}


Bool Platform::OpenAL::GetIsLooping(TRefRef AudioSourceRef, Bool& bLooping)
{
	TPtr<AudioObj> pAO = g_Sources.FindPtr(AudioSourceRef);
	
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
		TLDebug_Print(strerr);
		
		return FALSE;
	}
	
	// Success - write the value to the variable
	//	gr: compiler says performance warning, so changed
	//bLooping = (Bool)bResult;
	bLooping = (bResult==1);
	
	return TRUE;	
}



Bool Platform::OpenAL::SetPosition(TRefRef AudioSourceRef, const float3 vPosition)
{
	TPtr<AudioObj> pAO = g_Sources.FindPtr(AudioSourceRef);
	
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
		TLDebug_Print(strerr);
		
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
	TPtr<AudioObj> pAO = g_Sources.FindPtr(AudioSourceRef);
	
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
		TLDebug_Print(strerr);
		
		return FALSE;
	}
	
	// Success - write the values to the variable
	vPosition.Set(vResult[0], vResult[1], vResult[2]);
	
	return TRUE;	
}


Bool Platform::OpenAL::SetVelocity(TRefRef AudioSourceRef, const float3 vVelocity)
{
	TPtr<AudioObj> pAO = g_Sources.FindPtr(AudioSourceRef);
	
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
		TLDebug_Print(strerr);
		
		return FALSE;
	}
		
	return TRUE;	
}


Bool Platform::OpenAL::GetVelocity(TRefRef AudioSourceRef, float3& vVelocity)
{
	TPtr<AudioObj> pAO = g_Sources.FindPtr(AudioSourceRef);
	
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
		TLDebug_Print(strerr);
		
		return FALSE;
	}
	
	// Success - write the values to the variable
	vVelocity.Set(vResult[0], vResult[1], vResult[2]);
	
	return TRUE;	
}


Bool Platform::OpenAL::SetReferenceDistance(TRefRef AudioSourceRef, const float fDistance)
{
	TPtr<AudioObj> pAO = g_Sources.FindPtr(AudioSourceRef);
	
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
		TLDebug_Print(strerr);
		
		return FALSE;
	}
		
	return TRUE;	
}


Bool Platform::OpenAL::SetRollOffFactor(TRefRef AudioSourceRef, const float fRollOffFactor)
{
	TPtr<AudioObj> pAO = g_Sources.FindPtr(AudioSourceRef);
	
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
		TLDebug_Print(strerr);
		
		return FALSE;
	}
		
	return TRUE;	
}




Bool Platform::OpenAL::HasBuffer(TRefRef AudioAssetRef)
{
	TPtr<AudioObj> pAO = g_Buffers.FindPtr(AudioAssetRef);
	
	if(pAO)
		return TRUE;
	
	return FALSE;		
}

Bool Platform::OpenAL::HasSource(TRefRef AudioSourceRef)
{
	TPtr<AudioObj> pAO = g_Sources.FindPtr(AudioSourceRef);
	
	if(pAO)
		return TRUE;

	return FALSE;		
}


Bool Platform::OpenAL::GetBufferID(TRefRef AudioAssetRef, ALuint& buffer)
{
	TPtr<AudioObj> pAO = g_Buffers.FindPtr(AudioAssetRef);
	
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
	TPtr<AudioObj> pAO = g_Sources.FindPtr(AudioSourceRef);
	
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
