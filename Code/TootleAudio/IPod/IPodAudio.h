
#pragma once

#include <TootleCore/TLCore.h>

#include "../TLAudio.h"

#import <OpenAL/al.h>
#import <OpenAL/alc.h>

/*
// OpenAL audio include
#include <al.h>			
#include <alc.h>

// OpenAL libraries
#pragma comment( lib, "openal32.lib")	
*/
namespace TLAudio
{
	namespace Platform
	{
		namespace OpenAL
		{
			SyncBool		Init();
			SyncBool		Update();
			SyncBool		Shutdown();

			// Low level audio requests - may be moved onto a low level audio manager at
			// some stage
			Bool			StartAudio();
			Bool			StopAudio();
			
			
			Bool CreateBuffer(ALuint& uBuffer);
			Bool ReleaseBuffer(ALuint& uBuffer);
			
			Bool CreateSource(ALuint& uSource);
			Bool ReleaseSource(ALuint& uSource);
			
			Bool AttachSourceToBuffer(ALuint& uSource, ALuint& uBuffer, const Bool bStreaming);

			
			// Error routines
			TString			GetALErrorString(ALenum err);	
			TString			GetALCErrorString(ALCenum err);
		}

		SyncBool		Init();			
		SyncBool		Update();
		SyncBool		Shutdown();
	}
}

