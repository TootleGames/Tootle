#pragma once

#include <TootleCore/TRelay.h>

#include <xaudio2.h>


namespace TLAudio
{
	namespace Platform
	{
		// Low level audio item
		class TAudioItem;
	}
}

// Platform specific class for processing low level audio events and
// propagating them to the engine audio system for handling
class TLAudio::Platform::TAudioItem : public TLMessaging::TRelay, public IXAudio2VoiceCallback
{
public:
    TAudioItem(TRef refAudioItemID);
    ~TAudioItem();

    //Called when the voice has just finished playing a contiguous audio stream.
    void STDMETHODCALLTYPE OnStreamEnd();

    //Unused methods are stubs
    void STDMETHODCALLTYPE OnVoiceProcessingPassEnd() { }
    void STDMETHODCALLTYPE OnVoiceProcessingPassStart(UINT32 SamplesRequired) {    }
    void STDMETHODCALLTYPE OnBufferEnd(void * pBufferContext)    { }
    void STDMETHODCALLTYPE OnBufferStart(void * pBufferContext) {    }
    void STDMETHODCALLTYPE OnLoopEnd(void * pBufferContext) {    }
    void STDMETHODCALLTYPE OnVoiceError(void * pBufferContext, HRESULT Error) { }

private:
	TRef	m_refAudioItemID;
    HANDLE	m_hBufferEndEvent;
};