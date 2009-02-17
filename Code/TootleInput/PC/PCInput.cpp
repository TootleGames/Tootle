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
//#define DX_USE_BUFFERED_MOUSE_DATA

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
	u32 uid = pdidInstance->guidInstance.Data1;
	TRef InstanceRef = uid;

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
				if(!InitialiseDevice(pGenericDevice, lpdiDevice, uDeviceType))
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

	return DIENUM_CONTINUE;

}



// DB - TEST
static u32 HardwareDeviceID = 0x45;

Bool Platform::DirectX::InitialiseDevice(TPtr<TInputDevice> pDevice, LPDIRECTINPUTDEVICE8 lpdiDevice, u32 uDeviceType)
{
	// Generate a new device reference
	TRef HardwareDeviceRef = HardwareDeviceID;

	HardwareDeviceID++;

	// Create a platform specific device reference object
	TLInputDirectXDevice* pDXDevice = new TLInputDirectXDevice(HardwareDeviceRef, lpdiDevice);

	if(!pDXDevice)
		return FALSE;

	// Set quick access tot he device type without having to go through the directx interface
	pDXDevice->SetDeviceType(uDeviceType);

	// Default to bakcground non exclusive access 
	DWORD dwExclusivity = DISCL_NONEXCLUSIVE | DISCL_BACKGROUND;

	// Check the device type and determine the dataformat to use
	switch(uDeviceType)
	{
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

	if(uDeviceType == DI8DEVTYPE_GAMEPAD)
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
//#ifdef DX_USE_BUFFERED_MOUSE_DATA
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
	
	//	gr: bodge so that keyboard will read all 256 keys in the sensor code
	if(uDeviceType == DI8DEVTYPE_KEYBOARD)
		uCount = 256;

	u32 uUniqueID = 0;

	TString stringLabel;
	TRef refLabel;


	for(uIndex = 0; uIndex < uCount; uIndex++)
	{
		// For buttons we need to label them based on what type and the model
		// so get this information from a function in stead which will lookup the details required

		TPtr<TInputSensor> pSensor = pDevice->AttachSensor(uUniqueID, Button);

		if(pSensor.IsValid())
		{
			pSensor->SubscribeTo(pDXDevice);

			uUniqueID++;
		}
	}

	// Add axes inputs
	uCount = DIDevCaps.dwAxes;

	TArray<TRef> AxisRefs;

	AxisRefs.Add("AXX1");
	AxisRefs.Add("AXY1");
	AxisRefs.Add("AXZ1");
	AxisRefs.Add("AXX2");
	AxisRefs.Add("AXY2");
	AxisRefs.Add("AXZ2");
	AxisRefs.Add("AXX3");
	AxisRefs.Add("AXY3");
	AxisRefs.Add("AXZ3");
	AxisRefs.Add("AXX4");
	AxisRefs.Add("AXY4");
	AxisRefs.Add("AXZ4");

	u32 uArrayIndex = 0;
	for(uIndex = 0; uIndex < uCount; uIndex++)
	{
		// FILTHY HACK FOR NOW UNTIL I GET BETTER REF BUILDING
		if(uIndex > AxisRefs.GetSize())
		{
			TLDebug_Break("Exceeded temp ref array size");
			break;
		}

		if(uIndex % 3)
			uArrayIndex = 0;

		AxisRefs.ElementAt(uArrayIndex).GetString(stringLabel); // get the appropriate axis type x,y,z

		refLabel = stringLabel;

		TPtr<TInputSensor> pSensor = pDevice->AttachSensor(uUniqueID, Axis);

		if(pSensor.IsValid())
		{
			pSensor->SetLabel(refLabel);
			pSensor->SubscribeTo(pDXDevice);
			uUniqueID++;
		}

		uArrayIndex++;
	}

	// Add point of view inputs - cap axes on top of joysticks
	uCount = DIDevCaps.dwPOVs;

	for(uIndex = 0; uIndex < uCount; uIndex++)
	{
		stringLabel = "POV";
		stringLabel.Appendf("%d", uCount);
		refLabel = stringLabel;

		TPtr<TInputSensor> pSensor = pDevice->AttachSensor(uUniqueID, POV);

		if(pSensor.IsValid())
		{
			pSensor->SetLabel(refLabel);
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
			switch(DIDFT_GETTYPE(pdidObjectInstance->dwType))
			{
			case DIDFT_PSHBUTTON:
			case DIDFT_TGLBUTTON:
			case DIDFT_BUTTON:
				SensorType = Button;
				break;

			case DIDFT_RELAXIS:
			case DIDFT_ABSAXIS:
			case DIDFT_AXIS:
				SensorType = Axis;
				break;

			case DIDFT_POV:
				SensorType = POV;
				break;
			}

			if(SensorType != Unknown)
			{
				u32 uInstanceID = DIDFT_GETINSTANCE(pdidObjectInstance->dwType);

				TPtr<TInputSensor> pSensor = pDevice->AttachSensor(uInstanceID, SensorType);

				if(pSensor.IsValid())
				{
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



#ifdef DX_USE_BUFFERED_MOUSE_DATA

Bool Platform::DirectX::UpdateDirectXDevice_Mouse(TPtr<TInputDevice> pDevice, TPtr<TLInputDirectXDevice> pTLDirectInputDevice)
{
	LPDIRECTINPUTDEVICE8 lpdiDevice = pTLDirectInputDevice->GetDevice();

	// If valid poll and acquire the device ensuring no errors.
	if(lpdiDevice == 0)
		return FALSE;

	// Try and aqcuire the device
	 HRESULT hr = lpdiDevice->Acquire(); 

	 if(FAILED(hr))
 	 {
		while(hr == DIERR_NOTACQUIRED)
			hr = lpdiDevice->Acquire();

		 return FALSE;
	 }

	 lpdiDevice->Poll();

	// Success - now read some data

 	DIMOUSESTATE2 dims;      // DirectInput mouse state structure

	// Get the input's device state, and put the state in dims
	ZeroMemory( &dims, sizeof( dims ) );
	hr = lpdiDevice->GetDeviceState( sizeof( DIMOUSESTATE2 ), &dims );

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
	}
	else
	{
		/////////////////////////////////////////////////////////////
		// Copy data from the physical device to the generic input device for the sensors to use
		/////////////////////////////////////////////////////////////

		DIDEVICEOBJECTDATA		rgdod[16];
		DWORD					dwItems;

		hr = lpdiDevice->GetDeviceData(sizeof(DIDEVICEOBJECTDATA), rgdod, &dwItems, 0);

		if( FAILED( hr ) )
			dwItems = 0;

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
			}

			// Accumulate mouse data from buffer
			s32 uMouseX = 0;
			s32 uMouseY = 0;
			for(u32 uIndex = 0; uIndex < dwItems; uIndex++)
			{
				// Increment the mouse X axis 
				if(rgdod[uIndex].dwOfs == DIMOFS_X)
					uMouseX += rgdod[uIndex].dwData;
				// Increment the mouse X axis 
				if(rgdod[uIndex].dwOfs == DIMOFS_Y)
					uMouseY += rgdod[uIndex].dwData;
			}

			// Write the axis data to the buffer - values are relative (and in pixels?)
			// Prioritise the mouse buffer axis data over the immediate data
			if(dims.lX > uMouseX)
				uMouseX = dims.lX;

			if(dims.lY > uMouseY)
				uMouseY = dims.lY;

			pDataBuffer->Write( (float)(uMouseX) );
			pDataBuffer->Write( (float)(uMouseY) );		
	
			pDataBuffer->Write( (float)(dims.lZ) );		
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

	/*
	// Try and aqcuire the device
	 HRESULT hr = lpdiDevice->Acquire(); 

	 if(FAILED(hr))
	 {
 		while(hr == DIERR_INPUTLOST)
			hr = lpdiDevice->Acquire();
		//return FALSE;
	 }
	 */

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

	return SyncTrue;
}


void Platform::DirectX::RemoveAllDevices()
{
	g_TLDirectInputDevices.SetAll(NULL);
	g_TLDirectInputDevices.Empty(TRUE);
}


