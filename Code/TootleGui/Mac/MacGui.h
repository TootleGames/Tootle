/*
 *  MacGui.h
 *  TootleGui
 *
 *  Created by Graham Reeves on 17/02/2010.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */
#include "../TLGui.h"


namespace TLGui
{
	namespace Platform
	{
		int2			GetScreenMousePosition(TLGui::TWindow& Window,u8 MouseIndex);
		void			GetDesktopSize(Type4<s32>& DesktopSize);	//	get the desktop dimensions. note: need a window so we can decide which desktop?	
	}

}
