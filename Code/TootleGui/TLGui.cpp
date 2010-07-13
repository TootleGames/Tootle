/*
 *  TLGui.cpp
 *  TootleGui
 *
 *  Created by Graham Reeves on 14/01/2010.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */
#include "TLGui.h"
#include <TootleRender/TScreenManager.h>
#include <TootleCore/TLUnitTest.h>


namespace TLGui
{
	TString	g_AppExe;
}


//------------------------------------------------------
//	lib init
//------------------------------------------------------
SyncBool TLGui::Init()
{
	return TLGui::Platform::Init();
}

//------------------------------------------------------
//	lib shutdown
//------------------------------------------------------
SyncBool TLGui::Shutdown()
{
	g_AppExe.Empty(TRUE);
	
	return TLGui::Platform::Shutdown();
}


//------------------------------------------------------
//	get the path of the application
//------------------------------------------------------
const TString& TLGui::GetAppExe()
{
	return g_AppExe;
}


//------------------------------------------------------
//	Set new application exe (only done once)
//------------------------------------------------------
void TLGui::SetAppExe(const TString& NewExe)
{
	g_AppExe = NewExe;
}

//------------------------------------------------------
//	get the cursor position in the default screen's client space
//------------------------------------------------------
int2 TLGui::GetDefaultScreenMousePosition(u8 MouseIndex)
{
	//	get the default screen
	TLRender::TScreen* pScreen = TLRender::g_pScreenManager ? TLRender::g_pScreenManager->GetDefaultScreen().GetObjectPointer() : NULL;
	if ( !TLRender::g_pScreenManager )
		return int2(-1,-1);

	//	get the window of the screen
	TLGui::TWindow* pWindow = pScreen->GetWindow();
	if ( !pWindow )
		return int2(-1,-1);

	//	now get the mouse's position relative to the window's client space
	return TLGui::Platform::GetScreenMousePosition( *pWindow, MouseIndex );
}


//------------------------------------------------------
//	handle command line params - if true is returned, return Result from main
//------------------------------------------------------
bool TLGui::OnCommandLine(const TString& CommandLine,int& Result)
{
	//	process UnitTest++ if "UnitTest" is on the command line
	if ( CommandLine == "UnitTest" )
	{
		Result = TLUnitTest::RunAllTests();
		return true;
	}

	//	not handled
	return false; 
}
