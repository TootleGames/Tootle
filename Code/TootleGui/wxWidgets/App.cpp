/*
 *  TWxWindow.cpp
 *  TootleGui
 *
 *  Created by Graham Reeves on 14/01/2010.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */
#if !defined(TL_ENABLE_WX)
#error Should only be built in wx only build
#endif // TL_ENABLE_WX

#if defined(TL_TARGET_MAC)
	#import <Cocoa/Cocoa.h>	//	access to NSHomeDirectory
#endif


#include "App.h"
#include <TootleCore/TCoreManager.h>
#include <TootleCore/TLCore.h>

//    implement Main() and bootup which enters our app's own init
IMPLEMENT_APP(wx::App)



void wx::App::OnIdle(wxIdleEvent& Event)
{
	TLCore::TootUpdate();
}

void wx::App::OnTimer(wxTimerEvent& Event)
{
	if ( TLCore::g_pCoreManager )
	{
		TLCore::g_pCoreManager->SetReadyForUpdate();
		wxWakeUpIdle();
	}
}


bool wx::App::OnInit()
{
    if ( !wxApp::OnInit() )
        return FALSE;

	//	set global app exe
#if defined(TL_TARGET_PC)
	TBufferString<MAX_PATH> Filename;
	TArray<TChar>& Buffer = Filename.GetStringArray();
	Buffer.SetSize( MAX_PATH );
	u32 ExeStringLength = GetModuleFileName( NULL, Buffer.GetData(), Buffer.GetSize() );
	Filename.SetLength( ExeStringLength );
	TLGui::SetAppExe( Filename );
#elif defined(TL_TARGET_MAC)
	//	get the root directory that the app is in
	NSString *HomeDir = NSHomeDirectory();
	TTempString HomeDirString;
	HomeDirString << HomeDir;
	TLGui::SetAppExe( HomeDirString );
#endif

    //    do engine init
    if ( !TLCore::TootInit() )
        return FALSE;
	
    //    get the app idle to do an update in
	Bind( wxEVT_IDLE, &OnIdle, wxID_ANY, wxID_ANY, this );
	
	//	start the update timer
	m_pUpdateTimer = new wxTimer( this );
	u32 UpdateInterval = (u32)TLTime::GetUpdateTimeMilliSecsf();
	m_pUpdateTimer->Start( UpdateInterval );
	Bind( wxEVT_TIMER, &OnTimer, wxID_ANY, wxID_ANY, this );

    return TRUE;
}

