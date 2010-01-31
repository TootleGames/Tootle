//-------------------------------------------------------
//	Win32Input.cpp
//-------------------------------------------------------


// Include files
//-------------------------------------------------------
#include "PCInput.h"
#include <TootleCore/TSyncQueue.h>
#include <TootleCore/TPtr.h>
#include <TootleCore/TEventChannel.h> 
#include <TootleCore/TLCore.h>


// DirectX buffer sizes for each device type
#define INPUT_KEYBOARD_BUFFER_SIZE	16
#ifdef _DEBUG
	#define INPUT_MOUSE_BUFFER_SIZE		64
#else
	#define INPUT_MOUSE_BUFFER_SIZE		32
#endif
#define INPUT_GAMEPAD_BUFFER_SIZE	16

// Globals
namespace TLInput
{
	namespace Platform
	{
		namespace DirectX
		{
			LPDIRECTINPUT8							g_pTLDirectInputInterface = NULL;	// Global direct input interface pointer
			TPtrArray<TLInputDirectXDevice>			g_TLDirectInputDevices;					// Global array of direct device pointers

			TKeyArray<u32, TRef>					g_KeyboardRefMap;
			TKeyArray<u32, TRef>					g_MouseRefMap;

			// For now we have specifically created keyarrays for the extra button references on gamepads
			// Eventually this should be an array of arrays where we can 'plugin' the pads supported
			// via an XML data file rather than using specific initialise routines.
			// This could also apply to the keyboard and mouse ref maps as they will owrk in a similar fashion
			TKeyArray<u32, TRef>					g_Xbox360PadButtonRefs;		// XBox 360 pad button refs
			TKeyArray<u32, TRef>					g_WiimoteButtonRefs;		// Wiimote button refs
			TKeyArray<u32, TRef>					g_PS2PadButtonRefs;			// PS2 pad button refs

			TKeyArray<u32, TRef>					g_DirectXDeviceProductIDRefs;	//	table mapping DX device ProductID to TRefs - the Guids are not always valid TRef's (we use 30 bits exclusivly) - so this keeps an internal Ref<->Guid map

			void	InitialiseKeyboadRefMap();
			void	InitialiseMouseRefMap();
			void	InitialiseGamePadRefMaps();

			void	InitialiseXBox360PadRefMap();
			void	InitialiseWiimoteRefMap();
			void	InitialisePS2PadRefMap();

			Bool	GetSpecificButtonRef(const u32& uButtonID, TRefRef DeviceType, const u32& uProductID, TRef& LabelRef);
		
			u32		GetDeviceProductIDFromRef(TRefRef DeviceRef);				//	get the ProductID thats mapped to this ref. ZERO returned is assumed invalid
			TRef	GetDeviceRefFromProductID(u32 ProductID,Bool CreateNew);	//	get the ref thats mapped to this device ProductID. Optionally create a new entry
		};
	};

	const float AXIS_SCALE				= 1000.0f;
};

using namespace TLInput;



Bool Platform::UpdateDevice(TInputDevice& Device)
{
	return DirectX::UpdateDevice( Device );
}

int2 Platform::GetCursorPosition(u8 uIndex)
{
	// On PC there is only one cursor for all users really and that is the mouse position.
	// So simply get the mouse pos in screen space and return it
	POINT MouseScreenPos;
	GetCursorPos( &MouseScreenPos );
	ScreenToClient( TLCore::Platform::g_HWnd, &MouseScreenPos );
	int2 MousePos( MouseScreenPos.x, MouseScreenPos.y );

	return MousePos;
}



//---------------------------------------------------
//Initialises the direct input system
// NOTE: Does not create any input devices at this stage
//---------------------------------------------------
SyncBool Platform::DirectX::Init()
{
	InitialiseKeyboadRefMap();
	InitialiseMouseRefMap();
	InitialiseGamePadRefMaps();

	if(g_pTLDirectInputInterface)
		return SyncTrue;

	HINSTANCE hInstance = TLCore::Platform::g_HInstance;

	// Create the directx device interface
	HRESULT res = DirectInput8Create( hInstance, DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&g_pTLDirectInputInterface, NULL );

	// Did the interface get created?
	if(DI_OK != res)
		return SyncFalse;

	if ( !g_pTLDirectInputInterface )
	{
		TLDebug_Break("DirectInputInterface expected; shouldn't be NULL after creation");
		return SyncFalse;
	}

	/*
	SyncBool EnumDevs = Platform::DirectX::EnumerateDevices();

	if(EnumDevs == SyncFalse)
	{
		// Release the direct input interface and return failed
		Shutdown();
		return SyncFalse;
	}
	*/

	//	all succeeded
	return SyncTrue;
}


void Platform::DirectX::InitialiseKeyboadRefMap()
{
	g_KeyboardRefMap.Add(DIK_ESCAPE, "K_ESC");
	g_KeyboardRefMap.Add(DIK_1, "K_1");
	g_KeyboardRefMap.Add(DIK_2, "K_2");
	g_KeyboardRefMap.Add(DIK_3, "K_3");
	g_KeyboardRefMap.Add(DIK_4, "K_4");
	g_KeyboardRefMap.Add(DIK_5, "K_5");
	g_KeyboardRefMap.Add(DIK_6, "K_6");
	g_KeyboardRefMap.Add(DIK_7, "K_7");
	g_KeyboardRefMap.Add(DIK_8, "K_8");
	g_KeyboardRefMap.Add(DIK_9, "K_9");
	g_KeyboardRefMap.Add(DIK_0, "K_0");
	g_KeyboardRefMap.Add(DIK_MINUS, "K_MINUS");
	g_KeyboardRefMap.Add(DIK_EQUALS, "K_EQUALS");
	g_KeyboardRefMap.Add(DIK_BACK, "K_BACKSPACE");
	g_KeyboardRefMap.Add(DIK_TAB, "K_TAB");
	g_KeyboardRefMap.Add(DIK_Q, "K_Q");
	g_KeyboardRefMap.Add(DIK_W, "K_W");
	g_KeyboardRefMap.Add(DIK_E, "K_E");
	g_KeyboardRefMap.Add(DIK_R, "K_R");
	g_KeyboardRefMap.Add(DIK_T, "K_T");
	g_KeyboardRefMap.Add(DIK_Y, "K_Y");
	g_KeyboardRefMap.Add(DIK_U, "K_U");
	g_KeyboardRefMap.Add(DIK_I, "K_I");
	g_KeyboardRefMap.Add(DIK_O, "K_O");
	g_KeyboardRefMap.Add(DIK_P, "K_P");
	g_KeyboardRefMap.Add(DIK_LBRACKET, "K_LBRACKET");
	g_KeyboardRefMap.Add(DIK_RBRACKET, "K_RBRACKET");
	g_KeyboardRefMap.Add(DIK_RETURN, "K_RETURN");
	g_KeyboardRefMap.Add(DIK_LCONTROL, "K_LCTRL");
	g_KeyboardRefMap.Add(DIK_A, "K_A");
	g_KeyboardRefMap.Add(DIK_S, "K_S");
	g_KeyboardRefMap.Add(DIK_D, "K_D");
	g_KeyboardRefMap.Add(DIK_F, "K_F");
	g_KeyboardRefMap.Add(DIK_G, "K_G");
	g_KeyboardRefMap.Add(DIK_H, "K_H");
	g_KeyboardRefMap.Add(DIK_J, "K_J");
	g_KeyboardRefMap.Add(DIK_K, "K_K");
	g_KeyboardRefMap.Add(DIK_L, "K_L");
	g_KeyboardRefMap.Add(DIK_SEMICOLON, "K_COLON");
	g_KeyboardRefMap.Add(DIK_APOSTROPHE, "K_APOSTROPHE");
	g_KeyboardRefMap.Add(DIK_GRAVE, "K_GRAVE");
	g_KeyboardRefMap.Add(DIK_LSHIFT, "K_LSHIFT");
	g_KeyboardRefMap.Add(DIK_BACKSLASH, "K_BACKSLASH");
	g_KeyboardRefMap.Add(DIK_Z, "K_Z");
	g_KeyboardRefMap.Add(DIK_X, "K_X");
	g_KeyboardRefMap.Add(DIK_C, "K_C");
	g_KeyboardRefMap.Add(DIK_V, "K_V");
	g_KeyboardRefMap.Add(DIK_B, "K_B");
	g_KeyboardRefMap.Add(DIK_N, "K_N");
	g_KeyboardRefMap.Add(DIK_M, "K_M");
	g_KeyboardRefMap.Add(DIK_COMMA, "K_COMMA");
	g_KeyboardRefMap.Add(DIK_PERIOD, "K_PERIOD");
	g_KeyboardRefMap.Add(DIK_SLASH, "K_FORWARDSLASH");
	g_KeyboardRefMap.Add(DIK_RSHIFT, "K_RSHIFT");
	g_KeyboardRefMap.Add(DIK_MULTIPLY, "K_MULTIPLY");
	g_KeyboardRefMap.Add(DIK_LMENU, "K_MENU");
	g_KeyboardRefMap.Add(DIK_SPACE, "K_SPACE");
	g_KeyboardRefMap.Add(DIK_CAPITAL, "K_CAPS");
	g_KeyboardRefMap.Add(DIK_F1, "K_F1");
	g_KeyboardRefMap.Add(DIK_F2, "K_F2");
	g_KeyboardRefMap.Add(DIK_F3, "K_F3");
	g_KeyboardRefMap.Add(DIK_F4, "K_F4");
	g_KeyboardRefMap.Add(DIK_F5, "K_F5");
	g_KeyboardRefMap.Add(DIK_F6, "K_F6");
	g_KeyboardRefMap.Add(DIK_F7, "K_F7");
	g_KeyboardRefMap.Add(DIK_F8, "K_F8");
	g_KeyboardRefMap.Add(DIK_F9, "K_F9");
	g_KeyboardRefMap.Add(DIK_F10, "K_F10");
	g_KeyboardRefMap.Add(DIK_NUMLOCK, "K_NUMLOCK");
	g_KeyboardRefMap.Add(DIK_SCROLL, "K_SCROLL");
	g_KeyboardRefMap.Add(DIK_NUMPAD7, "K_NP7");
	g_KeyboardRefMap.Add(DIK_NUMPAD8, "K_NP8");
	g_KeyboardRefMap.Add(DIK_NUMPAD9, "K_NP9");
	g_KeyboardRefMap.Add(DIK_SUBTRACT, "K_SUBTRACT");
	g_KeyboardRefMap.Add(DIK_NUMPAD4, "K_NP4");
	g_KeyboardRefMap.Add(DIK_NUMPAD5, "K_NP5");
	g_KeyboardRefMap.Add(DIK_NUMPAD6, "K_NP6");
	g_KeyboardRefMap.Add(DIK_ADD, "K_ADD");
	g_KeyboardRefMap.Add(DIK_NUMPAD1, "K_NP1");
	g_KeyboardRefMap.Add(DIK_NUMPAD2, "K_NP2");
	g_KeyboardRefMap.Add(DIK_NUMPAD3, "K_NP3");
	g_KeyboardRefMap.Add(DIK_NUMPAD0, "K_NP0");
	g_KeyboardRefMap.Add(DIK_DECIMAL, "K_DECIMAL");
	g_KeyboardRefMap.Add(DIK_OEM_102, "K_OEM");
	g_KeyboardRefMap.Add(DIK_F11, "K_F11");
	g_KeyboardRefMap.Add(DIK_F12, "K_F12");
	g_KeyboardRefMap.Add(DIK_F13, "K_F13");
	g_KeyboardRefMap.Add(DIK_F14, "K_F14");
	g_KeyboardRefMap.Add(DIK_F15, "K_F15");
	g_KeyboardRefMap.Add(DIK_KANA, "");
	g_KeyboardRefMap.Add(DIK_ABNT_C1, "");
	g_KeyboardRefMap.Add(DIK_CONVERT, "");
	g_KeyboardRefMap.Add(DIK_NOCONVERT, "");
	g_KeyboardRefMap.Add(DIK_YEN, "");
	g_KeyboardRefMap.Add(DIK_ABNT_C2, "");
	g_KeyboardRefMap.Add(DIK_NUMPADEQUALS, "");
	g_KeyboardRefMap.Add(DIK_PREVTRACK, "");
	g_KeyboardRefMap.Add(DIK_AT, "");
	g_KeyboardRefMap.Add(DIK_COLON, "");
	g_KeyboardRefMap.Add(DIK_UNDERLINE, "");
	g_KeyboardRefMap.Add(DIK_KANJI, "");
	g_KeyboardRefMap.Add(DIK_STOP, "");   
	g_KeyboardRefMap.Add(DIK_AX, "");
	g_KeyboardRefMap.Add(DIK_UNLABELED, "");
	g_KeyboardRefMap.Add(DIK_NEXTTRACK, "");
	g_KeyboardRefMap.Add(DIK_NUMPADENTER, "");
	g_KeyboardRefMap.Add(DIK_RCONTROL, "K_RCTRL");
	g_KeyboardRefMap.Add(DIK_MUTE, "K_MUTE");
	g_KeyboardRefMap.Add(DIK_CALCULATOR, "");
	g_KeyboardRefMap.Add(DIK_PLAYPAUSE, "");
	g_KeyboardRefMap.Add(DIK_MEDIASTOP, "");
	g_KeyboardRefMap.Add(DIK_VOLUMEDOWN, "K_VDOWN");
	g_KeyboardRefMap.Add(DIK_VOLUMEUP, "K_VUP");
	g_KeyboardRefMap.Add(DIK_WEBHOME, "");
	g_KeyboardRefMap.Add(DIK_NUMPADCOMMA, "");
	g_KeyboardRefMap.Add(DIK_DIVIDE, "");
	g_KeyboardRefMap.Add(DIK_SYSRQ, "");
	g_KeyboardRefMap.Add(DIK_RMENU, "");
	g_KeyboardRefMap.Add(DIK_PAUSE, "K_PAUSE");
	g_KeyboardRefMap.Add(DIK_HOME, "K_HOME");
	g_KeyboardRefMap.Add(DIK_UP, "K_UP");
	g_KeyboardRefMap.Add(DIK_PRIOR, "");
	g_KeyboardRefMap.Add(DIK_LEFT, "K_LEFT");    
	g_KeyboardRefMap.Add(DIK_RIGHT, "K_RIGHT");
	g_KeyboardRefMap.Add(DIK_END, "K_END");
	g_KeyboardRefMap.Add(DIK_DOWN, "K_DOWN");
	g_KeyboardRefMap.Add(DIK_NEXT, "K_NEXT");
	g_KeyboardRefMap.Add(DIK_INSERT, "K_INSERT");
	g_KeyboardRefMap.Add(DIK_DELETE, "K_DELETE");
	g_KeyboardRefMap.Add(DIK_LWIN, "");
	g_KeyboardRefMap.Add(DIK_RWIN, "");
	g_KeyboardRefMap.Add(DIK_APPS, "");
	g_KeyboardRefMap.Add(DIK_POWER, "K_POWER");
	g_KeyboardRefMap.Add(DIK_SLEEP, "K_SLEEP");
	g_KeyboardRefMap.Add(DIK_WAKE, "K_WAKE");
	g_KeyboardRefMap.Add(DIK_WEBSEARCH, "");
	g_KeyboardRefMap.Add(DIK_WEBFAVORITES, "");
	g_KeyboardRefMap.Add(DIK_WEBREFRESH, "");
	g_KeyboardRefMap.Add(DIK_WEBSTOP, "");
	g_KeyboardRefMap.Add(DIK_WEBFORWARD, "");
	g_KeyboardRefMap.Add(DIK_WEBBACK, "");
	g_KeyboardRefMap.Add(DIK_MYCOMPUTER, "");
	g_KeyboardRefMap.Add(DIK_MAIL, "");
	g_KeyboardRefMap.Add(DIK_MEDIASELECT, "");

/*
 *  Alternate names for keys, to facilitate transition from DOS.
#define DIK_BACKSPACE       DIK_BACK            /* backspace 
#define DIK_NUMPADSTAR      DIK_MULTIPLY        /* * on numeric keypad 
#define DIK_LALT            DIK_LMENU           /* left Alt 
#define DIK_CAPSLOCK        DIK_CAPITAL         /* CapsLock 
#define DIK_NUMPADMINUS     DIK_SUBTRACT        /* - on numeric keypad 
#define DIK_NUMPADPLUS      DIK_ADD             /* + on numeric keypad 
#define DIK_NUMPADPERIOD    DIK_DECIMAL         /* . on numeric keypad 
#define DIK_NUMPADSLASH     DIK_DIVIDE          /* / on numeric keypad 
#define DIK_RALT            DIK_RMENU           /* right Alt 
#define DIK_UPARROW         DIK_UP              /* UpArrow on arrow keypad 
#define DIK_PGUP            DIK_PRIOR           /* PgUp on arrow keypad 
#define DIK_LEFTARROW       DIK_LEFT            /* LeftArrow on arrow keypad 
#define DIK_RIGHTARROW      DIK_RIGHT           /* RightArrow on arrow keypad 
#define DIK_DOWNARROW       DIK_DOWN            /* DownArrow on arrow keypad 
#define DIK_PGDN            DIK_NEXT            /* PgDn on arrow keypad 
 */
}


//---------------------------------------------------
//	get the ProductID thats mapped to this ref. ZERO returned is assumed invalid
//---------------------------------------------------
u32 Platform::DirectX::GetDeviceProductIDFromRef(TRefRef DeviceRef)
{
	const u32* pGuid = g_DirectXDeviceProductIDRefs.FindKey( DeviceRef );
	if ( !pGuid )
		return 0;

	return *pGuid;
}

//---------------------------------------------------
//	get the ref thats mapped to this device ProductID. Optionally create a new entry
//---------------------------------------------------
TRef Platform::DirectX::GetDeviceRefFromProductID(u32 DeviceGuid,Bool CreateNew)
{
	const TRef* pRef = g_DirectXDeviceProductIDRefs.Find( DeviceGuid );
	if ( pRef )
		return *pRef;
	
	//	no entry - and not creating a new one
	if ( !CreateNew )
		return TRef();

	//	get new device ref
	TRef NewDeviceRef = TLInput::GetFreeDeviceRef();
	pRef = g_DirectXDeviceProductIDRefs.Add( DeviceGuid, NewDeviceRef );

	return *pRef;
}


void Platform::DirectX::InitialiseMouseRefMap()
{
	g_MouseRefMap.Add(DIMOFS_BUTTON0, "LMB");
	g_MouseRefMap.Add(DIMOFS_BUTTON1, "RMB");
	g_MouseRefMap.Add(DIMOFS_BUTTON2, "WHEEL");
	g_MouseRefMap.Add(DIMOFS_BUTTON3, "LMB2");
	g_MouseRefMap.Add(DIMOFS_BUTTON4, "RMB2");
}



void Platform::DirectX::InitialiseGamePadRefMaps()
{
	//TODO: Add individual arrays per gamepad type we support
	// These should be added to a generic array or manager or something rather than having a g_XXXarray
	// then we can have a list of pads that are supported too for easy identification and setup
	InitialiseXBox360PadRefMap();
	InitialiseWiimoteRefMap();
	InitialisePS2PadRefMap();
}

void Platform::DirectX::InitialiseXBox360PadRefMap()
{
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
}

void Platform::DirectX::InitialiseWiimoteRefMap()
{
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
}

void Platform::DirectX::InitialisePS2PadRefMap()
{
	g_PS2PadButtonRefs.Add(DIJOFS_BUTTON0, "CROSS");
	g_PS2PadButtonRefs.Add(DIJOFS_BUTTON1, "CIRCLE");
	g_PS2PadButtonRefs.Add(DIJOFS_BUTTON2, "SQUARE");
	g_PS2PadButtonRefs.Add(DIJOFS_BUTTON3, "TRIANGLE");
	g_PS2PadButtonRefs.Add(DIJOFS_BUTTON4, "START");
	g_PS2PadButtonRefs.Add(DIJOFS_BUTTON5, "SELECT");
	g_PS2PadButtonRefs.Add(DIJOFS_BUTTON6, "ANALOG");
	g_PS2PadButtonRefs.Add(DIJOFS_BUTTON7, "R1");
	g_PS2PadButtonRefs.Add(DIJOFS_BUTTON8, "L1");
	g_PS2PadButtonRefs.Add(DIJOFS_BUTTON9, "R2");
	g_PS2PadButtonRefs.Add(DIJOFS_BUTTON10, "L2");
	g_PS2PadButtonRefs.Add(DIJOFS_BUTTON11, "R3");
	g_PS2PadButtonRefs.Add(DIJOFS_BUTTON12, "L3");
}


Bool Platform::DirectX::GetSpecificButtonRef(const u32& uButtonID, TRefRef DeviceType, const u32& uProductID, TRef& LabelRef)
{
	TKeyArray<u32, TRef>* pArray = NULL;

	if(DeviceType == TLInput::KeyboardRef)
	{
		pArray = &g_KeyboardRefMap;
	}
	else if(DeviceType == TLInput::MouseRef)
	{
		pArray = &g_MouseRefMap;
	}
	else if(DeviceType == TLInput::GamepadRef)
	{
		// Test the product ID for known I'ds
		switch(uProductID)
		{
			case 0x028e045e:	// XBox 360 pad
				pArray = &g_Xbox360PadButtonRefs;
				break;
		}
	}

	if(pArray != NULL)
	{
		// Look for the specified ID
		TRef* pRef = pArray->Find(uButtonID);

		if(pRef)
		{
			LabelRef = *pRef;
			return TRUE;
		}
	}

	// No label found
	return FALSE;
}




SyncBool Platform::DirectX::EnumerateDevices()
{
	// Enumarate the attached devices
	HRESULT hr = g_pTLDirectInputInterface->EnumDevices(DI8DEVCLASS_ALL, CreateDevice, NULL, DIEDFL_ATTACHEDONLY);

	if(DI_OK != hr)
		return SyncFalse;

	return SyncTrue;
}


/*
	DirectX specific createdevice callback - called when enumerate devices is called.

	Creates the input device and maps it to the physical device.  
	Tests the device capabiliteis and attaches 'sensors' and 'effects' to the input device.
*/
BOOL CALLBACK Platform::DirectX::CreateDevice(const DIDEVICEINSTANCE* pdidInstance, VOID* pContext)
{
	u8 uDeviceType = GET_DIDEVICE_TYPE(pdidInstance->dwDevType);
	u8 uDeviceSubtype = GET_DIDEVICE_SUBTYPE(pdidInstance->dwDevType);

	// Set a unique ID for the generic input device - using the directx GUID data for now.
	// NOTE: A single device may actually call the callback multiple times for different elements i.e. a keyboard with a mouse built into it
	//			  but they will both have the same GUID.
	//	gr: I've renamed this ProductID as that's what it's called on the TLInputDirectXDevice
	u32 uDeviceProductID = pdidInstance->guidInstance.Data1;	// Unique instance ID (unique for each device)

	//	get device ref mapped to this guid
	TRef InstanceRef = TLInput::Platform::DirectX::GetDeviceRefFromProductID( uDeviceProductID, TRUE );

	//	if device already exists, skip over (as per comments above)
	if ( InstanceRef.IsValid() )
	{
		//	there is a GUID<->DeviceRef link, but check there is an ACTUAL device that's been created - we could have mapping, but no device
		TPtr<TLInput::TInputDevice>& pDevice = TLInput::GetDevice( InstanceRef );
		if ( pDevice )
		{
			// Device exists
			// return continue
			return DIENUM_CONTINUE;
		}
	}

	// Initialise the DirectX specific data required for mapping the generic device to the physical device
	Bool bProcessDevice = FALSE;
	TRef refDeviceType;

	// Check the device type and only process the ones we want to be processed
	switch(uDeviceType)
	{
		case DI8DEVTYPE_MOUSE:
			refDeviceType = TLInput::MouseRef;
			bProcessDevice = TRUE;
			break;
		case DI8DEVTYPE_KEYBOARD:
			refDeviceType = TLInput::KeyboardRef;
			bProcessDevice = TRUE;
			break;
		case DI8DEVTYPE_GAMEPAD:
			refDeviceType = TLInput::GamepadRef;
			bProcessDevice = TRUE;
			break;

			/*
			// [25/02/09] DB -
			// In DirectX a connected Wiimote appears as a Device with a Supplemental for the nunchuck.
			// Neither are actually usable via DirectX unfortunately as they never provide any buttons or axes.
			// I *believe* under Vista a Wiimote should function correctly as a gamepad but at this time I am using
			// WinXP. I will look into alternatives using a Wiimote library of some sort that I have come across
			// but these have drawback sin that most are for windows only so wouldn't work on the Max. :(
		case DI8DEVTYPE_DEVICE:
			refDeviceType = TLInput::GamepadRef;
			bProcessDevice = TRUE;
			break;
		case DI8DEVTYPE_SUPPLEMENTAL:
			refDeviceType = TLInput::GamepadRef;
			bProcessDevice = TRUE;
			break;
			*/
#ifdef _DEBUG
		default:
			TString str("Unhandled device type:");
			str.Appendf("%d %d", uDeviceType, uDeviceSubtype);
			TLDebug::Print(str);
			break;
#endif
	};

	if(bProcessDevice)
	{
		LPDIRECTINPUTDEVICE8 lpdiDevice;

		// Obtain an interface to the enumerated device. 
		HRESULT hr = g_pTLDirectInputInterface->CreateDevice(pdidInstance->guidInstance, &lpdiDevice, NULL); 

		if(DI_OK == hr)
		{
			// Create the generic input object
			TPtr<TInputDevice> pGenericDevice = g_pInputSystem->GetInstance(InstanceRef, TRUE, refDeviceType);

			if(pGenericDevice.IsValid())
			{
				if(!InitialiseDevice(pGenericDevice, lpdiDevice, pdidInstance))
				{
					// Failed to initialise the input device data

					// Find the physical device from the platform based list and remove it
					for(u32 uIndex = 0; uIndex < g_TLDirectInputDevices.GetSize(); uIndex++)
					{
						TPtr<TLInputDirectXDevice> pTLDirectInputDevice = g_TLDirectInputDevices.ElementAtConst(uIndex);
					
						if(pTLDirectInputDevice->GetDevice() == lpdiDevice)
						{
							g_TLDirectInputDevices.ElementAt(uIndex) = NULL;
							g_TLDirectInputDevices.RemoveAt(uIndex);
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
	}
	else
	{
		TLDebug::Print("Unknown input device found - ignoring");
	}

	return DIENUM_CONTINUE;

}



// DB - TEST
static u32 HardwareDeviceID = 0x45;

Bool Platform::DirectX::InitialiseDevice(TPtr<TInputDevice> pDevice, const LPDIRECTINPUTDEVICE8 lpdiDevice, const DIDEVICEINSTANCE* pdidInstance)
{
	// Generate a new device reference
	TRef HardwareDeviceRef = HardwareDeviceID;

	HardwareDeviceID++;

	// Create a platform specific device reference object
	TLInputDirectXDevice* pDXDevice = new TLInputDirectXDevice(HardwareDeviceRef, lpdiDevice);

	if(!pDXDevice)
		return FALSE;

	u8 uDeviceType = GET_DIDEVICE_TYPE(pdidInstance->dwDevType);
	u8 uDeviceSubtype = GET_DIDEVICE_SUBTYPE(pdidInstance->dwDevType);

	pDXDevice->SetProductID(pdidInstance->guidProduct.Data1);	// Unique product ID (same for all instances of the same devices)

	// Set quick access tot he device type without having to go through the directx interface
	pDXDevice->SetDeviceType(uDeviceType);

	// Default to bakcground non exclusive access 
	DWORD dwExclusivity = DISCL_NONEXCLUSIVE | DISCL_BACKGROUND;


	// Check the device type and determine the dataformat to use
	switch(uDeviceType)
	{
//		case DI8DEVTYPE_DEVICE:
//		case DI8DEVTYPE_SUPPLEMENTAL:
		case DI8DEVTYPE_GAMEPAD:
			{
				pDXDevice->SetDataFormat(&c_dfDIJoystick2);
				dwExclusivity = DISCL_EXCLUSIVE | DISCL_FOREGROUND;
			}
			break;
		case DI8DEVTYPE_MOUSE:
			{
				pDXDevice->SetDataFormat(&c_dfDIMouse2);
				//dwExclusivity = DISCL_EXCLUSIVE | DISCL_FOREGROUND;
			}
			break;
		case DI8DEVTYPE_KEYBOARD:
			{
				pDXDevice->SetDataFormat(&c_dfDIKeyboard);
			}
			break;
	};

	pDevice->AssignToHardwareDevice(HardwareDeviceRef);

	g_TLDirectInputDevices.Add(pDXDevice);


	// Assign the hardware device pointer to the generic input device
	// Set the data format
	HRESULT hr = lpdiDevice->SetDataFormat( &pDXDevice->GetDataFormat() );

	if(DI_OK != hr)
		return FALSE;

	HWND hWnd = TLCore::Platform::g_HWnd;

	// Set the cooperative level
	hr = lpdiDevice->SetCooperativeLevel(hWnd, dwExclusivity );

	//NOTE: needs to be exclusive access for release
	//hr = lpdiDevice->SetCooperativeLevel(hWnd, DISCL_EXCLUSIVE | DISCL_FOREGROUND);

	if(DI_OK != hr)
		return FALSE;

	u32 uBufferSize = INPUT_MOUSE_BUFFER_SIZE;

	if(uDeviceType == DI8DEVTYPE_GAMEPAD)
	{
		// Set the ranges for X Y and Z axes
		DIPROPRANGE range;
		range.diph.dwSize       = sizeof(range);
		range.diph.dwHeaderSize = sizeof(range.diph);
		range.diph.dwObj = 0;
		range.diph.dwHow	  = DIPH_DEVICE;
		range.lMin		  = -(s32)AXIS_SCALE;
		range.lMax		  = +(s32)AXIS_SCALE;

		hr = lpdiDevice->SetProperty(DIPROP_RANGE, &range.diph);

		// Only return if failed
		if(DI_OK != hr)
			return FALSE;

		uBufferSize = INPUT_GAMEPAD_BUFFER_SIZE;
	}
	else if(uDeviceType == DI8DEVTYPE_MOUSE)
	{
		uBufferSize = INPUT_MOUSE_BUFFER_SIZE;
	}

	// Setup the data buffer
	DIPROPDWORD dipdw;
	dipdw.diph.dwSize			= sizeof(dipdw);
	dipdw.diph.dwHeaderSize		= sizeof(dipdw.diph);
	dipdw.diph.dwObj			= 0;
	dipdw.diph.dwHow			= DIPH_DEVICE;
	dipdw.dwData				= uBufferSize;

	hr = lpdiDevice->SetProperty(DIPROP_BUFFERSIZE, &dipdw.diph);

	// Only return if failed
	if(DI_OK != hr)
		return FALSE;
	// Enum objects
	hr = lpdiDevice->EnumObjects(EnumDeviceObject, (void*)pDevice->GetDeviceRef().GetData(), DIDFT_ALL);

	if(DI_OK != hr)
		return FALSE;

	pDevice->SetAttached(TRUE);

	lpdiDevice->Acquire();

	// Success
	return TRUE;
}


BOOL CALLBACK Platform::DirectX::EnumDeviceObject(const DIDEVICEOBJECTINSTANCE* pdidObjectInstance, VOID* pContext)
{
	TRef refDeviceID = (u32)pContext;

	TPtr<TInputDevice> pDevice = g_pInputSystem->GetInstance(refDeviceID);

	if(pDevice.IsValid())
	{
		TPtr<TLInputDirectXDevice> pDXDevice = g_TLDirectInputDevices.FindPtr(pDevice->GetHardwareDeviceID());

		if(pDXDevice.IsValid())
		{
			// We have the associated device
			// Now setup the hardware device object for the generic device

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
		}
	}

	return DIENUM_CONTINUE;
}


Bool Platform::DirectX::UpdateDevice(TInputDevice& Device)
{
	// Get the physical device ID
	TRef DeviceID = Device.GetHardwareDeviceID();

	// Find the physical device from the platform based list
	TPtr<TLInputDirectXDevice>& pTLDirectInputDevice = g_TLDirectInputDevices.FindPtr(DeviceID);

	if ( !pTLDirectInputDevice )
		return FALSE;

	Bool bResult = FALSE;

	switch(pTLDirectInputDevice->GetDeviceType())
	{
	case DI8DEVTYPE_MOUSE:
			bResult = UpdateDirectXDevice_Mouse( Device, *pTLDirectInputDevice );
			break;
	case DI8DEVTYPE_KEYBOARD:
			bResult = UpdateDirectXDevice_Keyboard( Device, *pTLDirectInputDevice );
			break;
	case DI8DEVTYPE_GAMEPAD:
			bResult = UpdateDirectXDevice_Gamepad( Device, *pTLDirectInputDevice );
			break;
	}

	return bResult;
}


Bool Platform::DirectX::UpdateDirectXDevice_Mouse(TInputDevice& Device,TLInputDirectXDevice& TLDirectInputDevice)
{
	LPDIRECTINPUTDEVICE8 lpdiDevice = TLDirectInputDevice.GetDevice();

	// If valid poll and acquire the device ensuring no errors.
	if(lpdiDevice == 0)
		return FALSE;


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
				/*
				// Get the range of the mouse axis
				DIPROPRANGE range;
				range.diph.dwSize       = sizeof(range);
				range.diph.dwHeaderSize = sizeof(range.diph);
				range.diph.dwObj		= rgdod[uCount].dwOfs;
				range.diph.dwHow		= DIPH_BYOFFSET;

				hr = lpdiDevice->GetProperty(DIPROP_RANGE, &range.diph);

				if(SUCCEEDED(hr))
				{
					// Get value as a percentage of total range possible
					// Divide by the max range
					if(fValue > 0.0f)
					{
						s32 sMax = range.lMax;
						float fMax = (float)(sMax);
						fValue /= fMax;
					}
					else
					{
						s32 sMin = range.lMin;
						float fMin = (float)(sMin);
						fValue /= TLMaths::Absf(fMin);
					}
					fValue /= 100.0f;

					bSuccess = TRUE;
				}
				*/
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
}


//-------------------------------------------------------------------
//	for an axis, consolidate all the movement in an axis to one movement
//-------------------------------------------------------------------
Bool Platform::DirectX::ConslidateAxisMovement(TArray<TInputData>& InputBuffer,TRefRef AxisSensorRef)
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
Bool Platform::DirectX::UpdateDirectXDevice_Keyboard(TInputDevice& Device,TLInputDirectXDevice& TLDirectInputDevice)
{
	LPDIRECTINPUTDEVICE8 lpdiDevice = TLDirectInputDevice.GetDevice();

		// If valid poll and acquire the device ensuring no errors.
	if(lpdiDevice == 0)
		return FALSE;

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

	return TRUE;
}

/*	
	Special update routine for a joypad
*/
Bool Platform::DirectX::UpdateDirectXDevice_Gamepad(TInputDevice& Device,TLInputDirectXDevice& TLDirectInputDevice)
{
	LPDIRECTINPUTDEVICE8 lpdiDevice = TLDirectInputDevice.GetDevice();

		// If valid poll and acquire the device ensuring no errors.
	if(lpdiDevice == 0)
		return FALSE;

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
}



void TLInput::Platform::DirectX::TLInputDirectXDevice::PublishData(u32 uUniqueID, float fValue)
{
	TLMessaging::TMessage Message(uUniqueID);
	Message.Write(fValue);									// Value from sensor
	PublishMessage(Message);
}



//---------------------------------------------------
//	directx input shutdown
//---------------------------------------------------
SyncBool Platform::DirectX::Shutdown()
{

	if (g_pTLDirectInputInterface) 
	{ 
		Platform::DirectX::RemoveAllDevices();

		// Release the direct input interface
		g_pTLDirectInputInterface->Release(); 
		g_pTLDirectInputInterface = NULL; 
	} 

	g_KeyboardRefMap.Empty(TRUE);
	g_MouseRefMap.Empty(TRUE);

	g_Xbox360PadButtonRefs.Empty(TRUE);		
	g_WiimoteButtonRefs.Empty(TRUE);		
	g_PS2PadButtonRefs.Empty(TRUE);			

	g_DirectXDeviceProductIDRefs.Empty(TRUE);

	return SyncTrue;
}


void Platform::DirectX::RemoveAllDevices()
{
	g_TLDirectInputDevices.SetAll(NULL);
	g_TLDirectInputDevices.Empty(TRUE);
}



SyncBool Platform::CreateVirtualDevice(TRefRef InstanceRef, TRefRef DeviceTypeRef)
{	
	return SyncFalse;	
}

SyncBool Platform::RemoveVirtualDevice(TRefRef InstanceRef)
{
	return SyncFalse;	
}


