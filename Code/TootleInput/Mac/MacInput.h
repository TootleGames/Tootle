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
		
		Bool			UpdateDevice(TInputDevice& Device);
		int2			GetCursorPosition(u8 uIndex);
		
		SyncBool		EnumerateDevices();
		void			RemoveAllDevices();
		
		
		SyncBool		CreateVirtualDevice(TRefRef InstanceRef, TRefRef DeviceTypeRef);
		SyncBool		RemoveVirtualDevice(TRefRef InstanceRef);

		// TEMP test routine
		void TestVibrateDevice();
				
		namespace Mac
		{			
			Bool CreateDevice();
			Bool InitialiseDevice(TPtr<TInputDevice> pDevice);
		}
	};
};
