/*

	The GUI application is the OS interface to an app. It controls main,
	and the OS update loop. The application will dictate the timer or timing
	thread or app delegate, or dumb while(1) loop depending on the OS implementation.

	The core knows nothing of this, the core manager just knows when an update is
	required (each timer callback will mark the core to update) and the OS will invoke
	a core update as required.

	The application will own the core manager, but the core manager will control all 
	the other managers

	There is no project overload of this, it is OS specific, not project specific.

	At some point it will be maluable by the project (for non-window stuff) but that will be
	implemented as it is required.
*/
#pragma once
#include "TLGui.h"



namespace TLGui
{
	class TApp;	//	base application type
}

//------------------------------------------------------
//	base application type 
//------------------------------------------------------
class TLGui::TApp
{
public:
	TApp()	{}
	virtual ~TApp()	{}
	
protected:
};



