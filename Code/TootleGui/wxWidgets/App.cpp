/*
 *  TWxWindow.cpp
 *  TootleGui
 *
 *  Created by Graham Reeves on 14/01/2010.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */
#if defined(TL_ENABLE_WX)


#include "App.h"
#include <TootleCore/TCoreManager.h>

//    implement Main() and bootup which enters our app's own init
IMPLEMENT_APP(TLGui::Platform::App)



void wx::App::OnIdle(wxIdleEvent& Event)
{
	TLCore::TootUpdate();
}

void wx::App::OnTimer(wxTimerEvent& Event)
{
	if ( g_pCoreManager )
	{
		g_pCoreManager->SetReadyForUpdate();
		wxWakeUpIdle();
	}
}


bool wx::App::OnInit()
{
    if ( !wxApp::OnInit() )
        return FALSE;
	
	//	get the root directory that the app is in
	NSString *HomeDir = NSHomeDirectory();
	TLCore::Platform::g_AppExe << (HomeDir);
	
    //    do engine init
    if ( !TLCore::TootInit() )
        return FALSE;
	
    //    get the app idle to do an update in
	Bind( wxEVT_IDLE, &TLCore::Platform::TwxApp::OnIdle, this );
	
	//	start the update timer
	m_pUpdateTimer = new wxTimer( this );
	u32 UpdateInterval = (u32)TLTime::GetUpdateTimeMilliSecsf();
	m_pUpdateTimer->Start( UpdateInterval );
	Bind( wxEVT_TIMER, &TLCore::Platform::TwxApp::OnTimer, this );

    return TRUE;
}


#endif // TL_ENABLE_WX
