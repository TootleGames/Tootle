/*
 *  MacMicrophone.mm
 *  TootleAudio
 *
 *  Created by Duane Bradbury on 24/03/2010.
 *  Copyright 2010 Tootle. All rights reserved.
 *
 */

#include "MacMicrophone.h"
#include <TootleCore/TLCore.h>

#import <OpenAL/al.h>
#import <OpenAL/alc.h>


namespace TLAudio
{
	namespace Platform
	{
		ALCdevice*	g_pMicrophoneDevice = NULL;
		Bool		g_bMicrophoneRecording = FALSE;
	}
}


using namespace TLAudio;


Bool Platform::IsMicrophoneConnected()
{
	return (g_pMicrophoneDevice != NULL);
}

Bool Platform::IsMicrophoneRecording()
{
	return g_bMicrophoneRecording;
}




Bool Platform::CreateMicrophoneDevice(Bool bStartImmediately)
{
	// Device exists?
	if(g_pMicrophoneDevice)
		return TRUE;

	//Open default capture device
	/* 
	 g_pMicrophoneDevice = alcCaptureOpenDevice(NULL,
									 ALCuint frequency,
									 ALCenum format,
									 ALCsizei buffersize);
	 */
	// Device created successfully?
	if(!g_pMicrophoneDevice)
		return FALSE;
	
	const ALCchar* actualDeviceName = alcGetString(g_pMicrophoneDevice, ALC_DEVICE_SPECIFIER);
	
	TTempString devicename("OpenAL Capture Device: ");
	devicename.Appendf("%s", actualDeviceName);
	TLDebug_Print("OpenAL Capture Device opened successfully");
	TLDebug_Print(devicename);
	
	
	// Start the device immediately?
	if(bStartImmediately)
		return StartMicrophoneRecording();
	
	return TRUE;
}


Bool Platform::DestroyMicrophoneDevice()
{
	if(g_pMicrophoneDevice)
	{
		const ALCchar* actualDeviceName = alcGetString(g_pMicrophoneDevice, ALC_DEVICE_SPECIFIER);
		TTempString devicename("Closing OpenAL Capture Device: ");
		devicename.Appendf("%s", actualDeviceName);
		TLDebug_Print(devicename);
	
		if(ALC_TRUE == alcCaptureCloseDevice(g_pMicrophoneDevice))
		{
			
			TLDebug_Print("OpenAL Capture Device closed successfully");			
			return TRUE;
		}
		
		TLDebug_Print("OpenAL Capture Device close failed");			
	}

	return FALSE;
}

Bool Platform::StartMicrophoneRecording()
{
	if(!g_bMicrophoneRecording && g_pMicrophoneDevice)
	{
		alcCaptureStart(g_pMicrophoneDevice);
		g_bMicrophoneRecording = TRUE;
	}
	
	return g_bMicrophoneRecording;
}

Bool Platform::StopMicrophoneRecording()
{
	if(g_bMicrophoneRecording && g_pMicrophoneDevice)
	{
		alcCaptureStop(g_pMicrophoneDevice);
		g_bMicrophoneRecording = FALSE;
	}
	
	return g_bMicrophoneRecording;
}


SyncBool Platform::GetMicrophoneData(void* pData, u32 uSamples)
{
	if(!g_pMicrophoneDevice)
		return SyncFalse;
	
	// Read how much data available
	//ALint numberofsamples = alcGetInteger(ALC_CAPTURE_SAMPLES);
	// wait for enough samples to be available
	// if(numberofsamples < uSamples)
	//		return SyncWait;
	
	alcCaptureSamples(g_pMicrophoneDevice, (ALCvoid*)pData, (ALCsizei)uSamples );
	
	return SyncTrue;	
}
