#include "IPodAudio.h"


namespace TLAudio
{
	namespace Platform
	{
		namespace OpenAL
		{
			ALboolean					g_bEAX = FALSE;
			
			TArray<ALuint> g_Sources;
			TArray<ALuint> g_Buffers;
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


Bool Platform::OpenAL::CreateBuffer(ALuint& uBuffer)
{	
	alGenBuffers(1, &uBuffer);
	
	ALenum error;
	if ((error = alGetError()) != AL_NO_ERROR)
	{
		TString strerr = GetALErrorString(error);
		TLDebug_Print(strerr);
		
		return FALSE;
	}
	
	g_Buffers.Add(uBuffer);
	
	return TRUE;
}


Bool Platform::OpenAL::ReleaseBuffer(ALuint& uBuffer)
{
	return FALSE;
}

Bool Platform::OpenAL::CreateSource(ALuint& uSource)
{	
	alGenSources(1, &uSource);
	
	ALenum error;
	if ((error = alGetError()) != AL_NO_ERROR)
	{
		TString strerr = GetALErrorString(error);
		TLDebug_Print(strerr);
		
		return FALSE;
	}
	
	g_Sources.Add(uSource);
	
	return TRUE;
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
	
	return TRUE;
}



Bool Platform::OpenAL::ReleaseSource(ALuint& uSource)
{
	return FALSE;
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


Bool Platform::OpenAL::StartAudio()
{
/*
	// Load test.wav
	loadWAVFile("test.wav",&format,&data,&size,&freq,&loop);
	if ((error = alGetError()) != AL_NO_ERROR)
	{
		DisplayALError("alutLoadWAVFile test.wav : ", error);
		alDeleteBuffers(NUM_BUFFERS, g_Buffers);
		return;
	}
	// Copy test.wav data into AL Buffer 0
	alBufferData(g_Buffers[0],format,data,size,freq);

	if ((error = alGetError()) != AL_NO_ERROR)
	{
		DisplayALError("alBufferData buffer 0 : ", error);
		alDeleteBuffers(NUM_BUFFERS, g_Buffers);
		return;
	}

	// Unload test.wav
	unloadWAV(format,data,size,freq);
	if ((error = alGetError()) != AL_NO_ERROR)
	{
		DisplayALError("alutUnloadWAV : ", error);
		alDeleteBuffers(NUM_BUFFERS, g_Buffers);
		return;
	}

	// Generate Sources
	alGenSources(1,source);
	if ((error = alGetError()) != AL_NO_ERROR)
	{
		DisplayALError("alGenSources 1 : ", error);
		return;
	}

	// Attach buffer 0 to source
	alSourcei(source[0], AL_BUFFER, g_Buffers[0]);
	if ((error = alGetError()) != AL_NO_ERROR)
	{
		DisplayALError("alSourcei AL_BUFFER 0 : ", error);
	}
*/
	return TRUE;
}

Bool Platform::OpenAL::StopAudio()
{
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


