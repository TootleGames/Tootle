#include "IPodAudio.h"

/*
namespace TLAudio
{
	namespace Platform
	{
		namespace OpenAL
		{
			ALboolean					g_bEAX = FALSE;

			const u32					NUM_BUFFERS = 1;
			ALuint						g_Buffers[1] = {0};
		}
	}
}
*/
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
	/*
	// Initialization
	ALCdevice* pDevice = alcOpenDevice(NULL); // select the "preferred device"

	if(pDevice) 
	{
		ALCcontext* pContext = alcCreateContext(pDevice,NULL);

		alcMakeContextCurrent(pContext);
	}
	else 
	{
		return SyncFalse;
	}

	// Check for EAX 2.0 support
	Platform::OpenAL::g_bEAX = alIsExtensionPresent("EAX2.0");

	// Generate Buffers
	alGetError(); // clear error code
	alGenBuffers(Platform::OpenAL::NUM_BUFFERS, Platform::OpenAL::g_Buffers);

	ALenum error;
	if ((error = alGetError()) != AL_NO_ERROR)
	{
		//DisplayALError("alGenBuffers :", error);
		return SyncFalse;
	}
*/
	return SyncTrue;
}


SyncBool Platform::OpenAL::Update()
{
	return SyncTrue;
}

SyncBool Platform::OpenAL::Shutdown()
{
	/*
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
