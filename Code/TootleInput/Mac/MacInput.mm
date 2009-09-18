#include "MacInput.h"

#import <IOKit/IOKitLib.h>
#import <IOKit/hid/IOHIDManager.h>
#import <IOKit/hid/IOHIDKeys.h>
#import <Appkit/NSCursor.h>


#include <TootleCore/TLMemory.h> // TEMP



#ifdef _DEBUG
	#define ENABLE_INPUTSYSTEM_TRACE
#endif

// [16/12/08] DB -	Possible responsiveness improvement
//					Calls the update manually from the touch events which should mean the inptu is processed immediately
//				    instead of during the next frame update
#define ENABLE_IMMEDIATE_TOUCHUPDATE

#define ACCEL_MAXPROCESS	10		//	at most, per device update only process the last N accelerometer data's
#define ACCEL_MINCHANGE		0.025f	//	minimum amount of change on an axis to register a change. anything smaller than this will be ignored and "jitter"



namespace TLInput
{
	namespace Platform 
	{
		namespace HID 
		{						
			void	SetCursorPosition(u8 uIndex, int2 uPos);			
			
			const u32 MAX_CURSOR_POSITIONS = 1;
			
			IOHIDManagerRef			g_IOHIDManagerRef = NULL;
			
			TPtrArray<TLInputHIDDevice>			g_TLHIDDevices;					// Global array of Mac specific HID device pointers

			TKeyArray<u32, TRef>	g_KeyboardRefMap;
			TKeyArray<u32, TRef>	g_MouseRefMap;
			
			// For now we have specifically created keyarrays for the extra button references on gamepads
			// Eventually this should be an array of arrays where we can 'plugin' the pads supported
			// via an XML data file rather than using specific initialise routines.
			// This could also apply to the keyboard and mouse ref maps as they will owrk in a similar fashion
			TKeyArray<u32, TRef>					g_Xbox360PadButtonRefs;		// XBox 360 pad button refs
			TKeyArray<u32, TRef>					g_WiimoteButtonRefs;		// Wiimote button refs
			TKeyArray<u32, TRef>					g_PS3PadButtonRefs;			// PS3 pad button refs
			

			TKeyArray<u32, TRef>					g_HIDDeviceProductIDRefs;	//	table mapping HID device ProductID to TRefs - the Guids are not always valid TRef's (we use 30 bits exclusivly) - so this keeps an internal Ref<->Guid map

			
			// Internal
			void	InitialiseKeyboadRefMap();
			void	InitialiseMouseRefMap();
			void	InitialiseGamePadRefMaps();
			
			void	InitialiseXBox360PadRefMap();
			void	InitialiseWiimoteRefMap();
			void	InitialisePS3PadRefMap();
			
			Bool	GetSpecificButtonRef(const u32& uButtonID, TRefRef DeviceType, const u32& uProductID, TRef& LabelRef);
			
			u32		GetDeviceProductIDFromRef(TRefRef DeviceRef);				//	get the ProductID thats mapped to this ref. ZERO returned is assumed invalid
			TRef	GetDeviceRefFromProductID(u32 ProductID,Bool CreateNew);	//	get the ref thats mapped to this device ProductID. Optionally create a new entry
			
			
			Bool			InitialisePhysicalDevice(TPtr<TInputDevice> pDevice, TRefRef DeviceTypeRef);
			Bool			InitialiseVirtualDevice(TPtr<TInputDevice> pDevice, TRefRef DeviceTypeRef);
			
			Bool			UpdatePhysicalDevice(TLInput::TInputDevice& Device);
			Bool			UpdateVirtualDevice(TLInput::TInputDevice& Device) { return FALSE; }
			
			
			// Callback routines
			void			DeviceEnumerateCallback(void* context, IOReturn result,  void* sender, IOHIDDeviceRef device);
			void			DeviceRemovedCallback(void* context, IOReturn result,  void* sender, IOHIDDeviceRef device);

			void			DeviceReportCallback(void* inContext, IOReturn inResult, void* inSender, IOHIDReportType inType, uint32_t inReportID, uint8_t* inReport, CFIndex inReportLength);

			void			EnumDeviceObject(void* context, IOReturn result, void* sender, IOHIDReportType type, uint32_t reportID, uint8_t* report, CFIndex reportLength);

			/*
			// Platform specific device creation
			void			CreateDevice_Keyboard(const IOHIDDeviceRef device);
			void			CreateDevice_Mouse(const IOHIDDeviceRef device);
			void			CreateDevice_Gamepad(const IOHIDDeviceRef device);
			 */	
			
			// Special update routines for devices in Mac OS HID.  Hopefully at some point I can remove these as this shouldn't really be necessary.
			Bool UpdateHIDDevice_Mouse(TInputDevice& Device, TLInputHIDDevice& TLHIDDevice);
			Bool UpdateHIDDevice_Keyboard(TInputDevice& Device, TLInputHIDDevice& TLHIDDevice);
			Bool UpdateHIDDevice_Gamepad(TInputDevice& Device, TLInputHIDDevice& TLHIDDevice);
			
			//	for an axis, consolidate all the movement in an axis to one movement
			Bool		ConslidateAxisMovement(TArray<TInputData>& InputBuffer,TRefRef AxisSensorRef);
	

			// Vendor ID definitions
			const u32 Vendor_Nintendo	= 0x057e;
			const u32 Vendor_Microsoft	= 0x045e;			
			const u32 Vendor_Apple		= 0x0000;			
			const u32 Vendor_Sony		= 0x0000;			
			
			// Device ID definitions
			const u32 Gamepad_Wiimote	= 0x0306;
			const u32 Gamepad_Xbox360	= 0x028e;
			const u32 Gamepad_PS3		= 0x0000;
			
		}
	}
}


using namespace TLInput;

// Not used on the ipod
SyncBool Platform::HID::Init()
{	
	if(g_IOHIDManagerRef != NULL)
		return SyncTrue;
	
	InitialiseKeyboadRefMap();
	InitialiseMouseRefMap();
	
	InitialiseGamePadRefMaps();
	
	
	// create the manager
	g_IOHIDManagerRef = IOHIDManagerCreate( kCFAllocatorDefault, kIOHIDOptionsTypeNone );

	if( !g_IOHIDManagerRef )
	{
		TLDebug_Print("Failed to create HID manager");
		return SyncFalse;
	}
		
	// Register callbacks
	IOHIDManagerRegisterDeviceMatchingCallback(g_IOHIDManagerRef, DeviceEnumerateCallback, 0);
	IOHIDManagerRegisterDeviceRemovalCallback(g_IOHIDManagerRef, DeviceRemovedCallback, 0);

	CFDictionaryRef matching = NULL;
	IOHIDManagerSetDeviceMatching( g_IOHIDManagerRef, matching);
	
	IOHIDManagerScheduleWithRunLoop( g_IOHIDManagerRef, CFRunLoopGetCurrent( ), kCFRunLoopDefaultMode );
	
	// Open the HID manager
	IOReturn result = IOHIDManagerOpen( g_IOHIDManagerRef, kIOHIDOptionsTypeNone);
	if ( kIOReturnSuccess != result ) 
	{
		TLDebug_Print("Failed to open HID manager");
		return SyncFalse;
	} 
	
	return SyncTrue;	
}


void Platform::HID::InitialiseKeyboadRefMap()
{
	g_KeyboardRefMap.Add(kHIDUsage_KeyboardEscape, "K_ESC");
	g_KeyboardRefMap.Add(kHIDUsage_Keyboard1, "K_1");
	g_KeyboardRefMap.Add(kHIDUsage_Keyboard2, "K_2");
	g_KeyboardRefMap.Add(kHIDUsage_Keyboard3, "K_3");
	g_KeyboardRefMap.Add(kHIDUsage_Keyboard4, "K_4");
	g_KeyboardRefMap.Add(kHIDUsage_Keyboard5, "K_5");
	g_KeyboardRefMap.Add(kHIDUsage_Keyboard6, "K_6");
	g_KeyboardRefMap.Add(kHIDUsage_Keyboard7, "K_7");
	g_KeyboardRefMap.Add(kHIDUsage_Keyboard8, "K_8");
	g_KeyboardRefMap.Add(kHIDUsage_Keyboard9, "K_9");
	g_KeyboardRefMap.Add(kHIDUsage_Keyboard0, "K_0");
	g_KeyboardRefMap.Add(kHIDUsage_KeyboardHyphen, "K_MINUS");
	g_KeyboardRefMap.Add(kHIDUsage_KeyboardEqualSign, "K_EQUALS");
	g_KeyboardRefMap.Add(kHIDUsage_KeyboardDeleteOrBackspace, "K_BACKSPACE");
	g_KeyboardRefMap.Add(kHIDUsage_KeyboardTab, "K_TAB");
	g_KeyboardRefMap.Add(kHIDUsage_KeyboardQ, "K_Q");
	g_KeyboardRefMap.Add(kHIDUsage_KeyboardW, "K_W");
	g_KeyboardRefMap.Add(kHIDUsage_KeyboardE, "K_E");
	g_KeyboardRefMap.Add(kHIDUsage_KeyboardR, "K_R");
	g_KeyboardRefMap.Add(kHIDUsage_KeyboardT, "K_T");
	g_KeyboardRefMap.Add(kHIDUsage_KeyboardY, "K_Y");
	g_KeyboardRefMap.Add(kHIDUsage_KeyboardU, "K_U");
	g_KeyboardRefMap.Add(kHIDUsage_KeyboardI, "K_I");
	g_KeyboardRefMap.Add(kHIDUsage_KeyboardO, "K_O");
	g_KeyboardRefMap.Add(kHIDUsage_KeyboardP, "K_P");
	g_KeyboardRefMap.Add(kHIDUsage_KeyboardOpenBracket, "K_LBRACKET");
	g_KeyboardRefMap.Add(kHIDUsage_KeyboardCloseBracket, "K_RBRACKET");
	g_KeyboardRefMap.Add(kHIDUsage_KeyboardReturnOrEnter, "K_RETURN");
	g_KeyboardRefMap.Add(kHIDUsage_KeyboardLeftControl, "K_LCTRL");
	g_KeyboardRefMap.Add(kHIDUsage_KeyboardA, "K_A");
	g_KeyboardRefMap.Add(kHIDUsage_KeyboardS, "K_S");
	g_KeyboardRefMap.Add(kHIDUsage_KeyboardD, "K_D");
	g_KeyboardRefMap.Add(kHIDUsage_KeyboardF, "K_F");
	g_KeyboardRefMap.Add(kHIDUsage_KeyboardG, "K_G");
	g_KeyboardRefMap.Add(kHIDUsage_KeyboardH, "K_H");
	g_KeyboardRefMap.Add(kHIDUsage_KeyboardJ, "K_J");
	g_KeyboardRefMap.Add(kHIDUsage_KeyboardK, "K_K");
	g_KeyboardRefMap.Add(kHIDUsage_KeyboardL, "K_L");
	g_KeyboardRefMap.Add(kHIDUsage_KeyboardSemicolon, "K_COLON");
	g_KeyboardRefMap.Add(kHIDUsage_KeyboardQuote, "K_APOSTROPHE");
	g_KeyboardRefMap.Add(kHIDUsage_KeyboardGraveAccentAndTilde, "K_GRAVE");
	g_KeyboardRefMap.Add(kHIDUsage_KeyboardLeftShift, "K_LSHIFT");
	g_KeyboardRefMap.Add(kHIDUsage_KeyboardBackslash, "K_BACKSLASH");
	g_KeyboardRefMap.Add(kHIDUsage_KeyboardZ, "K_Z");
	g_KeyboardRefMap.Add(kHIDUsage_KeyboardX, "K_X");
	g_KeyboardRefMap.Add(kHIDUsage_KeyboardC, "K_C");
	g_KeyboardRefMap.Add(kHIDUsage_KeyboardV, "K_V");
	g_KeyboardRefMap.Add(kHIDUsage_KeyboardB, "K_B");
	g_KeyboardRefMap.Add(kHIDUsage_KeyboardN, "K_N");
	g_KeyboardRefMap.Add(kHIDUsage_KeyboardM, "K_M");
	g_KeyboardRefMap.Add(kHIDUsage_KeypadComma, "K_,");
	g_KeyboardRefMap.Add(kHIDUsage_KeyboardPeriod, "K_.");
	g_KeyboardRefMap.Add(kHIDUsage_KeyboardSlash, "K_FORWARDSLASH");
	g_KeyboardRefMap.Add(kHIDUsage_KeyboardRightShift, "K_RSHIFT");
	g_KeyboardRefMap.Add(kHIDUsage_KeypadAsterisk, "K_MULTIPLY");
	g_KeyboardRefMap.Add(kHIDUsage_KeyboardSpacebar, "K_SPACE");
	g_KeyboardRefMap.Add(kHIDUsage_KeyboardCapsLock, "K_CAPS");
	g_KeyboardRefMap.Add(kHIDUsage_KeyboardRightArrow, "K_RIGHT");
	g_KeyboardRefMap.Add(kHIDUsage_KeyboardLeftArrow, "K_LEFT");
	g_KeyboardRefMap.Add(kHIDUsage_KeyboardDownArrow, "K_DOWN");
	g_KeyboardRefMap.Add(kHIDUsage_KeyboardUpArrow, "K_UP");
}

void Platform::HID::InitialiseMouseRefMap()
{
	g_MouseRefMap.Add(kHIDUsage_Button_1, "LMB");
	g_MouseRefMap.Add(kHIDUsage_Button_2, "RMB");
	g_MouseRefMap.Add(kHIDUsage_Button_3, "WHEEL");
	g_MouseRefMap.Add(kHIDUsage_Button_4, "LMB2");
	g_MouseRefMap.Add(0x05, "RMB2");
}



void Platform::HID::InitialiseGamePadRefMaps()
{
	//TODO: Add individual arrays per gamepad type we support
	// These should be added to a generic array or manager or something rather than having a g_XXXarray
	// then we can have a list of pads that are supported too for easy identification and setup
	InitialiseXBox360PadRefMap();
	InitialiseWiimoteRefMap();
	InitialisePS3PadRefMap();
}

void Platform::HID::InitialiseXBox360PadRefMap()
{
	/*
	 enum XBox360_ButtonFlags
	 04.{
	 05.XBOX360PAD_DPAD_UP = 1,
	 06.XBOX360PAD_DPAD_DOWN = 2,
	 07.XBOX360PAD_DPAD_LEFT = 4,
	 08.XBOX360PAD_DPAD_RIGHT = 8,
	 09.XBOX360PAD_START = 16,
	 10.XBOX360PAD_BACK = 32,
	 11.XBOX360PAD_LAXIS = 64,
	 12.XBOX360PAD_RAXIS = 128,
	 13.XBOX360PAD_LB = 256,
	 14.XBOX360PAD_RB = 512,
	 15.XBOX360PAD_XBOX = 1024,
	 16.XBOX360PAD_A = 4096,
	 17.XBOX360PAD_B = 8192,
	 18.XBOX360PAD_X = 16384,
	 19.XBOX360PAD_Y = 32768,
	 20.};
	 
	g_Xbox360PadButtonRefs.Add(DIJOFS_BUTTON0, "A");
	g_Xbox360PadButtonRefs.Add(DIJOFS_BUTTON1, "B");
	g_Xbox360PadButtonRefs.Add(DIJOFS_BUTTON2, "X");
	g_Xbox360PadButtonRefs.Add(DIJOFS_BUTTON3, "Y");
	g_Xbox360PadButtonRefs.Add(DIJOFS_BUTTON4, "START");
	g_Xbox360PadButtonRefs.Add(DIJOFS_BUTTON5, "BACK");
	g_Xbox360PadButtonRefs.Add(DIJOFS_BUTTON6, "RB");
	g_Xbox360PadButtonRefs.Add(DIJOFS_BUTTON7, "RT");
	g_Xbox360PadButtonRefs.Add(DIJOFS_BUTTON8, "LB");
	g_Xbox360PadButtonRefs.Add(DIJOFS_BUTTON9, "LT");
	 */
}

void Platform::HID::InitialiseWiimoteRefMap()
{
/*	
	g_WiimoteButtonRefs.Add(DIJOFS_BUTTON0, "A");
	g_WiimoteButtonRefs.Add(DIJOFS_BUTTON1, "1");
	g_WiimoteButtonRefs.Add(DIJOFS_BUTTON2, "2");
	g_WiimoteButtonRefs.Add(DIJOFS_BUTTON3, "B");
	g_WiimoteButtonRefs.Add(DIJOFS_BUTTON4, "PLUS");
	g_WiimoteButtonRefs.Add(DIJOFS_BUTTON5, "MINUS");
	g_WiimoteButtonRefs.Add(DIJOFS_BUTTON6, "HOME");
	g_WiimoteButtonRefs.Add(DIJOFS_BUTTON7, "POWER");
	g_WiimoteButtonRefs.Add(DIJOFS_BUTTON8, "C");
	g_WiimoteButtonRefs.Add(DIJOFS_BUTTON9, "Z");
 */
}

void Platform::HID::InitialisePS3PadRefMap()
{
	/*
	g_PS3PadButtonRefs.Add(DIJOFS_BUTTON0, "CROSS");
	g_PS3PadButtonRefs.Add(DIJOFS_BUTTON1, "CIRCLE");
	g_PS3PadButtonRefs.Add(DIJOFS_BUTTON2, "SQUARE");
	g_PS3PadButtonRefs.Add(DIJOFS_BUTTON3, "TRIANGLE");
	g_PS3PadButtonRefs.Add(DIJOFS_BUTTON4, "START");
	g_PS3PadButtonRefs.Add(DIJOFS_BUTTON5, "SELECT");
	g_PS3PadButtonRefs.Add(DIJOFS_BUTTON6, "ANALOG");
	g_PS3PadButtonRefs.Add(DIJOFS_BUTTON7, "R1");
	g_PS3PadButtonRefs.Add(DIJOFS_BUTTON8, "L1");
	g_PS3PadButtonRefs.Add(DIJOFS_BUTTON9, "R2");
	g_PS3PadButtonRefs.Add(DIJOFS_BUTTON10, "L2");
	g_PS3PadButtonRefs.Add(DIJOFS_BUTTON11, "R3");
	g_PS3PadButtonRefs.Add(DIJOFS_BUTTON12, "L3");
	 */
}


//---------------------------------------------------
//	get the ProductID thats mapped to this ref. ZERO returned is assumed invalid
//---------------------------------------------------
u32 Platform::HID::GetDeviceProductIDFromRef(TRefRef DeviceRef)
{
	const u32* pGuid = g_HIDDeviceProductIDRefs.FindKey( DeviceRef );
	if ( !pGuid )
		return 0;
	
	return *pGuid;
}

//---------------------------------------------------
//	get the ref thats mapped to this device ProductID. Optionally create a new entry
//---------------------------------------------------
TRef Platform::HID::GetDeviceRefFromProductID(u32 DeviceGuid,Bool CreateNew)
{
	const TRef* pRef = g_HIDDeviceProductIDRefs.Find( DeviceGuid );
	if ( pRef )
		return *pRef;
	
	//	no entry - and not creating a new one
	if ( !CreateNew )
		return TRef();
	
	//	get new device ref
	TRef NewDeviceRef = TLInput::GetFreeDeviceRef();
	pRef = g_HIDDeviceProductIDRefs.Add( DeviceGuid, NewDeviceRef );
	
	return *pRef;
}

void Platform::HID::DeviceEnumerateCallback(void* context, IOReturn result,  void* sender, IOHIDDeviceRef device)
{
	TLDebug_Print("HID Device enumerated");
	
	TTempString str("Device Ref: ");
	str.Appendf("%x", device);
	TLDebug_Print(str);
	str.Empty();
	
	
	//	get device ref mapped to this guid
	TRef InstanceRef = TLInput::Platform::HID::GetDeviceRefFromProductID( (u32)device, TRUE );
	
	//	if device already exists, skip over (as per comments above)
	if ( InstanceRef.IsValid() )
	{
		//	there is a GUID<->DeviceRef link, but check there is an ACTUAL device that's been created - we could have mapping, but no device
		TPtr<TLInput::TInputDevice>& pDevice = TLInput::GetDevice( InstanceRef );
		if ( pDevice )
		{
			// Device exists
			// return
			return;
		}
	}
	

	u32 usagePage = 0;
	s32 usage = 0;

	// Get the usage page	
	CFTypeRef tCFTypeRef = IOHIDDeviceGetProperty( device, CFSTR( kIOHIDPrimaryUsagePageKey ) );
    if ( tCFTypeRef ) 
	{
        // if this is a number
        if ( CFNumberGetTypeID( ) == CFGetTypeID( tCFTypeRef ) ) 
		{			
            // get its value
			CFNumberGetValue( ( CFNumberRef ) tCFTypeRef, kCFNumberSInt32Type, &usagePage );
        }
    }	
	
	// Get the usage key
	tCFTypeRef = IOHIDDeviceGetProperty( device, CFSTR( kIOHIDPrimaryUsageKey ) );
    if ( tCFTypeRef ) 
	{
        // if this is a number
        if ( CFNumberGetTypeID( ) == CFGetTypeID( tCFTypeRef ) ) 
		{
			
			CFNumberGetValue( ( CFNumberRef ) tCFTypeRef, kCFNumberSInt32Type, &usage );
        }
    }	
	

	str.Appendf("Usage Page: ");
	str.Appendf("0x%x", usagePage );
	TLDebug_Print(str);
	str.Empty();
	
	str.Appendf("Usage: ");
	str.Appendf("0x%x", usage );
	TLDebug_Print(str);
	str.Empty();
	
	Bool bProcessDevice = FALSE;
	TRef refDeviceType;

	switch(usagePage)
	{
		case kHIDPage_GenericDesktop:
		{
			// Check the usage
			switch(usage)
			{
			case kHIDUsage_GD_Pointer:
				TLDebug_Print("Device Type - Pointer");
				break;
			case kHIDUsage_GD_Mouse:
					//CreateDevice_Mouse(device);
					refDeviceType = TLInput::MouseRef;
					bProcessDevice = TRUE;
					break;					
			case kHIDUsage_GD_GamePad:
					//CreateDevice_Gamepad(device);
					refDeviceType = TLInput::GamepadRef;
					bProcessDevice = TRUE;
				break;
			case kHIDUsage_GD_Joystick:
				TLDebug_Print("Device Type - Joystick");
				break;
			case kHIDUsage_GD_Keyboard:
					//CreateDevice_Keyboard(device);
					refDeviceType = TLInput::KeyboardRef;
					bProcessDevice = TRUE;
					break;
					
			// Unknown usage		
			default:	
				TString str("Unhandled device usage:");
				str.Appendf("%d %d", usagePage, usage);
				TLDebug::Print(str);
				break;

			}
		}
		break;
	
		// Unused usage page
		default:	
			TString str("Unhandled device usage page:");
			str.Appendf("%d", usagePage);
			TLDebug::Print(str);			
			break;
			
	}
	
	
	if(bProcessDevice)
	{		
		// Create the generic input object
		TPtr<TInputDevice> pGenericDevice = g_pInputSystem->GetInstance(InstanceRef, TRUE, refDeviceType);
		
		if(pGenericDevice.IsValid())
		{
			if(!InitialiseDevice(pGenericDevice, device))
			{
				// Failed to initialise the input device data
				
				// Find the physical device from the platform based list and remove it
				for(u32 uIndex = 0; uIndex < g_TLHIDDevices.GetSize(); uIndex++)
				{
					TPtr<TLInputHIDDevice> pTLHIDDevice = g_TLHIDDevices.ElementAtConst(uIndex);
					
					if(pTLHIDDevice->GetHIDDevice() == device)
					{
						g_TLHIDDevices.ElementAt(uIndex) = NULL;
						g_TLHIDDevices.RemoveAt(uIndex);
						break;
					}
				}
				
				pGenericDevice = NULL;
				g_pInputSystem->RemoveInstance(InstanceRef);
			}
			else
			{
				// Notify to all subscribers of the input system that a new device was added
				TLMessaging::TMessage Message("DeviceChanged");
				Message.ExportData("State", TRef("ADDED"));					// state change
				Message.ExportData("DEVID", pGenericDevice->GetDeviceRef());	// device ID
				Message.ExportData("TYPE", pGenericDevice->GetDeviceType());					// device type
				
				g_pInputSystem->PublishMessage(Message);
			}
		}
	}
	else
	{
		TLDebug::Print("Unknown input device found - ignoring");
	}
	
}

// DB - TEST
static u32 HardwareDeviceID = 0x45;

Bool Platform::HID::InitialiseDevice(TPtr<TInputDevice> pDevice, const IOHIDDeviceRef device)
{
	// Generate a new device reference
	TRef HardwareDeviceRef = HardwareDeviceID;
	
	HardwareDeviceID++;
	
	// Create a platform specific device reference object
	TLInputHIDDevice* pHIDDevice = new TLInputHIDDevice(HardwareDeviceRef, device);
	
	if(!pHIDDevice)
		return FALSE;
	
	u32 uDeviceType = 0;
	u32 uProductID = 0;

	// Get the usage key - used as the device type
	CFTypeRef tCFTypeRef = IOHIDDeviceGetProperty( device, CFSTR( kIOHIDPrimaryUsageKey ) );
    if ( tCFTypeRef ) 
	{
        // if this is a number
        if ( CFNumberGetTypeID( ) == CFGetTypeID( tCFTypeRef ) ) 
		{
			
			CFNumberGetValue( ( CFNumberRef ) tCFTypeRef, kCFNumberSInt32Type, &uDeviceType );
        }
    }	
	
	// Get the product ID key
	tCFTypeRef = IOHIDDeviceGetProperty( device, CFSTR( kIOHIDProductIDKey ) );
    if ( tCFTypeRef ) 
	{
        // if this is a number
        if ( CFNumberGetTypeID( ) == CFGetTypeID( tCFTypeRef ) ) 
		{
			
			CFNumberGetValue( ( CFNumberRef ) tCFTypeRef, kCFNumberSInt32Type, &uProductID );
        }
    }	
	
	
	pHIDDevice->SetProductID(uProductID);	// Unique product ID (same for all instances of the same devices)
	
	// Set quick access to the device type without having to go through the HID interface
	pHIDDevice->SetDeviceType(uDeviceType);

	// Intiialise the report buffer
	pHIDDevice->InitialiseReportBuffer();
	
	pDevice->AssignToHardwareDevice(HardwareDeviceRef);
	
	g_TLHIDDevices.Add(pHIDDevice);
	
	// Enumerate device elements
	if(!pHIDDevice->EnumerateObjects())
		return FALSE;
		
	
	pDevice->SetAttached(TRUE);
		
	// Success
	return TRUE;
	
}


void Platform::HID::EnumDeviceObject(void* context, IOReturn result, void* sender, IOHIDReportType type, uint32_t reportID, uint8_t* report, CFIndex reportLength)
{
	//u32* pContext = (u32*)context;
	//TRef refDeviceID = (*pContext);
	TRef* pTRef = static_cast<TRef*>(context);
	TRef refDeviceID = *pTRef;

	
	TPtr<TInputDevice> pDevice = g_pInputSystem->GetInstance(refDeviceID);
	
	if(pDevice.IsValid())
	{
		TPtr<TLInputHIDDevice> pHIDDevice = g_TLHIDDevices.FindPtr(pDevice->GetHardwareDeviceID());
		
		if(pHIDDevice.IsValid())
		{			
			// We have the associated device
			// Now setup the hardware device object for the generic device
			
			/*
			TSensorType SensorType = Unknown;
			TRef	LabelRef;
			switch(DIDFT_GETTYPE(pdidObjectInstance->dwType))
			{
				case DIDFT_PSHBUTTON:
				case DIDFT_TGLBUTTON:
				case DIDFT_BUTTON:
					SensorType = Button;
					LabelRef = GetDefaultButtonRef(pDevice->GetSensorCount(SensorType));
					break;
					
				case DIDFT_RELAXIS:
				case DIDFT_ABSAXIS:
				case DIDFT_AXIS:
					SensorType = Axis;
					LabelRef = GetDefaultAxisRef(pDevice->GetSensorCount(SensorType));
					break;
					
				case DIDFT_POV:
					SensorType = POV;
					LabelRef = GetDefaultPOVRef(pDevice->GetSensorCount(SensorType));
					break;
			}
			
			if(SensorType != Unknown)
			{
				//u32 uInstanceID = DIDFT_GETINSTANCE(pdidObjectInstance->dwType);
				// Use the offset as the ID
				u32 uInstanceID = pdidObjectInstance->dwOfs;
				
				TPtr<TInputSensor> pSensor = pDevice->AttachSensor(uInstanceID, SensorType);
				
				if(pSensor.IsValid())
				{
					pSensor->AddLabel(LabelRef);
					pSensor->SubscribeTo(pDXDevice);
					
					
					if(SensorType == Button)
					{
						// Calcualte the index of the button - used for when accessing the 
						// button specific arrays as they can only be accessed via index
						// because the ID isn;t know in advance
						//u32	uIndex = pDevice->GetSensorCount(SensorType);
						
						// Add any additional labels
						if(GetSpecificButtonRef(uInstanceID, pDevice->GetDeviceType(), pDXDevice->GetProductID(), LabelRef))
						{
							pSensor->AddLabel(LabelRef);
						}
						else
						{
							//TLDebug::Break("Failed to find additional label for sensor");
						}
					}
				}
			}
			 */
		}
	}	
}



/*
void Platform::HID::CreateDevice_Keyboard(const IOHIDDeviceRef device)
{
	TLDebug_Print("Device Type - Keyboard");
	
	// For USB devices we need to use reports or transactions via the HID to be able to send and recieve data
	// Get the max erport size for this device
	if(!l_pKeyboardReportBuffer)
	{	
		u32 maxreportsize = 0;
		
		CFTypeRef tCFTypeRef = IOHIDDeviceGetProperty( device, CFSTR( kIOHIDMaxInputReportSizeKey ) );
		if ( tCFTypeRef ) 
		{
			// if this is a number
			if ( CFNumberGetTypeID( ) == CFGetTypeID( tCFTypeRef ) ) 
			{			
				// get its value
				if(CFNumberGetValue( ( CFNumberRef ) tCFTypeRef, kCFNumberSInt32Type, &maxreportsize ))
				{
					TTempString str;
					str.Appendf("Buffersize: ");
					str.Appendf("%d", maxreportsize);
					TLDebug_Print(str);
					str.Empty();
				}
			}
		}	
	
		// TEST
		l_pKeyboardReportBuffer = (uint8_t*)TLMemory::Platform::MemAlloc(maxreportsize);
		
		
		// Register a report callback for this device
		IOHIDDeviceRegisterInputReportCallback( device, l_pKeyboardReportBuffer, maxreportsize, DeviceReportCallback, 0 );	
		
		//IOHIDDeviceRegisterRemovalCallback( device, DeviceRemovedCallback, 0 );
		//IOHIDDeviceScheduleWithRunLoop( device, CFRunLoopGetCurrent( ), kCFRunLoopDefaultMode );
	}
}

void Platform::HID::CreateDevice_Mouse(const IOHIDDeviceRef device)
{
	TLDebug_Print("Device Type - Mouse");

	if(!l_pMouseReportBuffer)
	{
		u32 maxreportsize = 0;
		
		CFTypeRef tCFTypeRef = IOHIDDeviceGetProperty( device, CFSTR( kIOHIDMaxInputReportSizeKey ) );
		if ( tCFTypeRef ) 
		{
			// if this is a number
			if ( CFNumberGetTypeID( ) == CFGetTypeID( tCFTypeRef ) ) 
			{			
				// get its value
				if(CFNumberGetValue( ( CFNumberRef ) tCFTypeRef, kCFNumberSInt32Type, &maxreportsize ))
				{
					TTempString str;

					str.Appendf("Buffer size: ");
					str.Appendf("%d", maxreportsize);
					TLDebug_Print(str);
					str.Empty();
				}
			}
		}	
		
		// TEST
		l_pMouseReportBuffer = (uint8_t*)TLMemory::Platform::MemAlloc(maxreportsize);
		
		
		// Register a report callback for this device
		IOHIDDeviceRegisterInputReportCallback( device, l_pMouseReportBuffer, maxreportsize, DeviceReportCallback, 0 );	
		
		//IOHIDDeviceRegisterRemovalCallback( device, DeviceRemovedCallback, 0 );
		//IOHIDDeviceScheduleWithRunLoop( device, CFRunLoopGetCurrent( ), kCFRunLoopDefaultMode );
	}	
}

void Platform::HID::CreateDevice_Gamepad(const IOHIDDeviceRef device)
{
	TLDebug_Print("Device Type - Gamepad");
	
	TTempString str;
	s32 vendorID = 0;
	s32 productID = 0;
	
	// Get the vendor ID
	CFTypeRef tCFTypeRef = IOHIDDeviceGetProperty( device, CFSTR( kIOHIDVendorIDKey ) );
    if ( tCFTypeRef ) 
	{
        // if this is a number
        if ( CFNumberGetTypeID( ) == CFGetTypeID( tCFTypeRef ) ) 
		{
            // get its value
			if(CFNumberGetValue( ( CFNumberRef ) tCFTypeRef, kCFNumberSInt32Type, &vendorID ))
			{
				str.Appendf("Vendor ID: ");
				str.Appendf("%x", vendorID);
				TLDebug_Print(str);
				str.Empty();
			}
        }
    }
	
	// Get the product ID
	tCFTypeRef = IOHIDDeviceGetProperty( device, CFSTR( kIOHIDProductIDKey ) );
    if ( tCFTypeRef ) 
	{
        // if this is a number
        if ( CFNumberGetTypeID( ) == CFGetTypeID( tCFTypeRef ) ) 
		{			
            // get its value
			if(CFNumberGetValue( ( CFNumberRef ) tCFTypeRef, kCFNumberSInt32Type, &productID ))
			{
				str.Appendf("Product ID: ");
				str.Appendf("%x", productID);
				TLDebug_Print(str);
				str.Empty();
			}
        }
    }	
	
	switch(vendorID)
	{
		case Vendor_Nintendo:
		{
			if(productID == Gamepad_Wiimote)
			{
				TLDebug_Print("Wiimote found");
				
			}
		}
			break;
	}
	
	
	// For USB devices we need to use reports or transactions via the HID to be able to send and recieve data
	// Get the max erport size for this device	
	u32 maxreportsize = 0;
	
	tCFTypeRef = IOHIDDeviceGetProperty( device, CFSTR( kIOHIDMaxInputReportSizeKey ) );
    if ( tCFTypeRef ) 
	{
        // if this is a number
        if ( CFNumberGetTypeID( ) == CFGetTypeID( tCFTypeRef ) ) 
		{			
            // get its value
			if(CFNumberGetValue( ( CFNumberRef ) tCFTypeRef, kCFNumberSInt32Type, &maxreportsize ))
			{
				str.Appendf("Buffer size: ");
				str.Appendf("%d", maxreportsize);
				TLDebug_Print(str);
				str.Empty();
			}
        }
    }	
	
	// TEST
	l_pReportBuffer = (uint8_t*)TLMemory::Platform::MemAlloc(maxreportsize);
	
	
	// Register a report callback for this device
	IOHIDDeviceRegisterInputReportCallback( device, l_pReportBuffer, maxreportsize, DeviceReportCallback, 0 );	
	//IOHIDDeviceRegisterRemovalCallback( device, DeviceRemovedCallback, 0 );
	//IOHIDDeviceScheduleWithRunLoop( device, CFRunLoopGetCurrent( ), kCFRunLoopDefaultMode );
}

*/

void Platform::HID::DeviceRemovedCallback(void* context, IOReturn result,  void* sender, IOHIDDeviceRef device)
{
	TLDebug_Print("Device removed");
	
	//TODO:
	// Removed device from list of plaform specific devices
	// Notify engine of device removal
}


void Platform::HID::DeviceReportCallback(void * inContext, IOReturn inResult, void * inSender, IOHIDReportType inType, uint32_t inReportID, uint8_t * inReport, CFIndex inReportLength)
{
	TLDebug_Print("Device report");
	
	// TEST
	for( u32 uIndex = 0; uIndex < inReportLength; uIndex++ )
	{
		TTempString str;
		str.Appendf( "%02x", inReport[uIndex] );
		TLDebug_Print(str);
	}
	
	int2 cursorpos = GetCursorPosition(0);
	TTempString str;
	str.Appendf( "Cursor: (%d, %d)", cursorpos.x, cursorpos.y );
	TLDebug_Print(str);
}



SyncBool Platform::CreateVirtualDevice(TRefRef InstanceRef, TRefRef DeviceTypeRef)
{	
	
	return SyncFalse;
}

SyncBool Platform::RemoveVirtualDevice(TRefRef InstanceRef)
{
	return SyncFalse;
}




// Create Ipod input device
Bool Platform::HID::CreateDevice(TRefRef InstanceRef, TRefRef DeviceTypeRef, Bool bVirtual)
{
	// Create the generic input object
	TPtr<TInputDevice>& pGenericDevice = g_pInputSystem->GetInstance(InstanceRef, TRUE, DeviceTypeRef);
	
	if(pGenericDevice.IsValid())
	{
		if(!InitialiseDevice(pGenericDevice, DeviceTypeRef, bVirtual))
		{
			// Failed to initialise the input device data						
			pGenericDevice = NULL;
			
			g_pInputSystem->RemoveInstance(InstanceRef);
			
			return FALSE;
		}
		else
		{
			// Notify to all subscribers of the input system that a new device was added
			TLMessaging::TMessage Message("DeviceChanged");			
			
			TRef refState = "ADDED";
			Message.ExportData("State", refState);					// state change
			Message.ExportData("DEVID", pGenericDevice->GetDeviceRef() );	// device ID
			Message.ExportData("TYPE", pGenericDevice->GetDeviceType() );						// device type
			
			g_pInputSystem->PublishMessage(Message);
				
			// Success
			return TRUE;
		}
	}
	
	return FALSE;
}

// Initialise the device
Bool Platform::HID::InitialiseDevice(TPtr<TInputDevice> pDevice, TRefRef DeviceTypeRef, Bool bVirtual)
{
	// Intitialise a virtual device?
	if(bVirtual)
		return InitialiseVirtualDevice(pDevice, DeviceTypeRef);
	
	// Initialise the physical device
	return InitialisePhysicalDevice(pDevice, DeviceTypeRef);
}


Bool Platform::HID::InitialiseVirtualDevice(TPtr<TInputDevice> pDevice, TRefRef DeviceTypeRef)
{	
	return FALSE;
}

Bool Platform::HID::InitialisePhysicalDevice(TPtr<TInputDevice> pDevice, TRefRef DeviceTypeRef)
{
	// Create four 'buttons' and 'axis' sensors to be able to send data from
	// and for actions to be mapped to
	
	// Add button inputs
	u32 uIndex = 0;
	
	u32 uUniqueID = 0;
	
	TString stringLabel;
	TRef refLabel;
	
	
	for(uIndex = 0; uIndex < HID::MAX_CURSOR_POSITIONS; uIndex++)
	{
		// For buttons we need to label them based on what type and the model
		// so get this information from a function in stead which will lookup the details required
		
		TPtr<TInputSensor>& pSensor = pDevice->AttachSensor(uUniqueID, Button);
		
		if(pSensor.IsValid())
		{
		
			refLabel = GetDefaultButtonRef(uIndex);
			pSensor->AddLabel(refLabel);
			pSensor->SetCursorIndex(uIndex);
			uUniqueID++;
		}
	}

	u32 uAxisIndex = 0;
	for(uIndex = 0; uIndex < HID::MAX_CURSOR_POSITIONS; uIndex++)
	{		
		uAxisIndex = uIndex * 3;
		
		// Add X axis sensor
		TPtr<TInputSensor> pSensor = pDevice->AttachSensor(TRef(uUniqueID), Axis);
		
		if(pSensor.IsValid())
		{
			refLabel = GetDefaultAxisRef(uAxisIndex);
			pSensor->AddLabel(refLabel);
			pSensor->SetCursorIndex(uIndex);
			uUniqueID++;			
		}
		
		// Add Y axis sensor
		pSensor = pDevice->AttachSensor(uUniqueID, Axis);
		
		if(pSensor.IsValid())
		{
			refLabel = GetDefaultAxisRef(uAxisIndex+1);
			pSensor->AddLabel(refLabel);
			pSensor->SetCursorIndex(uIndex);
			uUniqueID++;
		}
	}
	
	TArray<TRef> AxisRefs;
	AxisRefs.Add("ACCX");
	AxisRefs.Add("ACCY");
	AxisRefs.Add("ACCZ");
	
	for(uIndex = 0; uIndex < 3; uIndex++)
	{
		// Add accelerometer axis
		TPtr<TInputSensor>& pSensor = pDevice->AttachSensor(uUniqueID, Axis);
		
		if(pSensor.IsValid())
		{
			TRef refLabel = AxisRefs.ElementAt(uIndex);
			pSensor->AddLabel(refLabel);
			uUniqueID++;
		}
	}
	
	return TRUE;
}


SyncBool Platform::HID::Shutdown()	
{	
	RemoveAllDevices();
	
	// Shutdown the mac hardware
	if(g_IOHIDManagerRef)
	{
		// Unschedule with run loop
		IOHIDManagerUnscheduleFromRunLoop( g_IOHIDManagerRef, CFRunLoopGetCurrent( ), kCFRunLoopDefaultMode );
		
		// Remove callbacks
		IOHIDManagerRegisterDeviceMatchingCallback(g_IOHIDManagerRef, NULL, 0);
		IOHIDManagerRegisterDeviceRemovalCallback(g_IOHIDManagerRef, NULL, 0);

		// Close the HID manager
		IOReturn result = IOHIDManagerClose(g_IOHIDManagerRef, 0L);
		
		if ( kIOReturnSuccess != result ) 
		{
			TLDebug_Break("Failed to close HID manager");
		} 
		
	}
	
	g_KeyboardRefMap.Empty(TRUE);
	g_MouseRefMap.Empty(TRUE);
	
	g_Xbox360PadButtonRefs.Empty(TRUE);		
	g_WiimoteButtonRefs.Empty(TRUE);		
	g_PS3PadButtonRefs.Empty(TRUE);			
	
	g_HIDDeviceProductIDRefs.Empty(TRUE);
	
	
	return SyncTrue;
}

	

void Platform::HID::RemoveAllDevices() 
{
	g_TLHIDDevices.SetAll(NULL);
	g_TLHIDDevices.Empty(TRUE);
}



Bool Platform::UpdateDevice(TLInput::TInputDevice& Device)
{
	//TODO: Add proper device type checking for virtual and physical devices
	if(Device.GetDeviceType() == /*TLInput::TrackpadRef*/ TLInput::MouseRef)
	{
		// Physical device
		return HID::UpdatePhysicalDevice(Device);
	}
	else
	{
		// Virtual device
		return HID::UpdateVirtualDevice(Device);
	}	
	
	return FALSE;
}


Bool Platform::HID::UpdatePhysicalDevice(TLInput::TInputDevice& Device)
{
	// Get the physical device ID
	TRef DeviceID = Device.GetHardwareDeviceID();
	
	// Find the physical device from the platform based list
	TPtr<TLInputHIDDevice>& pTLHIDDevice = g_TLHIDDevices.FindPtr(DeviceID);
	
	if ( !pTLHIDDevice )
		return FALSE;
	
	Bool bResult = FALSE;
	
	switch(pTLHIDDevice->GetDeviceType())
	{
		case kHIDUsage_GD_Mouse:
			bResult = UpdateHIDDevice_Mouse( Device, *pTLHIDDevice );
			break;
		case kHIDUsage_GD_Keyboard:
			bResult = UpdateHIDDevice_Keyboard( Device, *pTLHIDDevice );
			break;
		case kHIDUsage_GD_GamePad:
			bResult = UpdateHIDDevice_Gamepad( Device, *pTLHIDDevice );
			break;
	}
	
	return bResult;
}


Bool Platform::HID::UpdateHIDDevice_Mouse(TInputDevice& Device,TLInputHIDDevice& TLHIDDevice)
{
	IOHIDDeviceRef HIDDeviceRef = TLHIDDevice.GetHIDDevice();
	
	// If valid poll and acquire the device ensuring no errors.
	if(HIDDeviceRef == 0)
		return FALSE;
	
/*	
	lpdiDevice->Poll();
	
	// Now read some data
	DIDEVICEOBJECTDATA		rgdod[INPUT_MOUSE_BUFFER_SIZE];
	DWORD					dwItems = INPUT_MOUSE_BUFFER_SIZE;
	
	HRESULT hr = lpdiDevice->GetDeviceData(sizeof(DIDEVICEOBJECTDATA), rgdod, &dwItems, 0);
	
	if(FAILED(hr) || (dwItems == 0)) 
	{ 
		return FALSE;
	}
	
	// dwItems = Number of elements read (could be zero).
	if (hr == DI_BUFFEROVERFLOW) 
	{	
		// Buffer had overflowed. 
		TLDebug_Print("Mouse Input Buffer overflow");
	} 
	
	/////////////////////////////////////////////////////////////
	// Copy data from the physical device to the generic input device for the sensors to use
	/////////////////////////////////////////////////////////////
	
#ifdef _DEBUG
	TTempString inputinfo = "Mouse processing buffer: ";
	inputinfo.Appendf("%d items", dwItems);
	TLDebug::Print(inputinfo);
#endif
	
	//	make a list of axes to consolidate after processing
	TFixedArray<TRef,100> ConsolidateAxes;
	
	//	pre-alloc input buffer space
	TArray<TInputData>& InputBuffer = Device.GetInputBuffer();
	InputBuffer.AddAllocSize( dwItems );
	
	TInputData data;
	
	for(u32 uCount = 0; uCount < dwItems; uCount++)
	{
		// get the data from the buffer
		float fValue = 0.0f;
		Bool IsAxisData = (rgdod[uCount].dwOfs == DIMOFS_X) || (rgdod[uCount].dwOfs == DIMOFS_Y) || (rgdod[uCount].dwOfs == DIMOFS_Z);
		
		// Check for the axes and scale down the value
		if(	IsAxisData )
		{
			
			s32 sValue = rgdod[uCount].dwData;
			
			// Get the granularity
			DIPROPDWORD dipdw;
			dipdw.diph.dwSize			= sizeof(dipdw);
			dipdw.diph.dwHeaderSize		= sizeof(dipdw.diph);
			dipdw.diph.dwObj			= rgdod[uCount].dwOfs;
			dipdw.diph.dwHow			= DIPH_BYOFFSET;
			dipdw.dwData				= 0;
			
			hr = lpdiDevice->GetProperty(DIPROP_GRANULARITY, &dipdw.diph);
			
			
			Bool bSuccess = FALSE;
			
			// Only return if failed
			if(SUCCEEDED(hr))
			{
				// Divide by the granularity - most often this is 1 but for the Z axis 
				// it's usually something else like 20
				fValue = ((float)sValue / (float)dipdw.dwData);
				
				//Simple scale of the value instead of doing a percentage
				//	gr: needs to be changed so it's not some arbitry value... just keep it in screen space...
				fValue /= 100.0f;
				
				bSuccess = TRUE;
			}
			
			// If we couldn't get the value as a percentage just return raw value
			if(!bSuccess)
			{
				fValue = (float)sValue;
			}
		}
		else
		{
			//	is a button, not an axis
			fValue = (rgdod[uCount].dwData > 0.0f ? 1.0f : 0.0f);
		}
		
		data.m_SensorRef = rgdod[uCount].dwOfs;
		data.m_fData = fValue;
		InputBuffer.Add(data);
		
		if ( IsAxisData )
		{
			ConsolidateAxes.AddUnique( data.m_SensorRef );
		}
#ifdef _DEBUG
		// In debug print what button was pressed
		TString inputinfo = "Mouse input: ";
		inputinfo.Appendf("%d %.4f", rgdod[uCount].dwOfs, fValue);
		TLDebug::Print(inputinfo);
		
		if(fValue > 10000.0f)
		{
			TLDebug::Break("HUGE number");
		}
#endif
		
	}
	
	//	consolidate axis data
	for ( u32 i=0;	i<ConsolidateAxes.GetSize();	i++ )
		ConslidateAxisMovement( InputBuffer, ConsolidateAxes[i] );
	
	return (hr == DI_OK);
 */
	
	return TRUE;
}


//-------------------------------------------------------------------
//	for an axis, consolidate all the movement in an axis to one movement
//-------------------------------------------------------------------
Bool Platform::HID::ConslidateAxisMovement(TArray<TInputData>& InputBuffer,TRefRef AxisSensorRef)
{
	//	reduce all movement (values) on one axis down to one value
	//	gr: the caveat here is that we could have a sequence of
	//	move down move up
	//	and we'll lose the 2nd move (between down and up, ie, a drag)
	//	my thinking is that if this is in the input buffer then we've managed to
	//	do this in ONE frame, and the user couldn't have done that much important stuff in 1/60th of a second
	float RunningValue = 0.f;
	s32 PreviousValueIndex = -1;
	u32 Debug_DatasRemoved = 0;
	
	for ( s32 i=InputBuffer.GetLastIndex();	i>=0;	i-- )
	{
		TInputData& InputData = InputBuffer[i];
		if ( InputData.m_SensorRef != AxisSensorRef )
			continue;
		
		//	accumulate this movement 
		RunningValue += InputData.m_fData;
		InputData.m_fData = RunningValue;
		
		//	if there was a previous value remove the one we've now accumulated
		if ( PreviousValueIndex != -1 )
		{
			InputBuffer.RemoveAt( PreviousValueIndex );
			Debug_DatasRemoved++;
		}
		
		//	THIS is now the next one to remove if we find an earlier value
		PreviousValueIndex = i;
	}
	
#ifdef _DEBUG
	if ( Debug_DatasRemoved > 0 )
	{
		TTempString Debug_String("Removed ");
		Debug_String.Appendf("%d data's from the axis sensor ",Debug_DatasRemoved);
		AxisSensorRef.GetString( Debug_String );
		TLDebug_Print( Debug_String );
	}
#endif
	
	return (Debug_DatasRemoved>0);
}


//-------------------------------------------------------------------
//	Special update routine for a keyboard using directx
//-------------------------------------------------------------------
Bool Platform::HID::UpdateHIDDevice_Keyboard(TInputDevice& Device,TLInputHIDDevice& TLHIDDevice)
{
	IOHIDDeviceRef HIDDeviceRef = TLHIDDevice.GetHIDDevice();
	
	// If valid poll and acquire the device ensuring no errors.
	if(HIDDeviceRef == 0)
		return FALSE;
	
	/*
	// Acquire the device
	HRESULT hr = lpdiDevice->Acquire(); 
	
	if(FAILED( hr ))
		return FALSE;
	
	lpdiDevice->Poll();
	
	
	// Now read some data
	DIDEVICEOBJECTDATA		rgdod[INPUT_KEYBOARD_BUFFER_SIZE];
	DWORD					dwItems = INPUT_KEYBOARD_BUFFER_SIZE;
	
	hr = lpdiDevice->GetDeviceData(sizeof(DIDEVICEOBJECTDATA), rgdod, &dwItems, 0);
	
	
	if( FAILED( hr ) || (dwItems == 0) )
	{
		// DirectInput may be telling us that the input stream has been
		// interrupted.  We aren't tracking any state between polls, so
		// we don't have any special reset that needs to be done.
		// We just re-acquire and try again.
		return FALSE;
	}
	
	// dwItems = Number of elements read (could be zero).
	if (hr == DI_BUFFEROVERFLOW) 
	{	
		// Buffer had overflowed. 
		TLDebug_Print("Keyboard Input Buffer overflow");
	} 
	
	
#ifdef _DEBUG
	TTempString inputinfo = "Keyboard processing buffer: ";
	inputinfo.Appendf("%d items", dwItems);
	TLDebug::Print(inputinfo);
#endif
	
	TArray<TInputData>& InputBuffer = Device.GetInputBuffer();
	InputBuffer.AddAllocSize( dwItems );
	
	TInputData data;
	
	// Publish the keyboard data
	for(u32 uIndex = 0; uIndex < dwItems; uIndex++)
	{
		data.m_SensorRef = rgdod[uIndex].dwOfs;
		
		// For every keyboard button publish it's value - if any sensors subscribe to the 
		// keyboard buttons they will process the information
		float fValue = (rgdod[uIndex].dwData & 0x80) ? 1.0f : 0.0f;
		data.m_fData = fValue;
		
		// Add to the buffer
		InputBuffer.Add( data );
		
#ifdef _DEBUG
		// In debug print what key was pressed
		TString inputinfo = "Keyboard input: ";
		inputinfo.Appendf("%d %.2f", rgdod[uIndex].dwOfs, fValue);
		TLDebug::Print(inputinfo);
#endif
	}
	*/
	
	return TRUE;
}

/*	
 Special update routine for a joypad
 */
Bool Platform::HID::UpdateHIDDevice_Gamepad(TInputDevice& Device,TLInputHIDDevice& TLHIDDevice)
{
	IOHIDDeviceRef HIDDeviceRef = TLHIDDevice.GetHIDDevice();
	
	// If valid poll and acquire the device ensuring no errors.
	if(HIDDeviceRef == 0)
		return FALSE;
	
	/*
	// Poll and acquire the device
	HRESULT hr = lpdiDevice->Acquire(); 
	
	if(FAILED(hr))
		return FALSE;
	
	// NOTE: May need to test for DIERR_INPUTLOST
	hr = lpdiDevice->Poll(); 
	
	if(FAILED(hr))
		return FALSE;
	
	
	// Now read some data
	DIDEVICEOBJECTDATA		rgdod[INPUT_GAMEPAD_BUFFER_SIZE];
	DWORD					dwItems = INPUT_GAMEPAD_BUFFER_SIZE;
	
	hr = lpdiDevice->GetDeviceData(sizeof(DIDEVICEOBJECTDATA), rgdod, &dwItems, 0);
	
	
	if( FAILED( hr ) || (dwItems == 0) )
	{
		return FALSE;
	}
	
	// dwItems = Number of elements read (could be zero).
	if (hr == DI_BUFFEROVERFLOW) 
	{	
		// Buffer had overflowed. 
		TLDebug_Print("Gamepad Input Buffer overflow");
	} 
	
	
#ifdef _DEBUG
	TString inputinfo = "Gamepad processing buffer: ";
	inputinfo.Appendf("%d items", dwItems);
	TLDebug::Print(inputinfo);
#endif
	
	TArray<TInputData>& InputBuffer = Device.GetInputBuffer();
	InputBuffer.AddAllocSize( dwItems );
	
	TInputData data;
	
	for(u32 uIndex = 0; uIndex < dwItems; uIndex++)
	{
		data.m_SensorRef = rgdod[uIndex].dwOfs;
		
		float fValue = ((rgdod[uIndex].dwData == 0) ? 0.0f : 1.0f);
		data.m_fData = fValue;
		
		// Add to the buffer
		InputBuffer.Add( data );
#ifdef _DEBUG
		TString inputinfo = "Gamepad input: ";
		inputinfo.Appendf("%d %.2f", rgdod[uIndex].dwOfs, fValue);
		TLDebug::Print(inputinfo);
#endif
	}
	
	return (hr == DI_OK);
	 */
	
	return TRUE;
}


int2 Platform::GetCursorPosition(u8 uIndex)
{
	// TODO: Return cursor pos
	return int2(0,0);
}


// On Ipod set the cursor position
void Platform::HID::SetCursorPosition(u8 uIndex, int2 uPos)
{
	//TODO: Set cursor pos
}


TLInput::Platform::HID::TLInputHIDDevice::~TLInputHIDDevice()
{
	// Release the buffer
	if(m_pReportBuffer)
		TLMemory::Platform::MemDealloc(m_pReportBuffer);

	// Remove from callbacks and runloop schedule
	IOHIDDeviceRegisterInputReportCallback( GetHIDDevice(), NULL, 0, NULL, 0 );		

}

void TLInput::Platform::HID::TLInputHIDDevice::InitialiseReportBuffer()
{
	// Initialise the data buffer for the HID reports		
	 u32 maxreportsize = 0;
	 
	 CFTypeRef tCFTypeRef = IOHIDDeviceGetProperty( GetHIDDevice(), CFSTR( kIOHIDMaxInputReportSizeKey ) );
	 if ( tCFTypeRef ) 
	 {
		 // if this is a number
		 if ( CFNumberGetTypeID( ) == CFGetTypeID( tCFTypeRef ) ) 
		 {			
			 // get its value
			 if(CFNumberGetValue( ( CFNumberRef ) tCFTypeRef, kCFNumberSInt32Type, &maxreportsize ))
			 {
				 TTempString str;
				 str.Appendf("Buffersize: ");
				 str.Appendf("%d", maxreportsize);
				 TLDebug_Print(str);
				 str.Empty();
			 }
		 }
	 }	
	
	// Allocate the memory for the report buffer
	m_pReportBuffer = static_cast<uint8_t*>(TLMemory::Platform::MemAlloc(maxreportsize)); 
	 
	// Setup the callback
	IOHIDDeviceRegisterInputReportCallback( GetHIDDevice(), 
										   GetReportBuffer(), 
										   maxreportsize, 
										   HID::DeviceReportCallback, 
										   &m_DeviceRef );		
}

void TLInput::Platform::HID::TLInputHIDDevice::PublishData(u32 uUniqueID, float fValue)
{
	TLMessaging::TMessage Message(uUniqueID);
	Message.Write(fValue);									// Value from sensor
	PublishMessage(Message);
}

Bool TLInput::Platform::HID::TLInputHIDDevice::EnumerateObjects()
{
	/*
	 kIOHIDMaxFeatureReportSizeKey
	CFIndex ReportIndex = 0;
	IOReturn result = IOHIDDeviceGetReportWithCallback(GetHIDDevice(), 
													   kIOHIDReportTypeFeature, 
													   ReportIndex, 
													   GetReportBuffer(), 
													   NULL,
													   0L, 
													   HID::EnumDeviceObject, 
													   &m_DeviceRef);
	
	if(result != kIOReturnSuccess)
		return FALSE;
	*/
	return TRUE;
}





