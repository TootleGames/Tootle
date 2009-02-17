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
			// Base touch data - used for the persistent TouchObject class and as temp data passed form the
			// ipod touch system to the engine input system
			class TTouchData
			{
			public:
				enum TouchPhase
				{
					Begin = 0,
					Move,
					End,
					Cancel,
				};
				
			public:
				int2						uCurrentPos;			// Current position of the touch event
				int2						uPreviousPos;			// Previous position of the touch event
				float						fTimestamp;				// Timestamp
				TouchPhase					uPhase;					// Currrent touch phase 
				u8							uTapCount;				// Number of taps for a particular touch event
			};
			
			// Engine side persistent touch object class
			class TTouchObject : public TTouchData
			{
			public:
				enum TouchFlags
				{
					Changed = 0,
					Remove,	
				};
				
			public:
				int2						uStartPosition;			// Starting position of the touch object
				TFlags<TouchFlags, u8>		uFlags;					// Flags for this touh obejct
			};
						
			void ProcessTouchBegin(const TTouchData& touchData);
			void ProcessTouchMoved(const TTouchData& touchData);
			void ProcessTouchEnd(const TTouchData& touchData);

			void ProcessAcceleration(const float3& vAccelerationData);

			Bool CreateDevice();
			Bool InitialiseDevice(TPtr<TInputDevice> pDevice);
		}
	};
};


TLCore_DeclareIsDataType( TLInput::Platform::IPod::TTouchData );
