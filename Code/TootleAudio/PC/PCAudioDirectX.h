
#pragma once

#include <TootleCore/TLCore.h>
#include <TootleCore/TPtr.h>

// DirectX audio include
#include <xaudio2.h>	


// DirectX libraries
//#pragma comment( lib, "xaudio2.lib" )
#pragma comment( lib, "ole32.lib" )

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
	}
}


// DirectX audio engine callback
class TLAudio::Platform::DirectX::TAudioEngineCallback : public IXAudio2EngineCallback
{
    void STDMETHODCALLTYPE OnProcessingPassEnd () {}
    void STDMETHODCALLTYPE OnProcessingPassStart() {}
    void STDMETHODCALLTYPE OnCriticalError (HRESULT Error) {}
};





