
#pragma once

#include "TAudiograph.h"

#include <TootleCore/TLTypes.h>

namespace TLAudio
{
	namespace Platform
	{
		SyncBool	Init();
		SyncBool	Update();
		SyncBool	Shutdown();

		// Low level audio request
		Bool		StartAudio();
		Bool		StopAudio();
	}
};