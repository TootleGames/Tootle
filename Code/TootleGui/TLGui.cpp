/*
 *  TLGui.cpp
 *  TootleGui
 *
 *  Created by Graham Reeves on 14/01/2010.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */
#include "TLGui.h"
#include <TootleCore/TLUnitTest.h>


namespace TLGui
{
	TString	g_AppExePath;

	namespace Platform
	{
#if defined(TL_TARGET_PC)
		void*		g_HInstance = NULL;	//	HINSTANCE
#endif
	}
}

TEST(GuiSetup)
{
	//	gr: these fail atm because the path is setup AFTER the tests are run.
	//	maybe ditch this entirely and let the file system stuff worry about the app path?
	/*
	TTempString Path = TLGui::GetAppPath();

	//	make sure the app path isn't empty
	CHECK( Path.GetLength() > 0 );

	//	make sure the app path ends with a back slash.
	TChar TrailingChar = Path.GetCharLast();
	CHECK( TrailingChar == '/' );

	//	make sure the path contains no forward slashes (only back slashes!)
	CHECK( Path.GetCharIndex('\\') == -1 );
	 */
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
	g_AppExePath.Empty(true);
	
	return TLGui::Platform::Shutdown();
}

//------------------------------------------------------
//	get the path of the application
//------------------------------------------------------
TString TLGui::GetAppExe()
{
	TString Exe = g_AppExePath;
	Exe << "yourapp.exe";

	return Exe;
}

//------------------------------------------------------
//	get the path of the application
//------------------------------------------------------
const TString& TLGui::GetAppPath()
{
	return g_AppExePath;
}


//------------------------------------------------------
//	Set new application executable path
//------------------------------------------------------
void TLGui::SetAppPath(const TString& Path)
{
	g_AppExePath = Path;
}


//------------------------------------------------------
//	handle command line params - if true is returned, return Result from main
//	note; assume all command line options are lower case (the caller should probably pass a TStringLowercase type of string in
//------------------------------------------------------
bool TLGui::OnCommandLine(const TString& CommandLine,int& Result)
{
	//	process UnitTest++ if "UnitTest" is on the command line
	if ( CommandLine == "unittest" )
	{
		//	if the tests aren't supported, return no error, but exit
		if ( !TLUnitTest::RunAllTests( Result ) )
			Result = 0;
		return true;
	}

	//	not handled
	return false; 
}
