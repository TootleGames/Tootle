#include "PCDebug.h"
#include <TootleGui/PC/PCGui.h>	//	windows headers
#include "../TString.h"

#pragma comment(lib,"user32.lib")


namespace TLDebug
{
	namespace Platform
	{
		static TTempString g_ConsoleBuffer;

		void		Print(const TString& String);	//	platform specific debug output - immediate
		void		FlushBuffer();
	}
}


SyncBool TLDebug::Platform::Initialise()
{
	return SyncTrue;
}

SyncBool TLDebug::Platform::Shutdown()
{
	FlushBuffer();
	return SyncTrue;
}



void TLDebug::Platform::PrintToBuffer(const TString& String)
{
	//	check in case this additional string and linefeed AND terminator will overflow the buffer
	if(g_ConsoleBuffer.GetLength() + String.GetLength() + 1 + 1 >= g_ConsoleBuffer.GetMaxAllocSize() )
		FlushBuffer();

	g_ConsoleBuffer.Append( String.GetData() );
	g_ConsoleBuffer.Append('\n');
}

void TLDebug::Platform::FlushBuffer()
{
	if ( g_ConsoleBuffer.GetLength() )
	{
		Print( g_ConsoleBuffer );
		g_ConsoleBuffer.Empty();
	}
}

//--------------------------------------------------
//	platform specific debug text output
//--------------------------------------------------
void TLDebug::Platform::Print(const TString& String)
{
	if ( String.GetLength() )
	{
		OutputDebugString( String.GetData() );

		//	todo: also print out to STD_OUT with WriteConsole()
	}
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
	PrintToBuffer( String );
	FlushBuffer();

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

	int Result = MessageBox( NULL, BreakMessage.GetData(), TLCharString("Debug break"), Flags );

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
	u32 NewLength = FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM, &Error, Error, 0, ErrorString.GetStringArray().GetData(), ErrorString.GetLength(), NULL );
	ErrorString.SetLength( NewLength );

	TTempString BreakString;
	BreakString.Appendf("Win32 Last Error: [%d] %s\n", Error, ErrorString.GetData() );
	return TLDebug::Break( BreakString );
}
