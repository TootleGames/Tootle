/*
 *  TCameraManager.cpp
 *  TootleCamera
 *
 *  Created by Duane Bradbury on 27/01/2010.
 *  Copyright 2010 Tootle. All rights reserved.
 *
 */

#include "TCameraManager.h"

#include "TLCamera.h"


namespace TLCamera
{
	TPtr<TCameraManager>	g_pCameraManager = NULL;	// The global camera manager
}

using namespace TLCamera;

SyncBool TCameraManager::Initialise()
{
	// Initialise the low level device
	return TLCamera::Initialise();
}

SyncBool TCameraManager::Shutdown()
{
	return TLCamera::Shutdown();
}


Bool TCameraManager::ConnectToCamera()
{
	return TLCamera::ConnectToCamera();
}

Bool TCameraManager::DisconnectFromCamera()
{
	return TLCamera::DisconnectFromCamera();
}

