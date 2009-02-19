#pragma once

#include <TootleCore/TLCore.h>

#include "../TLInput.h"


namespace TLInput
{
	namespace Platform
	{
		SyncBool		Init();
		SyncBool		Update();
		SyncBool		Shutdown();
		
		Bool			UpdateDevice(TPtr<TInputDevice> pDevice);
		int2			GetCursorPosition(u8 uIndex);
		
		SyncBool		EnumerateDevices();
		void			RemoveAllDevices();
		
		namespace Mac
		{			
			Bool CreateDevice();
			Bool InitialiseDevice(TPtr<TInputDevice> pDevice);
		}
	};
};
