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
		
		namespace IPod
		{			
			typedef enum TouchPhase
			{
				Begin = 0,
				Move,
				End,
				Cancel,
			};
			
			struct TTouchData
			{
				int2	uCurrentPos;
				int2	uPreviousPos;
				u32		uTimestamp;
				u8		uPhase;
				u8		uIndex;
			};
			
			struct TAccelerationData
			{
				float3 vAcceleration;
			};
			
			void ProcessTouchBegin(TPtr<TTouchData>& touchData);
			void ProcessTouchMoved(TPtr<TTouchData>& touchData);
			void ProcessTouchEnd(TPtr<TTouchData>& touchData);

			void ProcessAcceleration(TPtr<TAccelerationData>& pAccelerationData);

			Bool CreateDevice();
			Bool InitialiseDevice(TPtr<TInputDevice> pDevice);
		}
	};
};

