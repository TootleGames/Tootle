/*
 *  IPodGui.h
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
		SyncBool		Init();
		SyncBool		Shutdown();

		int2			GetScreenMousePosition(TLGui::TWindow& Window,u8 MouseIndex);
		void			GetDesktopSize(Type4<s32>& DesktopSize);
	}

}
