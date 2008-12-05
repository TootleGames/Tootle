
#pragma once

#include <TootleCore/TLCore.h>

#include "../TLAudio.h"

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
			// som stage
			Bool			StartAudio();
			Bool			StopAudio();
		}

		SyncBool		Init();			
		SyncBool		Update();
		SyncBool		Shutdown();
	}
}

