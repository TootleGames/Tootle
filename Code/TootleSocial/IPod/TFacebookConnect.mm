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

//	gr: need the screen and gui to get hold of the window to attach the keyboard to
#include <TootleRender/TScreen.h>
#include <TootleRender/TScreenManager.h>
#include <TootleGui/IPod/IPodWindow.h>


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

Bool TLSocial::Platform::IPod::Facebook::CreateSession(const TString& APIKey, const TString& APISecret)
{
	g_pFacebookSession = [[SessionViewController alloc] init];

	//	gr: would it be better to create a new window?
	//	get the default screen's window
	TLRender::TScreen* pScreen = TLRender::g_pScreenManager->GetDefaultScreen();
	TLGui::TWindow* pWindow = pScreen ? pScreen->GetWindow() : NULL;
	TLGui::Platform::Window* pPlatformWindow = static_cast<TLGui::Platform::Window*>( pWindow );
	UIWindow* pUiWindow = pPlatformWindow ? pPlatformWindow->m_pWindow : NULL;
	
	[pUiWindow addSubview:g_pFacebookSession.view];	
	
	return TRUE; 
}

void TLSocial::Platform::IPod::Facebook::DestroySession()
{
	[g_pFacebookSession release];
}

void TLSocial::Platform::IPod::Facebook::BeginSession()
{
}

void TLSocial::Platform::IPod::Facebook::EndSession()
{
}

void TLSocial::Platform::IPod::Facebook::OpenDashboard()	
{
}



