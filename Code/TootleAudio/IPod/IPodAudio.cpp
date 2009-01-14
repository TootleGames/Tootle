#include "IPodAudio.h"

#include <TootleAsset/TAsset.h>
#include <TootleAsset/TAudio.h>

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

using namespace TLAudio;

SyncBool Platform::Init()			
{	
	return OpenAL::Init();	
}

SyncBool Platform::Update()		
{	
	return SyncTrue;	
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
	// Stop the source
	return OpenAL::PauseAudio(AudioSourceRef);
}



Bool Platform::GetBufferID(TRefRef AudioAssetRef, ALuint& buffer)
{
	TPtr<OpenAL::AudioObj> pAO = OpenAL::g_Buffers.FindPtr(AudioAssetRef);
	
	if(pAO)
	{
		buffer = pAO->m_OpenALID;
		return TRUE;
	}

	// Not found
	return FALSE;	
}


Bool Platform::GetSourceID(TRefRef AudioSourceRef, ALuint& source)
{
	TPtr<OpenAL::AudioObj> pAO = OpenAL::g_Sources.FindPtr(AudioSourceRef);
	
	if(pAO)
	{
		source = pAO->m_OpenALID;
		return TRUE;
	}
	
	// Not found
	return FALSE;	
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





//////////////////////////////////////////////////////////
//
//	OpenAL specific code
//
//////////////////////////////////////////////////////////



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

/*
	//[11/12/08] DB - Docs seem to suggest a null pointer will mean failure but seems to always fail???

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
 */
	
	
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
	
	return SyncTrue;
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
	
	// Remove the source object from the array
	return g_Sources.Remove(pAO);	
}



SyncBool Platform::OpenAL::Update()
{
	return SyncTrue;
}

SyncBool Platform::OpenAL::Shutdown()
{
	
	// TODO: The removal of the sources will be done when the nodes shutdown
	//		 The removal of the buffers will be done when the file assets are shutdown
	
	ALuint		returnedName;
	// Delete the Sources
	if(g_Sources.GetSize())
		alDeleteSources(g_Sources.GetSize(), &returnedName);
	
	// Delete the Buffers	
	if(g_Buffers.GetSize())
		alDeleteBuffers(g_Buffers.GetSize(), &returnedName);
	
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


