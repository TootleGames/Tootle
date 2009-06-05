/*
 *  TFacebookConnect.mm
 *  TootleSocial
 *
 *  Created by Duane Bradbury on 04/06/2009.
 *  Copyright 2009 Tootle. All rights reserved.
 *
 */

#include "TFacebookConnect.h"

#import "TFacebookSession.h"

#include <TootleCore/IPod/IPodApp.h>

// Create the facebook session view controller

namespace TLSocial 
{
	namespace Platform
	{
		namespace IPod
		{
			SessionViewController* g_pFacebookSession = NULL;
		}
	}
}

void TLSocial::Platform::BeginSession()
{
	IPod::g_pFacebookSession = [[SessionViewController alloc] init];
	
	[TLCore::Platform::g_pIPodApp.window addSubview:IPod::g_pFacebookSession.view];		
}

void TLSocial::Platform::EndSession()
{
	[IPod::g_pFacebookSession release];
}

