//-------------------------------------------------------
//	Win32Input.cpp
//-------------------------------------------------------


// Include files
//-------------------------------------------------------
#include "PCInput.h"
#include <TootleCore/TSyncQueue.h>
#include <TootleCore/TPtr.h>
#include <TootleCore/TEventChannel.h> //TEST

// DB - Quick define to switch between using the enum objects to instance buttons, axes, etc.
//#define DX_USE_ENUMOBJECTS 

// DB - Quick define to allow switching buffred and non-buffered mouse data
//#define DX_USE_BUFFERED_DATA

// User the enum objects if we are going to use buffered data
#ifdef DX_USE_BUFFERED_DATA
	#define DX_USE_ENUMOBJECTS 
#endif

// Globals
namespace TLInput
{
	namespace Platform
	{
		namespace DirectX
		{
			LPDIRECTINPUT8									g_pTLDirectInputInterface = NULL;	// Global direct input interface pointer
			TPtrArray<TLInputDirectXDevice>			g_TLDirectInputDevices;					// Global array of direct device pointers

			TPtr<TLInputDirectXDevice>					FindDirectXDevice(TRef DirectXDeviceRef);

			TKeyArray<u32, TRef>					g_KeyboardRefMap;

			TArray<TRef>							g_Xbox360PadButtonRefs;	// XBox 360 pad button refs
			TArray<TRef>							g_WiimoteButtonRefs;		// Wiimote button refs
			TArray<TRef>							g_PS2PadButtonRefs;		// PS2 pad button refs

			void	InitialiseKeyboadRefMap();
			void	InitialiseGamePadRefMaps();

			void	InitialiseXBox360PadRefMap();
			void	InitialiseWiimoteRefMap();
			void	InitialisePS2PadRefMap();

			Bool	GetSpecificGamepadButtonRef(const u32& uIndex, const u32& uProductID, TRef& LabelRef);
		};
	};

	const float AXIS_SCALE				= 1000.0f;
};

using namespace TLInput;



Bool Platform::UpdateDevice(TPtr<TInputDevice> pDevice)
{
	return DirectX::UpdateDevice(pDevice);
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
	g_KeyboardRefMap.Add(DIK_BACK, "K_BACK");
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
	g_KeyboardRefMap.Add(DIK_COMMA, "K_,");
	g_KeyboardRefMap.Add(DIK_PERIOD, "K_.");
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
	g_Xbox360PadButtonRefs.Add("A");
	g_Xbox360PadButtonRefs.Add("B");
	g_Xbox360PadButtonRefs.Add("X");
	g_Xbox360PadButtonRefs.Add("Y");
	g_Xbox360PadButtonRefs.Add("START");
	g_Xbox360PadButtonRefs.Add("BACK");
	g_Xbox360PadButtonRefs.Add("RB");
	g_Xbox360PadButtonRefs.Add("RT");
	g_Xbox360PadButtonRefs.Add("LB");
	g_Xbox360PadButtonRefs.Add("LT");
}

void Platform::DirectX::InitialiseWiimoteRefMap()
{
	g_WiimoteButtonRefs.Add("A");
	g_WiimoteButtonRefs.Add("1");
	g_WiimoteButtonRefs.Add("2");
	g_WiimoteButtonRefs.Add("B");
	g_WiimoteButtonRefs.Add("PLUS");
	g_WiimoteButtonRefs.Add("MINUS");
	g_WiimoteButtonRefs.Add("HOME");
	g_WiimoteButtonRefs.Add("POWER");
	g_WiimoteButtonRefs.Add("C");
	g_WiimoteButtonRefs.Add("Z");
}

void Platform::DirectX::InitialisePS2PadRefMap()
{
	g_PS2PadButtonRefs.Add("CROSS");
	g_PS2PadButtonRefs.Add("CIRCLE");
	g_PS2PadButtonRefs.Add("SQUARE");
	g_PS2PadButtonRefs.Add("TRIANGLE");
	g_PS2PadButtonRefs.Add("START");
	g_PS2PadButtonRefs.Add("SELECT");
	g_PS2PadButtonRefs.Add("ANALOG");
	g_PS2PadButtonRefs.Add("R1");
	g_PS2PadButtonRefs.Add("L1");
	g_PS2PadButtonRefs.Add("R2");
	g_PS2PadButtonRefs.Add("L2");
	g_PS2PadButtonRefs.Add("R3");
	g_PS2PadButtonRefs.Add("L3");
}


Bool Platform::DirectX::GetSpecificGamepadButtonRef(const u32& uIndex, const u32& uProductID, TRef& LabelRef)
{
	// Test the product ID for known I'ds
	TArray<TRef>* pArray = NULL;

	switch(uProductID)
	{
		case 0x028e045e:	// XBox 360 pad
			pArray = &g_Xbox360PadButtonRefs;
			break;
	}

	if(pArray && (uIndex < pArray->GetSize() ))
	{
		LabelRef = pArray->ElementAt(uIndex);
		return TRUE;
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
	u32 uDeviceID = pdidInstance->guidInstance.Data1;	// Unique instance ID (unique for each device)

	TRef InstanceRef = uDeviceID;

	TArray<TRef> refArray;
	if(g_pInputSystem->GetDeviceIDs(refArray))
	{
		// Check to see if the device has already been registered for use
		for(u32 uIndex = 0; uIndex < refArray.GetSize(); uIndex++)
		{
			if(refArray.ElementAt(uIndex) == InstanceRef)
			{
				// Device exists
				// return continue
				return DIENUM_CONTINUE;
			}
		}
	}


	// Initialise the DirectX specific data required for mapping the generic device to the physical device
	Bool bProcessDevice = FALSE;
	TRef refDeviceType;

	// Check the device type and only process the ones we want to be processed
	switch(uDeviceType)
	{
		case DI8DEVTYPE_MOUSE:
			refDeviceType = "MOUSE";
			bProcessDevice = TRUE;
			break;
		case DI8DEVTYPE_KEYBOARD:
			refDeviceType = "KEYBOARD";
			bProcessDevice = TRUE;
			break;
		case DI8DEVTYPE_GAMEPAD:
			refDeviceType = "GAMEPAD";
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
			refDeviceType = "GAMEPAD";
			bProcessDevice = TRUE;
			break;
		case DI8DEVTYPE_SUPPLEMENTAL:
			refDeviceType = "GAMEPAD";
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
					TPtr<TLMessaging::TMessage> pMessage = new TLMessaging::TMessage("Input");

					if(pMessage.IsValid())
					{
						pMessage->AddChannelID("DEVICE");									// device information message
						pMessage->AddChildAndData("STATE", TRef("ADDED"));					// state change
						pMessage->AddChildAndData("DEVID", pGenericDevice->GetDeviceRef());	// device ID
						pMessage->AddChildAndData("TYPE", pGenericDevice->GetDeviceType());					// device type

						g_pInputSystem->PublishMessage(pMessage);
					}
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

	u32 uProductID = pdidInstance->guidProduct.Data1;	// Unique product ID (same for all instances of the same devices)

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

	if((uDeviceType == DI8DEVTYPE_GAMEPAD))
	{
		// Now set the ranges for X Y and Z axes
		DIPROPRANGE range;
		range.diph.dwSize       = sizeof(range);
		range.diph.dwHeaderSize = sizeof(range.diph);
		range.diph.dwHow	  = DIPH_DEVICE;
		range.lMin		  = -(s32)AXIS_SCALE;
		range.lMax		  = +(s32)AXIS_SCALE;
		range.diph.dwObj = 0;

		hr = lpdiDevice->SetProperty(DIPROP_RANGE, &range.diph);

		// Only return if failed
		if(DI_OK != hr)
			return FALSE;
	}
//#ifdef DX_USE_BUFFERED_DATA
	else if(uDeviceType == DI8DEVTYPE_MOUSE)
	{
		// Setup the mouse data buffer
		DIPROPDWORD dipdw;
		dipdw.diph.dwSize			= sizeof(dipdw);
		dipdw.diph.dwHeaderSize		= sizeof(dipdw.diph);
		dipdw.diph.dwObj			= 0;
		dipdw.diph.dwHow			= DIPH_DEVICE;
		dipdw.dwData				= 16;	// Buffer size

		hr = lpdiDevice->SetProperty(DIPROP_BUFFERSIZE, &dipdw.diph);

		// Only return if failed
		if(DI_OK != hr)
			return FALSE;
	}
//#endif

#ifdef DX_USE_ENUMOBJECTS
	// Enum objects
	hr = lpdiDevice->EnumObjects(EnumDeviceObject, (void*)pDevice->GetDeviceID().GetData(), DIDFT_ALL);

	if(DI_OK != hr)
		return FALSE;
#else

	// Get the device's capabilities.  This data will be used to determine what sensors to attach to the generic input device.
	DIDEVCAPS DIDevCaps;
	DIDevCaps.dwSize = sizeof(DIDEVCAPS); 

	hr = lpdiDevice->GetCapabilities( &DIDevCaps );

	if(DI_OK != hr)
		return FALSE;

	// Add button inputs
	u32 uIndex = 0;
	u32 uCount = DIDevCaps.dwButtons;

	// Force 256 buttons to be added.  This ensures the button range for the DIK values are used correctly
	if(uDeviceType == DI8DEVTYPE_KEYBOARD)
		uCount = 256;

	
	u32 uUniqueID = 0;

	TString stringLabel;
	TRef refLabel;

	for(uIndex = 0; uIndex < uCount; uIndex++)
	{
		// For buttons we need to label them based on what type and the model
		// so get this information from a function instead which will lookup the details required
		TPtr<TInputSensor> pSensor = pDevice->AttachSensor(uUniqueID, Button);

		if(pSensor.IsValid())
		{
			refLabel = GetDefaultButtonRef(uIndex);

			pSensor->AddLabel(refLabel);

			if(uDeviceType == DI8DEVTYPE_KEYBOARD)
			{
				u32 dikvalue = uIndex;
				TRef* pKeyLabel = g_KeyboardRefMap.Find(dikvalue);
				
				if(pKeyLabel)
					pSensor->AddLabel(*pKeyLabel);
			}
			else if(uDeviceType == DI8DEVTYPE_GAMEPAD)
			{
				if(GetSpecificGamepadButtonRef(uIndex, uProductID, refLabel))
				{
					pSensor->AddLabel(refLabel);
				}
			}

			pSensor->SubscribeTo(pDXDevice);

			uUniqueID++;
		}
	}

	// Add axes inputs
	uCount = DIDevCaps.dwAxes;

	for(uIndex = 0; uIndex < uCount; uIndex++)
	{
		TPtr<TInputSensor> pSensor = pDevice->AttachSensor(uUniqueID, Axis);

		if(pSensor.IsValid())
		{
			refLabel = GetDefaultAxisRef(uIndex);

			pSensor->AddLabel(refLabel);
			pSensor->SubscribeTo(pDXDevice);
			uUniqueID++;
		}
	}

	// Add point of view inputs - cap axes on top of joysticks
	uCount = DIDevCaps.dwPOVs;

	for(uIndex = 0; uIndex < uCount; uIndex++)
	{
		TPtr<TInputSensor> pSensor = pDevice->AttachSensor(uUniqueID, POV);

		if(pSensor.IsValid())
		{
			refLabel = GetDefaultPOVRef(uIndex);

			pSensor->AddLabel(refLabel);
			pSensor->SubscribeTo(pDXDevice);
			uUniqueID++;
		}
	}
#endif

	// TODO: Add effects i.e. force feedback, speakers, etc
	/*
	for(uIndex = 0; uIndex < uButtonCount; uIndex++)
	{
		pDevice->AttachEffect();
	}
	*/
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
		TPtr<TLInputDirectXDevice> pDXDevice = FindDirectXDevice(pDevice->GetHardwareDeviceID());

		if(pDXDevice.IsValid())
		{
			// We have the associated device
			// Now setup the hardware device object for the generic device

			TSensorType SensorType = Unknown;
			TRef	DefaultLabel;
			switch(DIDFT_GETTYPE(pdidObjectInstance->dwType))
			{
			case DIDFT_PSHBUTTON:
			case DIDFT_TGLBUTTON:
			case DIDFT_BUTTON:
				SensorType = Button;
				DefaultLabel = GetDefaultButtonRef(pDevice->GetSensorCount(SensorType));
				break;

			case DIDFT_RELAXIS:
			case DIDFT_ABSAXIS:
			case DIDFT_AXIS:
				SensorType = Axis;
				DefaultLabel = GetDefaultAxisRef(pDevice->GetSensorCount(SensorType));
				break;

			case DIDFT_POV:
				SensorType = POV;
				DefaultLabel = GetDefaultPOVRef(pDevice->GetSensorCount(SensorType));
				break;
			}

			if(SensorType != Unknown)
			{
				u32 uInstanceID = DIDFT_GETINSTANCE(pdidObjectInstance->dwType);

				TPtr<TInputSensor> pSensor = pDevice->AttachSensor(uInstanceID, SensorType);

				if(pSensor.IsValid())
				{
					pSensor->AddLabel(DefaultLabel);
					pSensor->SubscribeTo(pDXDevice);
				}
			}
		}
	}

	return DIENUM_CONTINUE;
}


Bool Platform::DirectX::UpdateDevice(TPtr<TInputDevice> pDevice)
{
	// Get the physical device ID
	TRef DeviceID = pDevice->GetHardwareDeviceID();

	// Find the physical device from the platform based list
	TPtr<TLInputDirectXDevice> pTLDirectInputDevice = FindDirectXDevice(DeviceID);

	Bool bResult = FALSE;

	if(pTLDirectInputDevice.IsValid())
	{
		switch(pTLDirectInputDevice->GetDeviceType())
		{
		case DI8DEVTYPE_MOUSE:
				bResult = UpdateDirectXDevice_Mouse(pDevice, pTLDirectInputDevice);
				break;
		case DI8DEVTYPE_KEYBOARD:
				bResult = UpdateDirectXDevice_Keyboard(pDevice, pTLDirectInputDevice);
				break;
		case DI8DEVTYPE_GAMEPAD:
				bResult = UpdateDirectXDevice_Gamepad(pDevice, pTLDirectInputDevice);
				break;
		}
	}

	return bResult;
}

TPtr<Platform::DirectX::TLInputDirectXDevice> Platform::DirectX::FindDirectXDevice(TRef DirectXDeviceRef)
{
	TPtr<TLInputDirectXDevice> pResult = NULL;
	
	for(u32 uIndex = 0; uIndex < g_TLDirectInputDevices.GetSize(); uIndex++)
	{
		TPtr<TLInputDirectXDevice> pDXDevice = g_TLDirectInputDevices.ElementAtConst(uIndex);
	
		if(pDXDevice->GetDeviceID() == DirectXDeviceRef)
		{
			// Found the paltform specific device data
			pResult = pDXDevice;
			break;
		}
	}

	return pResult;
}



#ifdef DX_USE_BUFFERED_DATA

Bool Platform::DirectX::UpdateDirectXDevice_Mouse(TPtr<TInputDevice> pDevice, TPtr<TLInputDirectXDevice> pTLDirectInputDevice)
{
	LPDIRECTINPUTDEVICE8 lpdiDevice = pTLDirectInputDevice->GetDevice();

	// If valid poll and acquire the device ensuring no errors.
	if(lpdiDevice == 0)
		return FALSE;


	lpdiDevice->Poll();

	// Now read some data
	DIDEVICEOBJECTDATA		rgdod[16];
	DWORD					dwItems = 16;

	HRESULT hr = lpdiDevice->GetDeviceData(sizeof(DIDEVICEOBJECTDATA), rgdod, &dwItems, 0);

	if (SUCCEEDED(hr)) 
	{ 
	    // dwItems = Number of elements read (could be zero).

		if (hr == DI_BUFFEROVERFLOW) 
		{	
			// Buffer had overflowed. 
			TLDebug::Print("Input Buffer overflow");
		} 


		/////////////////////////////////////////////////////////////
		// Copy data from the physical device to the generic input device for the sensors to use
		/////////////////////////////////////////////////////////////

#ifdef _DEBUG
		TString inputinfo = "Mouse processing buffer: ";
		inputinfo.Appendf("%d items", dwItems);
		TLDebug::Print(inputinfo);
#endif


		for(u32 uCount = 0; uCount < dwItems; uCount++)
		{
			// Go through all of the buffered data

			TPtr<TBinaryTree>& MainBuffer = pDevice->GetDataBuffer();

			MainBuffer->Empty();

			TPtr<TBinaryTree> pDataBuffer = MainBuffer->AddChild("Input");

			if(pDataBuffer.IsValid())
			{
				u32 uSensorCount = pDevice->GetSensorCount(Button);

				for(u32 uIndex = 0; uIndex < uSensorCount; uIndex++)
				{
					u32 ButtonOffset = (FIELD_OFFSET(DIMOUSESTATE, rgbButtons) + uIndex);

					// Assume a zero value
					float fValue = 0.0f;

					if(rgdod[uCount].dwOfs == ButtonOffset)
					{
						// get the data from the buffer
						fValue = (float)(rgdod[uCount].dwData);

						TLDebug::Print("Buffer Data");
					}

					pDataBuffer->Write( fValue );

#ifdef _DEBUG
					// In debug print what button was pressed
					TString inputinfo = "Mouse input: ";
					inputinfo.Appendf("%d %.2f", uIndex, fValue);
					TLDebug::Print(inputinfo);
#endif

				}

				if(rgdod[uCount].dwOfs == DIMOFS_X)
					pDataBuffer->Write( (float)(rgdod[uCount].dwData) );
				else
					pDataBuffer->Write(0.0f);

				if(rgdod[uCount].dwOfs == DIMOFS_Y)
					pDataBuffer->Write( (float)(rgdod[uCount].dwData) );		
				else
					pDataBuffer->Write(0.0f);


				if(rgdod[uCount].dwOfs == DIMOFS_Y)
					pDataBuffer->Write( (float)(rgdod[uCount].dwData) );		
				else
					pDataBuffer->Write(0.0f);
			}

			// Tell the device to process the data
			pDataBuffer->ResetReadPos();
			pDevice->ForceUpdate();
		}

		/////////////////////////////////////////////////////////////

	}

	return (hr == DI_OK);
}

#else

/*	
	Special update routine for a mouse using directx
*/
Bool Platform::DirectX::UpdateDirectXDevice_Mouse(TPtr<TInputDevice> pDevice, TPtr<TLInputDirectXDevice> pTLDirectInputDevice)
{
	LPDIRECTINPUTDEVICE8 lpdiDevice = pTLDirectInputDevice->GetDevice();

	// If valid poll and acquire the device ensuring no errors.
	if(lpdiDevice == 0)
		return FALSE;

	lpdiDevice->Poll();

	// Success - now read some data
	DIMOUSESTATE2 dims;      // DirectInput mouse state structure

	// Get the input's device state, and put the state in dims
	ZeroMemory( &dims, sizeof( dims ) );
	HRESULT hr = lpdiDevice->GetDeviceState( sizeof( DIMOUSESTATE2 ), &dims );

	if( FAILED( hr ) )
	{
		// DirectInput may be telling us that the input stream has been
		// interrupted.  We aren't tracking any state between polls, so
		// we don't have any special reset that needs to be done.
		// We just re-acquire and try again next time.

		// Update the dialog text 
		if( hr == DIERR_OTHERAPPHASPRIO ||
			hr == DIERR_NOTACQUIRED )
		{
			
		}

		//hr == DIERR_INPUTLOST

		// hr may be DIERR_OTHERAPPHASPRIO or other errors.  This
		// may occur when the app is minimized or in the process of 
		// switching, so just try again later 

		hr = lpdiDevice->Acquire();
        while( hr == DIERR_INPUTLOST ) 
            hr = lpdiDevice->Acquire();

		return FALSE;
	}

	/////////////////////////////////////////////////////////////
	// Copy data from the physical device to the generic input device for the sensors to use
	/////////////////////////////////////////////////////////////

	TPtr<TBinaryTree>& MainBuffer = pDevice->GetDataBuffer();

	MainBuffer->Empty();

	TPtr<TBinaryTree> pDataBuffer = MainBuffer->AddChild("Input");

	if(pDataBuffer.IsValid())
	{

		u32 uSensorCount = pDevice->GetSensorCount(Button);

		// Write the button data to the buffer
		for(u32 uIndex = 0; uIndex < uSensorCount; uIndex++)
		{
			float fValue = (dims.rgbButtons[uIndex] == 0 ? 0.0f : 1.0f);
			pDataBuffer->Write( fValue );

#ifdef _DEBUG
			// In debug print what button was pressed
			if(fValue > 0.0f)
			{
				TString inputinfo = "Mouse input: ";
				inputinfo.Appendf("%d %.2f", uIndex, fValue);
				TLDebug::Print(inputinfo);
			}
#endif


		}

#ifdef _DEBUG
			// In debug print what axis movements occured
			if(dims.lX > 0)
			{
				TString inputinfo = "Mouse X-Axis input: ";
				inputinfo.Appendf("%.2f, %.2f", (float)dims.lX, (float)dims.lX/100.0f);
				TLDebug::Print(inputinfo);
			}

			if(dims.lY > 0)
			{
				TString inputinfo = "Mouse Y-Axis input: ";
				inputinfo.Appendf("%.2f, %.2f", (float)dims.lY, (float)dims.lY/100.0f);
				TLDebug::Print(inputinfo);
			}

			if(dims.lZ > 0)
			{
				TString inputinfo = "Mouse Z-Axis input: ";
				inputinfo.Appendf("%.2f, %.2f", (float)dims.lZ, (float)dims.lZ/100.0f);
				TLDebug::Print(inputinfo);
			}

#endif


		// Write the axis data to the buffer - values are relative (and in pixels?)
		pDataBuffer->Write( (float)(dims.lX)/100.0f );		
		pDataBuffer->Write( (float)(dims.lY)/100.0f );		
		pDataBuffer->Write( (float)(dims.lZ)/100.0f );		
	}

	/////////////////////////////////////////////////////////////

	return TRUE;
}

#endif

/*	
	Special update routine for a keyboard using directx
*/
Bool Platform::DirectX::UpdateDirectXDevice_Keyboard(TPtr<TInputDevice> pDevice, TPtr<TLInputDirectXDevice> pTLDirectInputDevice)
{
	LPDIRECTINPUTDEVICE8 lpdiDevice = pTLDirectInputDevice->GetDevice();

		// If valid poll and acquire the device ensuring no errors.
	if(lpdiDevice == 0)
		return FALSE;

	// Acquire the device
	HRESULT hr = lpdiDevice->Acquire(); 

	if(FAILED( hr ))
		return FALSE;

	lpdiDevice->Poll();

	BYTE diks[256];   // DirectInput keyboard state buffer 

	// Get the input's device state, and put the state in dims
	ZeroMemory( diks, sizeof( diks ) );
	hr = lpdiDevice->GetDeviceState( sizeof( diks ), diks );

	if( FAILED( hr ) )
	{
		// DirectInput may be telling us that the input stream has been
		// interrupted.  We aren't tracking any state between polls, so
		// we don't have any special reset that needs to be done.
		// We just re-acquire and try again.
		return FALSE;
	}
	
	TPtr<TBinaryTree>& MainBuffer = pDevice->GetDataBuffer();

	MainBuffer->Empty();

	TPtr<TBinaryTree> pDataBuffer = MainBuffer->AddChild("Input");

	if(pDataBuffer.IsValid())
	{

		u32 uSensorCount = pDevice->GetSensorCount(Button);

		// Publish the keyboard data
		for(u32 uIndex = 0; uIndex < uSensorCount; uIndex++)
		{
			//	gr: the uIndex is equivelent to DIK_ESCAPE, DIK_SPACE etc... convert these to some Tootle keyboard/alphabet/keys?
			//		the we can easily convert from input source (mac keyboard, ipod keyboard, dx keyboard) to alphabet or non-platform
			//		specific buttons

			// For every keyboard button publish it's value - if any sensors subscribe to the 
			// keyboard buttons they will process the information
			float fValue = (diks[uIndex] & 0x80) ? 1.0f : 0.0f;
		
			pDataBuffer->Write( fValue );

#ifdef _DEBUG
			// In debug print what key was pressed
			if(fValue > 0.0f)
			{
				TString inputinfo = "Keyboard input: ";
				inputinfo.Appendf("%d %.2f", uIndex, fValue);
				TLDebug::Print(inputinfo);
			}
#endif
		}
	}

	return TRUE;
}

/*	
	Special update routine for a joypad
*/
Bool Platform::DirectX::UpdateDirectXDevice_Gamepad(TPtr<TInputDevice> pDevice, TPtr<TLInputDirectXDevice> pTLDirectInputDevice)
{
	LPDIRECTINPUTDEVICE8 lpdiDevice = pTLDirectInputDevice->GetDevice();

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

	// Success - now read the data
	DIJOYSTATE2 js;           // DInput joystick state 

    // Get the input's device state
	hr = lpdiDevice->GetDeviceState( sizeof( DIJOYSTATE2 ), &js );

    if( FAILED( hr) )
	{

	}
	else
	{
		TPtr<TBinaryTree>& MainBuffer = pDevice->GetDataBuffer();

		MainBuffer->Empty();

		TPtr<TBinaryTree> pDataBuffer = MainBuffer->AddChild("Input");

		if(pDataBuffer.IsValid())
		{

			u32 uSensorCount = pDevice->GetSensorCount(Button);

			// Write the button data to the buffer
			for(u32 uIndex = 0; uIndex < uSensorCount; uIndex++)
			{
				float fValue = ((js.rgbButtons[uIndex] == 0) ? 0.0f : 1.0f);
				pDataBuffer->Write( fValue );

#ifdef _DEBUG
			// In debug print what button was pressed
			if(fValue > 0.0f)
			{
				TString inputinfo = "Gamepad input: ";
				inputinfo.Appendf("%d %.2f", uIndex, fValue);
				TLDebug::Print(inputinfo);
			}
#endif

			}

			// Write the axis data to the buffer
			pDataBuffer->Write( (float)(js.lX / AXIS_SCALE) );		// Left analogue stick on XB360 pad
			pDataBuffer->Write( (float)(js.lY / AXIS_SCALE) );		// Left analogue stick on XB360 pad

			pDataBuffer->Write( (float)(js.lRx / AXIS_SCALE) );		// Right analogue stick on XB360 pad
			pDataBuffer->Write( (float)(js.lRy / AXIS_SCALE) );		// Right analogue stick on XB360 pad

			pDataBuffer->Write( (float)(js.lZ / AXIS_SCALE) );		// L+R analogue buttons combined on XB360 pad

			// Write POV data
			// float fValue = js.POV[0];		// D-Pad on XB360 Pad
			//dataBuffer->Write( fValue );						
			pDataBuffer->Write( 0.0f );		// D-Pad on XB360 Pad - not sure how we use one value for the whole pad which essentially has 2 axes?				
		}

		// TODO: Copy data from the generic device to the physical device for the effects output?
	}

	return (hr == DI_OK);
}



void TLInput::Platform::DirectX::TLInputDirectXDevice::PublishData(u32 uUniqueID, float fValue)
{
	TPtr<TLMessaging::TMessage>	 pMessage = new TLMessaging::TMessage("Input");

	if(pMessage.IsValid())
	{
		pMessage->AddChannelID(uUniqueID);						// Sensor ID which we want to publish to
		pMessage->Write(fValue);								// Value from sensor
		PublishMessage(pMessage);
	}
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

	g_KeyboardRefMap.Empty();

	return SyncTrue;
}


void Platform::DirectX::RemoveAllDevices()
{
	g_TLDirectInputDevices.SetAll(NULL);
	g_TLDirectInputDevices.Empty(TRUE);
}


