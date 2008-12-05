/*------------------------------------------------------
	PC Platform core

	contains the program entry

------------------------------------------------------*/
#include "PCCore.h"
#include "../TLCore.h"
#include "../TLTypes.h"
#include "../TString.h"
#include "PCTime.h"
#include "../TCoreManager.h"
#include <mmsystem.h>

 
#pragma comment( lib, "user32.lib" )
#pragma comment( lib, "gdi32.lib" )
#pragma comment( lib, "kernel32.lib" )
#pragma comment( lib, "winmm.lib" )	//	required for [multimedia] time functions
//#pragma comment(linker, "/NODEFAULTLIB:msvcrt.lib") 
#pragma comment(linker, "/NODEFAULTLIB:libcmt.lib") 
//#pragma comment( lib, "libc.lib" )


//---------------------------------------------------
//	globals
//---------------------------------------------------
HINSTANCE	TLCore::Platform::g_HInstance = NULL;
HWND		TLCore::Platform::g_HWnd = NULL;

namespace TLCore
{
	namespace Platform
	{
		TString						g_AppExe;
		u32							g_TimerUpdateID;	//	ID of the win32 timer we're using for the update intervals
		MMRESULT					g_MMTimerUpdateID;	//	

		void CALLBACK				UpdateTimerCallback(HWND hwnd,UINT uMsg,UINT_PTR idEvent,DWORD dwTime);
		void CALLBACK				UpdateMMTimerCallback(UINT uTimerID, UINT uMsg, DWORD_PTR dwUser, DWORD_PTR dw1, DWORD_PTR dw2);
	}

	extern TPtr<TCoreManager>		g_pCoreManager;
}


//---------------------------------------------------
//	win32 entry
//---------------------------------------------------
int WINAPI WinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance,LPSTR lpCmdLine,int nCmdShow)
{
	//	set the global reference to HInstance
	TLCore::Platform::g_HInstance = hInstance;

	//	set global app exe
	TLCore::Platform::g_AppExe.SetLength( MAX_PATH );
	u32 ExeStringLength = GetModuleFileName( NULL, TLCore::Platform::g_AppExe.GetData(), TLCore::Platform::g_AppExe.GetLength() );
	TLCore::Platform::g_AppExe.SetLength( ExeStringLength );

	//	go to the tootle main loop
	Bool Result = TLCore::TootMain();

	return Result ? 0 : 255;
}


//--------------------------------------------------
//	platform init
//--------------------------------------------------
SyncBool TLCore::Platform::Init()
{
	//	setup the update timer
	u32 UpdateInterval = TLTime::GetUpdateTimeMilliSecs();

	Bool UseMMTimer = TRUE;

	//	to make debugging easier in VS (ie. let windows breath) we dont use the MM timer, this way the windows
	//	message queue is blocking
	if ( TLDebug::IsEnabled() )
		UseMMTimer = FALSE;

	if ( UseMMTimer )
	{
		g_MMTimerUpdateID = timeSetEvent( UpdateInterval, 0, TLCore::Platform::UpdateMMTimerCallback, 0, TIME_PERIODIC );
		if ( g_MMTimerUpdateID == NULL )
			UseMMTimer = FALSE;
	}

	if ( !UseMMTimer )
	{
		g_TimerUpdateID = (u32)SetTimer( NULL, 0, UpdateInterval, TLCore::Platform::UpdateTimerCallback );
		if ( g_TimerUpdateID == 0 )
		{
			TLDebug::Platform::CheckWin32Error();
			return SyncFalse;
		}
	}

	return SyncTrue;
}



//--------------------------------------------------
//	platform update
//--------------------------------------------------
SyncBool TLCore::Platform::Update()
{
	MSG msg;
	
	//	win32 style app update (blocking)
	Bool Blocking = TRUE;
	Blocking = (g_MMTimerUpdateID == NULL);

	if ( Blocking )
	{
		//	wait for message
		while (GetMessage(&msg, NULL, 0, 0)) 
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);

			//	no more messages, and we've got updates to do so break out and let the app loop
			if ( !PeekMessage(&msg,NULL,0,0,PM_NOREMOVE) )
			{
				if ( TLCore::g_pCoreManager->HasTimeSteps() )
					break;
			}
		}

	}
	else
	{
		//	process windows messages if there are any
		while ( PeekMessage(&msg,NULL,0,0,PM_REMOVE) )
		{
			if ( msg.message == WM_QUIT )
			{
				Platform::DoQuit();
				return SyncTrue;
			}

			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	//	have we got a load of spare time before next update?
	//Sleep(12);

	//	keep app running
	return SyncTrue;
}



//--------------------------------------------------
//	platform shutdown
//--------------------------------------------------
SyncBool TLCore::Platform::Shutdown()
{
	if ( g_TimerUpdateID != 0 )
	{
		KillTimer( NULL, g_TimerUpdateID );
	}

	return SyncTrue;
}


void TLCore::Platform::DoQuit()
{
	// Send a message to the core manager telling it to quit
	TPtr<TLMessaging::TMessage> pMessage = new TLMessaging::TMessage("Quit");

	if(pMessage.IsValid())
	{
		TLCore::g_pCoreManager->QueueMessage(pMessage);
	}
	else
		TLDebug_Break("Unable to send quit message");
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
	OutputDebugString( String.GetData() );
	OutputDebugString( "\n" );
}


//--------------------------------------------------
//	return FALSE to stop app, TRUE and will attempt to continue
//--------------------------------------------------
Bool TLDebug::Platform::Break(const TString& String)
{
	//	gr: popup a message box with the error message etc
	//	let me know if this gets annoying when debugging and can turn it off 
	//	if a debugger is attached to the process (I think that's possible)

	//	make up break message
	TTempString BreakMessage = String;
	BreakMessage.Append("\n\r \n\r");

	//	get last break position as string
	TLDebug::GetLastBreak( BreakMessage );

	BreakMessage.Append("\n\r \n\r Press Retry to try and continue or Cancel to break to debug");

	//	show message box
	u32 Flags = MB_RETRYCANCEL|MB_DEFBUTTON2;	//	make CANCEL default so debug break is default
	Flags |= MB_TASKMODAL;			//	no easily accessible hwnd so make it thread modal
	Flags |= MB_ICONERROR;			//	icons R COOL

	int Result = MessageBox( NULL, BreakMessage.GetData(), "Debug break", Flags );

	//	anything other than cancel will continue without breaking
	if ( Result != IDCANCEL )
		return TRUE;
	
	//	break
	//	gr: changed debug break to an alternative - for some reason on my system (since I reformatted, maybe
	//	visual studio express problem, or something I've installed differently, but I wasn't getting a call stack
	//	when I let it break (breakpointing lines above would be fine).
	//	this version does work though...
	__debugbreak();	//	_asm { int 3 }	//	same thing
	
	//DebugBreak();	

	//	fail
	return FALSE;
}


//--------------------------------------------------
//	checks for a win32 error and does a break
//--------------------------------------------------
Bool TLDebug::Platform::CheckWin32Error()
{
	//	print out the last error from windows
	u32 Error = GetLastError();

	//	not a real error
	if ( Error == ERROR_SUCCESS )
		return TRUE;

	//	check code against
	//	http://msdn.microsoft.com/library/default.asp?url=/library/en-us/debug/base/system_error_codes.asp

	//	get error string from windows
	TTempString ErrorString;
	ErrorString.SetLength(256);
	u32 NewLength = FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM, &Error, Error, 0, ErrorString.GetData(), ErrorString.GetLength(), NULL );
	ErrorString.SetLength( NewLength );

	TTempString BreakString;
	BreakString.Appendf("Win32 Last Error: [%d] %s\n", Error, ErrorString.GetData() );
	return TLDebug::Break( BreakString );
}


//--------------------------------------------------
//	mmsystem update timer callback
//--------------------------------------------------
void CALLBACK TLCore::Platform::UpdateMMTimerCallback(UINT uTimerID, UINT uMsg, DWORD_PTR dwUser, DWORD_PTR dw1, DWORD_PTR dw2)
{
	//	dont do any threaded code whilst breaking
	if ( TLDebug::IsBreaking() )
		return;

	//	add an update timestamp to the update time queue
	TLTime::TTimestamp UpdateTimerTime( TRUE );

	if ( TLCore::g_pCoreManager )
		TLCore::g_pCoreManager->AddTimeStep( UpdateTimerTime );
}


//--------------------------------------------------
//	win32 update timer callback
//--------------------------------------------------
void CALLBACK TLCore::Platform::UpdateTimerCallback(HWND hwnd,UINT uMsg,UINT_PTR idEvent,DWORD dwTime)
{
	//	dont do any threaded code whilst breaking
	if ( TLDebug::IsBreaking() )
		return;

	//	check params, this should just be a callback for the update timer
	if( uMsg != WM_TIMER )
	{
		if ( !TLDebug_Break("Unexpected timer callback") )
			return;
	}

	if(idEvent != g_TimerUpdateID)
	{
		// Suggests that we are running out of a frame??
	}

	//	add an update timestamp to the update time queue
	TLTime::TTimestamp UpdateTimerTime;
	TLTime::Platform::GetTimestampFromTickCount( UpdateTimerTime, dwTime );

	TLCore::g_pCoreManager->AddTimeStep(UpdateTimerTime);
}
