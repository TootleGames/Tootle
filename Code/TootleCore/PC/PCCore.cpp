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

#include "cpuid.h"


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

		// detailed cpu info class
		class CPUINFOEX
		{
		public:
			u16 NUMAnodes;
			u16 numProcessorPackages;
			u16 numProcessorCores;
			u16 numLogicalProcessors;

			u16 numL1Caches;
			u16 numL2Caches;
			u16 numL3Caches;
			u16 pad;
		};

		Bool	GetVerboseProcessorInformation(CPUINFOEX& cpudata);

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
	u32 UpdateInterval = (u32)TLTime::GetUpdateTimeMilliSecsf();

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
		UpdateInterval = 1;
		g_TimerUpdateID = (u32)SetTimer( NULL, 0, UpdateInterval, TLCore::Platform::UpdateTimerCallback );
		if ( g_TimerUpdateID == 0 )
		{
			TLDebug::Platform::CheckWin32Error();
			return SyncFalse;
		}
	}

	return SyncTrue;
}

#ifdef _DEBUG
void expand(int avail, int mask)
{
	TTempString cpustr;

    if (mask & _CPU_FEATURE_MMX) 
	{
		cpustr.Appendf("\t%s\t_CPU_FEATURE_MMX", avail & _CPU_FEATURE_MMX ? "yes" : "no");
	    TLDebug_Print(cpustr);
		cpustr.Empty();

    }
    if (mask & _CPU_FEATURE_SSE) 
	{
		cpustr.Appendf("\t%s\t_CPU_FEATURE_SSE", avail & _CPU_FEATURE_SSE ? "yes" : "no");
	    TLDebug_Print(cpustr);
		cpustr.Empty();
    }
    if (mask & _CPU_FEATURE_SSE2) 
	{
		cpustr.Appendf("\t%s\t_CPU_FEATURE_SSE2", avail & _CPU_FEATURE_SSE2 ? "yes" : "no");
	    TLDebug_Print(cpustr);
		cpustr.Empty();
    }
    if (mask & _CPU_FEATURE_3DNOW) 
	{
		cpustr.Appendf("\t%s\t_CPU_FEATURE_3DNOW", avail & _CPU_FEATURE_3DNOW ? "yes" : "no");
	    TLDebug_Print(cpustr);
		cpustr.Empty();
    }
}


#endif


typedef BOOL (WINAPI *LPFN_GLPI)(
    PSYSTEM_LOGICAL_PROCESSOR_INFORMATION, 
    PDWORD);

// Helper function to count set bits in the processor mask.
WORD CountSetBits(ULONG_PTR bitMask)
{
    DWORD LSHIFT = sizeof(ULONG_PTR)*8 - 1;
    WORD bitSetCount = 0;
    ULONG_PTR bitTest = (ULONG_PTR)1 << LSHIFT;    
    DWORD i;
    
    for (i = 0; i <= LSHIFT; ++i)
    {
        bitSetCount += ((bitMask & bitTest)?1:0);
        bitTest/=2;
    }

    return bitSetCount;
}

Bool TLCore::Platform::GetVerboseProcessorInformation(CPUINFOEX& cpudata)
{
	LPFN_GLPI glpi;
    BOOL done = FALSE;
    PSYSTEM_LOGICAL_PROCESSOR_INFORMATION buffer = NULL;
    PSYSTEM_LOGICAL_PROCESSOR_INFORMATION ptr = NULL;
    DWORD returnLength = 0;
    
	WORD logicalProcessorCount = 0;
    WORD numaNodeCount = 0;
    WORD processorCoreCount = 0;
    WORD processorL1CacheCount = 0;
    WORD processorL2CacheCount = 0;
    WORD processorL3CacheCount = 0;
    WORD processorPackageCount = 0;

    DWORD byteOffset = 0;
    PCACHE_DESCRIPTOR Cache;

    glpi = (LPFN_GLPI) GetProcAddress(
                            GetModuleHandle(TEXT("kernel32")),
                            "GetLogicalProcessorInformation");
    if (NULL == glpi) 
    {
        TLDebug_Print("GetLogicalProcessorInformation is not supported.");
        return FALSE;
    }

    while (!done)
    {
        DWORD rc = glpi(buffer, &returnLength);

        if (FALSE == rc) 
        {
            if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) 
            {
                if (buffer) 
                    free(buffer);

                buffer = (PSYSTEM_LOGICAL_PROCESSOR_INFORMATION)malloc(
                        returnLength);

                if (NULL == buffer) 
                {
                    TLDebug_Print("Error: Allocation failure in ");
                    return FALSE;
                }
            } 
            else 
            {
#ifdef _DEBUG
				TTempString errstr;
				errstr.Appendf("Error %d", GetLastError());
                TLDebug_Print(errstr);
#endif
                return FALSE;
            }
        } 
        else
        {
            done = TRUE;
        }
    }

    ptr = buffer;

    while (byteOffset + sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION) <= returnLength) 
    {
        switch (ptr->Relationship) 
        {
        case RelationNumaNode:
            // Non-NUMA systems report a single record of this type.
            numaNodeCount++;
            break;

        case RelationProcessorCore:
            processorCoreCount++;

            // A hyperthreaded core supplies more than one logical processor.
            logicalProcessorCount += CountSetBits(ptr->ProcessorMask);
            break;

        case RelationCache:
            // Cache data is in ptr->Cache, one CACHE_DESCRIPTOR structure for each cache. 
            Cache = &ptr->Cache;
            if (Cache->Level == 1)
            {
                processorL1CacheCount++;
            }
            else if (Cache->Level == 2)
            {
                processorL2CacheCount++;
            }
            else if (Cache->Level == 3)
            {
                processorL3CacheCount++;
            }
            break;

        case RelationProcessorPackage:
            // Logical processors share a physical package.
            processorPackageCount++;
            break;

        default:
            TLDebug_Print("Error: Unsupported LOGICAL_PROCESSOR_RELATIONSHIP value");
            break;
        }
        byteOffset += sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION);
        ptr++;
    }

	// Update the structure
	cpudata.NUMAnodes = numaNodeCount;
	cpudata.numProcessorPackages = processorPackageCount;
	cpudata.numProcessorCores = processorCoreCount;
	cpudata.numLogicalProcessors = logicalProcessorCount;

	cpudata.numL1Caches = processorL1CacheCount;
	cpudata.numL2Caches = processorL2CacheCount;
	cpudata.numL3Caches = processorL3CacheCount;
    
    free(buffer);

	return TRUE;
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

	SYSTEM_INFO SysInfo;
	GetSystemInfo(&SysInfo);

#ifdef _DEBUG
	TTempString cpustr;
	cpustr.Appendf("Physical CPU Count:\t\t%d", SysInfo.dwNumberOfProcessors);
    TLDebug_Print(cpustr);
	cpustr.Empty();
#endif

	CPUINFOEX morecpudata;

	if(GetVerboseProcessorInformation(morecpudata))
	{

#ifdef _DEBUG
		cpustr.Appendf("Core count:\t\t%d", morecpudata.numProcessorCores);
		TLDebug_Print(cpustr);
		cpustr.Empty();

		cpustr.Appendf("Logical Processor count:\t\t%d", morecpudata.numLogicalProcessors);
		TLDebug_Print(cpustr);
		cpustr.Empty();
#endif
		// Number of processors
		Data.ExportData("CPU#", morecpudata.numLogicalProcessors);
	}
	else
	{
		// Number of processors
		Data.ExportData("CPU#", SysInfo.dwNumberOfProcessors);
	}

	//More detailed CPU info
	_p_info info;

	_cpuid(&info);

#ifdef _DEBUG
	cpustr.Appendf("Name:\t\t%s", info.v_name);
    TLDebug_Print(cpustr);
	cpustr.Empty();

	cpustr.Appendf("Model:\t\t%s", info.model_name);
    TLDebug_Print(cpustr);
	cpustr.Empty();

	cpustr.Appendf("Family:\t\t%d", info.family);
    TLDebug_Print(cpustr);
	cpustr.Empty();

	cpustr.Appendf("Model:\t\t%d", info.model);
    TLDebug_Print(cpustr);
	cpustr.Empty();

	cpustr.Appendf("Stepping:\t%d", info.stepping);
    TLDebug_Print(cpustr);
	cpustr.Empty();

	cpustr.Appendf("Feature:\t%08x", info.feature);
    TLDebug_Print(cpustr);
	cpustr.Empty();

    expand(info.feature, info.checks);

	cpustr.Appendf("OS Support:\t%08x", info.os_support);
    TLDebug_Print(cpustr);
	cpustr.Empty();

    expand(info.os_support, info.checks);

	cpustr.Appendf("Checks:\t\t%08x", info.checks);
    TLDebug_Print(cpustr);
	cpustr.Empty();
#endif

	// Can also use IsProcessorFeaturePresent() to test for specific features of the processor if required.

	// OS Version info

	OSVERSIONINFO OSInfo;

	OSInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

	GetVersionEx(&OSInfo);

	TTempString OS("Unknown");
	TTempString OSVer("Unknown");

	switch(OSInfo.dwPlatformId)
	{
		case VER_PLATFORM_WIN32s:
			{
				// A version of windows
				OS = "Windows";
				OSVer.Appendf("%d.%d", OSInfo.dwMajorVersion, OSInfo.dwMinorVersion);
			}
			break;
		case VER_PLATFORM_WIN32_WINDOWS:
			{
				OS = "Windows";

				switch(OSInfo.dwMinorVersion)
				{
					case 0:	
						// Windows 95
						OSVer = "95";
						break;
					case 10:
						// Windows 98
						OSVer = "98";
						break;
					case 90:
						// Windows Me
						OSVer = "Me";
						break;
				}
			}
			break;
		case VER_PLATFORM_WIN32_NT:
			{
				OS = "Windows";

				if(OSInfo.dwMajorVersion == 5)
				{
					if(OSInfo.dwMinorVersion == 0)
					{
						// Windows 2000
						OSVer = "2000";
					}
					else if(OSInfo.dwMinorVersion == 1)
					{
						// Windows XP
						OSVer = "XP";
					}
				}
				else if(OSInfo.dwMajorVersion <= 4)
				{
					OSVer = "NT";
				}
				else
				{
					// Unknown version so append the version info
					OSVer.Appendf("%d.%d", OSInfo.dwMajorVersion, OSInfo.dwMinorVersion);
				}
			}
			break;
			/*
		case VER_PLATFORM_WIN32_CE:
			{
				OS = "Windows";
				OSVer = "CE";
			}
			break;
			*/
	}

	// Print the OS details
	TLDebug_Print(OS);
	TLDebug_Print(OSVer);

#ifdef _DEBUG
	// Print out the major and minor version anyway
	TTempString osstr;
	osstr.Appendf("Major: %d \nMinor:%d", OSInfo.dwMajorVersion, OSInfo.dwMinorVersion);
	TLDebug_Print(osstr);

	// Print out any extra data such as service pack info
	TLDebug_Print(OSInfo.szCSDVersion);

#endif

	// Add the OS to the data
	Data.ExportData("OS", OS);
	Data.ExportData("OSVer", OSVer);

	/////////////////////////////////////////////////////////////
	TLDebug_Print("End Device Information");
}

void TLCore::Platform::QueryLanguageInformation(TBinaryTree& Data)	
{
	TLDebug_Print("Language Information:");

	/////////////////////////////////////////////////////////////
	// Langauge
	/////////////////////////////////////////////////////////////

	LANGID UserLanguage = GetUserDefaultUILanguage();

	WORD PrimaryLanguage = PRIMARYLANGID(UserLanguage);
	WORD SubLanguage = SUBLANGID(UserLanguage);
#ifdef _DEBUG
	// Print the primary and sub anguage ID's
	TTempString languagestr;
	languagestr.Appendf("Primary: %d", PrimaryLanguage);
	TLDebug_Print(languagestr);

	languagestr.Empty();
	languagestr.Appendf("Secondary: %d", SubLanguage);
	TLDebug_Print(languagestr);
#endif	

	// Convert the language ID into a TRef we can use
	// Default to english
	TRef LanguageRef = "eng";

	switch(PrimaryLanguage)
	{
		case LANG_ENGLISH:
			{
				switch(SubLanguage)
				{
					case SUBLANG_ENGLISH_US: 
						LanguageRef = "usa";
						break;
					case SUBLANG_ENGLISH_UK:
						LanguageRef = "eng";
						break;

					default:
						TLDebug_Print("Hardware langauge not supported - defaulting to english");
						LanguageRef = "eng";
						break;
				}
			}
			break;
		case LANG_FRENCH:
			LanguageRef = "fre";
			break;
		case LANG_GERMAN:
			LanguageRef = "ger";
			break;
		case LANG_ITALIAN:
			LanguageRef = "ita";
			break;
		case LANG_SPANISH:
			LanguageRef = "spa";
			break;
		case SUBLANG_DUTCH:
			LanguageRef = "ned";
			break;
		case LANG_JAPANESE:
			LanguageRef = "jap";
			break;
		default:
			TLDebug_Print("Hardware langauge not supported - defaulting to english");
			break;
	}
	
	// Export the users language to the data - actual language selection will be done 
	// via the core manager
	Data.ExportData("Language", LanguageRef);
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
				//	ready for an update - break out so we can do an update
				if ( TLCore::g_pCoreManager->IsReadyForUpdate() )
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

	if ( TLCore::g_pCoreManager )
		TLCore::g_pCoreManager->SetReadyForUpdate();
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

	//	ready for another update
	if ( TLCore::g_pCoreManager )
		TLCore::g_pCoreManager->SetReadyForUpdate();
}
