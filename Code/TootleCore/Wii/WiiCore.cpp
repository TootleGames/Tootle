/*------------------------------------------------------
	Wii Platform core

	contains the program entry

------------------------------------------------------*/
#include "WiiCore.h"
#include "../TLCore.h"
#include "../TLTypes.h"
#include "../TString.h"
#include "WiiTime.h"
#include "../TCoreManager.h"

 
#pragma comment( lib, "user32.lib" )
#pragma comment( lib, "gdi32.lib" )
#pragma comment( lib, "kernel32.lib" )
#pragma comment( lib, "winmm.lib" )	//	required for [multimedia] time functions
//#pragma comment(linker, "/NODEFAULTLIB:msvcrt.lib") 
#pragma comment(linker, "/NODEFAULTLIB:libcmt.lib") 
//#pragma comment( lib, "libc.lib" )




namespace TLCore
{
	namespace Platform
	{
		TString						g_AppExe;
		u32							g_TimerUpdateID;	//	ID of the win32 timer we're using for the update intervals



	}

	extern TPtr<TCoreManager>		g_pCoreManager;
}

//--------------------------------------------------
//	platform thread/process sleep
//--------------------------------------------------
void TLCore::Platform::Sleep(u32 Millisecs)
{
	//::Sleep( Millisecs );
}



//--------------------------------------------------
//	platform init
//--------------------------------------------------
SyncBool TLCore::Platform::Init()
{

	return SyncTrue;
}



void TLCore::Platform::QueryHardwareInformation(TBinaryTree& Data)	
{
	TLDebug_Print("Device Information:");

	/////////////////////////////////////////////////////////////
	// Device ID, OS and type
	/////////////////////////////////////////////////////////////	

	// General system info
	// NOTE: Should use GetNativeSystemInfo if possible and will also return the OS info
	// without having to do it separately.  Windows is a mess in how this info is retrieved! :(


	/////////////////////////////////////////////////////////////
	TLDebug_Print("End Device Information");
}

void TLCore::Platform::QueryLanguageInformation(TBinaryTree& Data)	
{
	TLDebug_Print("Language Information:");

	/////////////////////////////////////////////////////////////
	// Langauge
	/////////////////////////////////////////////////////////////


	// Convert the language ID into a TRef we can use
	// Default to english
	TRef LanguageRef = "eng";

	
	// Export the users language to the data - actual language selection will be done 
	// via the core manager
	Data.ExportData("Language", LanguageRef);
}


//--------------------------------------------------
//	platform update
//--------------------------------------------------
SyncBool TLCore::Platform::Update()
{

	//	keep app running
	return SyncTrue;
}



//--------------------------------------------------
//	platform shutdown
//--------------------------------------------------
SyncBool TLCore::Platform::Shutdown()
{

	return SyncTrue;
}


void TLCore::Platform::DoQuit()
{
	// Send a message to the core manager telling it to quit
	TLMessaging::TMessage Message("Quit");

	TLCore::g_pCoreManager->QueueMessage(Message);
}



//--------------------------------------------------
//	get the application exe
//--------------------------------------------------
const TString& TLCore::Platform::GetAppExe()
{
	return g_AppExe;
}


//--------------------------------------------------
//	platform specific debug text output
//--------------------------------------------------
void TLDebug::Platform::Print(const TString& String)
{
}


//--------------------------------------------------
//	return FALSE to stop app, TRUE and will attempt to continue
//--------------------------------------------------
Bool TLDebug::Platform::Break(const TString& String)
{
	//	gr: popup a message box with the error message etc
	//	let me know if this gets annoying when debugging and can turn it off 
	//	if a debugger is attached to the process (I think that's possible)

	//	print out message so it appears in output still after dismissing dialog
	Print( String );

	//	make up break message
	TTempString BreakMessage = String;
	BreakMessage.Append("\n\r \n\r");

	//	get last break position as string
	TLDebug::GetLastBreak( BreakMessage );

	BreakMessage.Append("\n\r \n\r Press Retry to try and continue or Cancel to break to debug");


	//	fail
	return FALSE;
}


