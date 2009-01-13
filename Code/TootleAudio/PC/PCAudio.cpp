
#include "PCAudio.h"

namespace TLAudio
{
	namespace Platform
	{
		namespace DirectX
		{
			IXAudio2*					g_pXAudio2 = NULL;
			IXAudio2MasteringVoice*		g_pMasteringVoice = NULL;

			TPtr<TAudioEngineCallback> g_pAudioEngineCallback = NULL;
		}

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
	#if(AUDIO_SYSTEM == AUDIO_DIRECTX)
		return DirectX::Init();	
	#else if(AUDIO_SYSTEM == AUDIO_OPENAL)
		return OpenAL::Init();	
	#endif
}

SyncBool Platform::Update()		
{	
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
	TPtr<OpenAL::AudioObj> pAO = OpenAL::CreateSource(AudioSourceRef);
	
	if(pAO)
	{
		TLDebug_Print("Audio source created successfully");	
		return TRUE;
	}
#endif
	
	
	TLDebug_Print("Failed to create source for audio");	
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

Bool Platform::CreateBuffer(TRefRef AudioAssetRef)
{
#if(AUDIO_SYSTEM == AUDIO_OPENAL)

	TPtr<OpenAL::AudioObj> pAO = OpenAL::CreateBuffer(AudioAssetRef);
	
	if(pAO)
	{
		return TRUE;
	}
#endif

	TLDebug_Print("Failed to create buffer for audio");	
	return FALSE;
}

Bool Platform::RemoveBuffer(TRefRef AudioAssetRef)
{
#if(AUDIO_SYSTEM == AUDIO_OPENAL)

	// Release the platform specific buffer data		
	if(!Platform::OpenAL::ReleaseBuffer(AudioAssetRef))
	{
		// Failed? No low level buffer? 
		TLDebug_Print("Failed to release low level audio buffer");
		return FALSE;
	}
#endif	
	return TRUE;
}

Bool Platform::HasBuffer(TRefRef AudioAssetRef)
{
#if(AUDIO_SYSTEM == AUDIO_OPENAL)

	TPtr<OpenAL::AudioObj> pAO = OpenAL::g_Buffers.FindPtr(AudioAssetRef);
	
	if(pAO)
		return TRUE;
#endif
	
	return FALSE;		
}

Bool Platform::HasSource(TRefRef AudioSourceRef)
{
#if(AUDIO_SYSTEM == AUDIO_OPENAL)

	TPtr<OpenAL::AudioObj> pAO = OpenAL::g_Sources.FindPtr(AudioSourceRef);
	
	if(pAO)
		return TRUE;
#endif

	return FALSE;		
}



Bool Platform::StartAudio(TRefRef AudioSourceRef)
{
#if(AUDIO_SYSTEM == AUDIO_OPENAL)
	// Play the source
	return OpenAL::StartAudio(AudioSourceRef);
#endif

	return FALSE;
}

Bool Platform::StopAudio(TRefRef AudioSourceRef)
{
#if(AUDIO_SYSTEM == AUDIO_OPENAL)
	// Stop the source
	return OpenAL::StopAudio(AudioSourceRef);
#endif

	return FALSE;
}

Bool Platform::PauseAudio(TRefRef AudioSourceRef)
{
#if(AUDIO_SYSTEM == AUDIO_OPENAL)
	// Stop the source
	return OpenAL::PauseAudio(AudioSourceRef);
#endif

	return FALSE;
}



Bool Platform::GetBufferID(TRefRef AudioAssetRef, ALuint& buffer)
{
#if(AUDIO_SYSTEM == AUDIO_OPENAL)
	TPtr<OpenAL::AudioObj> pAO = OpenAL::g_Buffers.FindPtr(AudioAssetRef);
	
	if(pAO)
	{
		buffer = pAO->m_OpenALID;
		return TRUE;
	}
#endif

	// Not found
	return FALSE;	
}


Bool Platform::GetSourceID(TRefRef AudioSourceRef, ALuint& source)
{
#if(AUDIO_SYSTEM == AUDIO_OPENAL)

	TPtr<OpenAL::AudioObj> pAO = OpenAL::g_Sources.FindPtr(AudioSourceRef);
	
	if(pAO)
	{
		source = pAO->m_OpenALID;
		return TRUE;
	}
#endif
	
	// Not found
	return FALSE;	
}


Bool Platform::AttachSourceToBuffer(TRefRef AudioSourceRef, TRefRef AudioAssetRef, Bool bStreaming)
{
#if(AUDIO_SYSTEM == AUDIO_OPENAL)

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
#endif

	return FALSE;
}



/////////////////////////////////////////////////////////////////////////////////////////
// DirectX Audio
/////////////////////////////////////////////////////////////////////////////////////////

SyncBool Platform::DirectX::Init()
{
	// Create the low level audio engine callback handler
	if(!g_pAudioEngineCallback)
		g_pAudioEngineCallback = new Platform::DirectX::TAudioEngineCallback();

    // Initialize XAudio2
    CoInitializeEx( NULL, COINIT_MULTITHREADED );

    UINT32 flags = 0;
#ifdef _DEBUG
    flags |= XAUDIO2_DEBUG_ENGINE;
#endif

	HRESULT hr = XAudio2Create( &Platform::DirectX::g_pXAudio2, flags, XAUDIO2_DEFAULT_PROCESSOR );

	if( FAILED( hr ) )
    {
        TLDebug_Break( "Failed to init XAudio2 engine");

		Platform::Shutdown();

		return SyncFalse;
    }

/*
	// TODO: determine device requirements and select the most appropriate device
	UINT32 deviceCount;
	Platform::DirectX::g_pXAudio2->GetDeviceCount(&deviceCount);
	
	//Loop through the available devices to determine the most suitable device.
	//Details of an audio device can be retrieved with the GetDeviceDetails function.

	XAUDIO2_DEVICE_DETAILS deviceDetails;
	s32 preferredDevice = 0;
	for (u32 uIndex = 0; uIndex < deviceCount; uIndex++)
	{
		Platform::DirectX::g_pXAudio2->GetDeviceDetails(uIndex,&deviceDetails);

		if (deviceDetails.OutputFormat.Format.nChannels > 2)
			preferredDevice = i;
		}
	}

	
	hr = Platform::DirectX::g_pXAudio2->CreateMasteringVoice( &pMasterVoice, XAUDIO2_DEFAULT_CHANNELS,
                            XAUDIO2_DEFAULT_SAMPLERATE, 0, preferredDevice, NULL ) ) );

*/

	// Register for callbacks
	hr = Platform::DirectX::g_pXAudio2->RegisterForCallbacks(Platform::DirectX::g_pAudioEngineCallback.GetObject());

	if(FAILED(hr))
	{
		Platform::Shutdown();

		return SyncFalse;
	}

    // Create a mastering voice
	hr = Platform::DirectX::g_pXAudio2->CreateMasteringVoice( &Platform::DirectX::g_pMasteringVoice );

	if( FAILED( hr ) )
    {
        TLDebug_Break( "Failed to create mastering voice");
        
		Platform::Shutdown();

		return SyncFalse;
    }

	return SyncTrue;
}

SyncBool Platform::DirectX::Update()
{
	return SyncTrue;
}

SyncBool Platform::DirectX::Shutdown()
{
	// Delete the mastering voice
	if(Platform::DirectX::g_pMasteringVoice)
	{
		Platform::DirectX::g_pMasteringVoice->DestroyVoice();
		Platform::DirectX::g_pMasteringVoice = NULL;
	}

	// Delete the directx audio engine
	if(Platform::DirectX::g_pXAudio2)
	{
		Platform::DirectX::g_pXAudio2->Release();
		Platform::DirectX::g_pXAudio2 = NULL;
	}

	CoUninitialize();

	// Delete the callback handler
	if(Platform::DirectX::g_pAudioEngineCallback)
	{
		Platform::DirectX::g_pAudioEngineCallback = NULL;
	}

	return SyncTrue;
}


Bool Platform::DirectX::StartAudio()
{
/*
	// Fill out a WAVEFORMATEX structure or one of the extended format structures containing a 
	// WAVEFORMATEX structure depending on the audio data format.
	// For information on possible structures see the table in the WAVEFORMATEX reference page. 
	// These structures are typicaly populated by loading data from the 'fmt ' chunk in an audio file. 
	// For an example of parsing and loading data from a RIFF file in XAudio2 see the XAudio2 BasicSound 
	// Sample sample.
	
	WAVEFORMATEXTENSIBLE wfx = {0};
	FindChunk(hFile,'fmt ', dwChunkSize, dwChunkPosition );
	ReadChunkData(hFile, &wfx, dwChunkSize, dwChunkPosition );

	IXAudio2SourceVoice* pSourceVoice;

	HRESULT hr = Platform::DirectX::g_pXAudio2->CreateSourceVoice( &pSourceVoice, 
												(WAVEFORMATEX*)&wfx, 
												0, 
												XAUDIO2_DEFAULT_FREQ_RATIO, 
												NULL,	// Source voice callback 
												NULL, 
												NULL );

	if( FAILED( hr ) ) 
		return hr;
	
	// Populate an XAUDIO2_BUFFER structure with data. 
	XAUDIO2_BUFFER buffer = {0};
	buffer.AudioBytes = dwChunkSize;  //size of the audio buffer in bytes
	buffer.pAudioData = pDataBuffer;  //buffer containing audio data
	buffer.Flags = XAUDIO2_END_OF_STREAM; // tell the source voice not to expect any data after this buffer
	
	//Note:
	// The application is responsible for providing audio data in a format usable by XAudio2. 
	// XAudio2 does not provide methods for loading audio data. 
	// See Supported Audio Formats for information on the formats supported by XAudio2.

	// Submit the XAUDIO2_BUFFER to the source voice using the function SubmitSourceBuffer.
	hr = pSourceVoice->SubmitSourceBuffer( &buffer );

	if( FAILED( hr ) ) 
		return FALSE;

	// Start the source voice using the Start function.
	// Since all XAudio2 voices send their output to the mastering voice by default, audio from the source voice will automatically make its way to the audio device selected at initialization. In a more complicated audio graph the source voice would have to specify the voice its output should be sent to.

	hr = pSourceVoice->Start( 0, XAUDIO2_COMMIT_NOW );

	if ( FAILED( hr ) )
		return FALSE;
*/
	return TRUE;
}

Bool Platform::DirectX::StopAudio()
{
	return TRUE;
}




/////////////////////////////////////////////////////////////////////////////////////////
// OpenAL
/////////////////////////////////////////////////////////////////////////////////////////

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
