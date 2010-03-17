/*
 *  PCCamera.mm
 *  TootleCamera
 *
 *  Created by Duane Bradbury on 28/01/2010.
 *  Copyright 2010 Tootle. All rights reserved.
 *
 */

#include "PCCamera.h"

using namespace TLCamera;

SyncBool Platform::Initialise()
{
	return SyncTrue;
}

SyncBool Platform::Shutdown()
{
	return SyncTrue;
}



Bool Platform::ConnectToCamera()
{
	return FALSE;
}

Bool Platform::DisconnectFromCamera()
{
	return FALSE;
}

void Platform::SubscribeToCamera(TLMessaging::TSubscriber* pSubscriber)
{
	
}