
#include "PCAudio.h"
#include "PCAudioDirectX.h"

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
	}
}

using namespace TLAudio;


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
