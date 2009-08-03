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
			namespace Facebook
			{
				SessionViewController* g_pFacebookSession = NULL;
			}
		}
	}
}

void TLSocial::Platform::IPod::Facebook::BeginSession(const TString& APIKey, const TString& APISecret)
{
	g_pFacebookSession = [[SessionViewController alloc] init];
	
	[TLCore::Platform::g_pIPodApp.window addSubview:g_pFacebookSession.view];		
}

void TLSocial::Platform::IPod::Facebook::EndSession()
{
	[g_pFacebookSession release];
}

void TLSocial::Platform::IPod::Facebook::OpenDashboard()	
{
}



