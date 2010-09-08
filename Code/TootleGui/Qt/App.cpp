/*
 *  TWxWindow.cpp
 *  TootleGui
 *	mpanyName__. All rights reserved.
 *
 */
#if defined(TL_TARGET_MAC)
//	#import <Cocoa/Cocoa.h>	//	access to NSHomeDirectory
#endif

#include "App.h"
#include <TootleCore/TCoreManager.h>
#include <TootleCore/TLCore.h>
#include <TootleFileSys/TLFileSys.h>


TLGui::Platform::App::App(int argc, char *argv[]) :
	QApplication	( argc, argv )
{
}


bool TLGui::Platform::App::Init()
{
	//	extract the application path
	TTempString Path;
	Path << applicationDirPath();
	
	//	on osx, this path will be inside the bundle. To get the .app's path we have to go up a directory.
	//	Path should end with .app/
#if defined(TL_TARGET_MAC)
	TLFileSys::GetParentDir( Path );
	TLFileSys::GetParentDir( Path );
#endif
	
	//	end the path with a slash
	if ( Path.GetCharLast() != '/' )
		Path << '/';

	//	set path
	TLGui::SetAppPath( Path );
	
	
    //    do engine init
    if ( !TLCore::TootInit() )
        return false;

	//	setup the update timer
	u32 UpdateInterval = (u32)TLTime::GetUpdateTimeMilliSecsf();
	startTimer( UpdateInterval );
	
	return true;
}

void TLGui::Platform::App::Shutdown()
{
	//	stop any timers
//	killTimers();
	
	TLCore::TootShutdown(true);
}

void TLGui::Platform::App::timerEvent(QTimerEvent* TimerEvent)
{
	//	ready for another update
	if ( TLCore::g_pCoreManager )
		TLCore::g_pCoreManager->SetReadyForUpdate();
	
	//	just do one!
	TLCore::TootUpdate();
}


