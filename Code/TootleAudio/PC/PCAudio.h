
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
			SyncBool		Init();
			SyncBool		Update();
			SyncBool		Shutdown();

			// Low level audio requests - may be moved onto a low level audio manager at
			// som stage
			Bool			StartAudio();
			Bool			StopAudio();
		}

		SyncBool		Init();			
		SyncBool		Update();
		SyncBool		Shutdown();

	}
}


// DirectX audio engine callback
class TLAudio::Platform::DirectX::TAudioEngineCallback : public IXAudio2EngineCallback
{
    void STDMETHODCALLTYPE OnProcessingPassEnd () {}
    void STDMETHODCALLTYPE OnProcessingPassStart() {}
    void STDMETHODCALLTYPE OnCriticalError (HRESULT Error) {}
};





