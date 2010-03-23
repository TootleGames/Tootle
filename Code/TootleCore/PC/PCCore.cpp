/*------------------------------------------------------
	PC Platform core

	contains the program entry

------------------------------------------------------*/
#include "PCCore.h"
#include "PCDebug.h"
#include "../TLCore.h"
#include "../TLTypes.h"
#include "../TString.h"
#include "PCTime.h"
#include "../TCoreManager.h"
#include <mmsystem.h>


//--------------------------------------------------
//	platform thread/process sleep
//--------------------------------------------------
void TLCore::Platform::Sleep(u32 Millisecs)
{
	::Sleep( Millisecs );
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
    TLDebug_Print( TString( "Physical CPU Count:\t\t%d", SysInfo.dwNumberOfProcessors ) );
/*
	CPUINFOEX morecpudata;

	if(GetVerboseProcessorInformation(morecpudata))
	{
		TLDebug_Print( TString("Core count:\t\t%d", morecpudata.numProcessorCores) );
		TLDebug_Print( TString("Logical Processor count:\t\t%d", morecpudata.numLogicalProcessors) );
		
		// Number of processors
		TLDebug_Print( TString("CPU_", morecpudata.numLogicalProcessors) );
	}
	else
	{
		// Number of processors
		Data.ExportData("CPU_", SysInfo.dwNumberOfProcessors);
	}

	//More detailed CPU info
	_p_info info;

	_cpuid(&info);
    TLDebug_Print( TString("Name:\t\t%s", info.v_name) );
	TLDebug_Print( TString("Model:\t\t%s", info.model_name) );
	TLDebug_Print( TString("Family:\t\t%d", info.family) );
	TLDebug_Print( TString("Model:\t\t%d", info.model) );
	TLDebug_Print( TString("Stepping:\t%d", info.stepping) );
	TLDebug_Print( TString("Feature:\t%08x", info.feature) );

    expand(info.feature, info.checks);
	TLDebug_Print( TString("OS Support:\t%08x", info.os_support ) );

    expand(info.os_support, info.checks);
	TLDebug_Print( TString("Checks:\t\t%08x", info.checks ) );
*/
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

	// Print out the major and minor version anyway
	TLDebug_Print( TString("Major: %d \nMinor:%d", OSInfo.dwMajorVersion, OSInfo.dwMinorVersion) );

	// Print out any extra data such as service pack info
	TLDebug_Print(OSInfo.szCSDVersion);

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

	// Print the primary and sub anguage ID's
	TLDebug_Print( TString("Primary: %d", PrimaryLanguage ) );
	TLDebug_Print( TString("Secondary: %d", SubLanguage ) );

	// Convert the language ID into a TRef we can use
	// Default to english
	TRef LanguageRef = "eng";

	switch(PrimaryLanguage)
	{
		case LANG_ENGLISH:
			{
				switch(SubLanguage)
				{
					default:
						TLDebug_Print("Hardware langauge not supported - defaulting to english");
					case SUBLANG_ENGLISH_UK:	
						LanguageRef = "eng";	
						break;
					
					case SUBLANG_ENGLISH_US: 	
						LanguageRef = "usa";	
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
			TRef LanguageRef = "eng";
			break;
	}
	
	// Export the users language to the data - actual language selection will be done 
	// via the core manager
	Data.ExportData("Language", LanguageRef);
}
