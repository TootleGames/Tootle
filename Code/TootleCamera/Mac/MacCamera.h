/*
 *  MacCamera.h
 *  TootleCamera
 *
 *  Created by Duane Bradbury on 27/01/2010.
 *  Copyright 2010 Tootle. All rights reserved.
 *
 */

#include <TootleCore/TLTypes.h>
#include <TootleCore/TSubscriber.h>


namespace TLCamera
{
	namespace Platform
	{
		SyncBool Initialise();
		SyncBool Shutdown();
		
		
		Bool	ConnectToCamera();
		Bool	DisconnectFromCamera();
		
		
		void	SubscribeToCamera(TLMessaging::TSubscriber* pSubscriber);
	}
}