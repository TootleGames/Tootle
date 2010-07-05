/*
 *  MacCamera.cpp
 *  TootleCamera
 *
 *  Created by Duane Bradbury on 27/01/2010.
 *  Copyright 2010 Tootle. All rights reserved.
 *
 */

#include "MacCamera.h"
#include "MacCameraController.h"
//#include <TootleCore/Mac/MacApp.h>


namespace TLCamera
{
	MacCameraController* m_CameraController = nil;	
}

using namespace TLCamera;


SyncBool Platform::Initialise()
{
	return SyncTrue;
}

SyncBool Platform::Shutdown()
{
	if(m_CameraController)
	{
		if( [m_CameraController IsConnected] )
		{
			if(! [m_CameraController DisconnectFromCamera] )
			{
				return SyncFalse;
			}
		}
		
		// release the camera controller
		[m_CameraController release];
		
		m_CameraController = nil;
	}
	
	return SyncTrue;
}



Bool Platform::ConnectToCamera()
{
	if(!m_CameraController)
	{
		// Allocate the camera controller object
		m_CameraController = [[MacCameraController alloc] init];
		
		// Failed?
		if(!m_CameraController)
			return FALSE;	
	
		//	g_pFacebookSession = [[SessionViewController alloc] init];	
		NSApplication* app = [NSApplication sharedApplication];

		NSWindow* pWindow = [app mainWindow];
	
		NSView* pMainView = [pWindow contentView];
		
		[pMainView addSubview: m_CameraController.view];
		
		
		//[TLCore::Platform::g_pMacApp.window addSubview:m_CameraController.view];		
	}
	
	// Camera session already in progress?
	if([m_CameraController IsConnected])
		return TRUE;
	
	return [m_CameraController ConnectToCamera];
}

Bool Platform::DisconnectFromCamera()
{
	// Camera session already in progress?
	if(![m_CameraController IsConnected])
		return TRUE;
	
	return [m_CameraController DisconnectFromCamera];
}


void Platform::SubscribeToCamera(TLMessaging::TSubscriber* pSubscriber)
{
	if(m_CameraController)
	{
		[m_CameraController SubscribeToCamera:pSubscriber];
	}
}

