/*
 *  MacMicrophone.h
 *  TootleAudio
 *
 *  Created by Duane Bradbury on 24/03/2010.
 *  Copyright 2010 Tootle. All rights reserved.
 *
 */

#pragma once

#include <TootleCore/TLTypes.h>

namespace TLAudio
{
	namespace Platform
	{
		Bool	CreateMicrophoneDevice(Bool bStartImmediately = FALSE);
		Bool	DestroyMicrophoneDevice();
		
		Bool	StartMicrophoneRecording();		
		Bool	StopMicrophoneRecording();
		
		Bool	UpdateMicrophone();
		
		Bool	IsMicrophoneConnected();
		Bool	IsMicrophoneRecording();
		
		SyncBool	GetMicrophoneData(void* pData, u32 uSamples);

	}
}