
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

		// Low level audio routines		
		Bool		CreateSource(TRefRef SourceRef);
		Bool		RemoveSource(TRefRef SourceRef);
		
		Bool		CreateBuffer(TRefRef AudioAssetRef);
		Bool		RemoveBuffer(TRefRef AudioAssetRef);
		
		Bool		HasSource(TRefRef AudioSourceRef);
		Bool		HasBuffer(TRefRef AudioAssetRef);
		
		Bool		AttachSourceToBuffer(TRefRef AudioSourceRef, TRefRef AudioAssetRef, Bool bStreaming);

		// Audio control
		Bool		StartAudio(TRefRef AudioSourceRef);
		Bool		StopAudio(TRefRef AudioSourceRef);
		Bool		PauseAudio(TRefRef AudioSourceRef);

		Bool		DetermineFinishedAudio(TArray<TRef>& refArray);
	}
};