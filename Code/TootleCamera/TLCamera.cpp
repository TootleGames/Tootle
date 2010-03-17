/*
 *  TLCamera.cpp
 *  TootleCamera
 *
 *  Created by Duane Bradbury on 27/01/2010.
 *  Copyright 2010 Tootle. All rights reserved.
 *
 */

#include "TLCamera.h"

namespace TLCamera
{
	namespace Platform
	{
		Bool	ConnectToCamera();
		Bool	DisconnectFromCamera();
		
		SyncBool	Initialise();
		SyncBool	Shutdown();
		
		void		SubscribeToCamera(TLMessaging::TSubscriber* pSubscriber);		
	}
}


SyncBool TLCamera::Initialise()
{
	return Platform::Initialise();
}

SyncBool TLCamera::Shutdown()
{
	return Platform::Shutdown();
}


Bool TLCamera::ConnectToCamera()
{
	return Platform::ConnectToCamera();
}

Bool TLCamera::DisconnectFromCamera()
{
	return Platform::DisconnectFromCamera();
}


void TLCamera::SubscribeToCamera(TLMessaging::TSubscriber* pSubscriber)
{
	Platform::SubscribeToCamera(pSubscriber);
}

