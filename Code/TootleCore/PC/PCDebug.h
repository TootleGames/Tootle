

#pragma once

#include "../TLTypes.h"

//	forward declarations
class TString;

namespace TLDebug
{
	namespace Platform
	{
		SyncBool	Initialise();
		SyncBool	Shutdown();


		void		PrintToBuffer(const TString& String);	//	platform specific debug output - buffered
		Bool		Break(const TString& String);	//	return FALSE to stop app, TRUE and will attempt to continue
		Bool		CheckWin32Error();				//	checks for a win32 error and does a break
	}
};
