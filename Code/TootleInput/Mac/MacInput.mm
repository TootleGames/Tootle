#include "MacInput.h"

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
		namespace Mac 
		{						
			void	SetCursorPosition(u8 uIndex, int2 uPos);			
			// Internal
			void			InitialiseKeyboadRefMap();
			
			Bool			InitialisePhysicalDevice(TPtr<TInputDevice> pDevice, TRefRef DeviceTypeRef);
			Bool			InitialiseVirtualDevice(TPtr<TInputDevice> pDevice, TRefRef DeviceTypeRef);

			Bool			CreateVirtualKeyboard();
			Bool			DestroyVirtualKeyboard();
			
			Bool			UpdatePhysicalDevice(TLInput::TInputDevice& Device);
			Bool			UpdateVirtualDevice(TLInput::TInputDevice& Device);

			// TEMP test routine
			void			VibrateDevice();
		}
	}
}


using namespace TLInput;

// Not used on the ipod
SyncBool Platform::Init()
{		
	IPod::InitialiseKeyboadRefMap();
	return SyncTrue;	
}

void Platform::IPod::InitialiseKeyboadRefMap()
{
	g_KeyboardRefMap.Add("ESC", "K_ESC");
	g_KeyboardRefMap.Add("1", "K_1");
	g_KeyboardRefMap.Add("2", "K_2");
	g_KeyboardRefMap.Add("3", "K_3");
	g_KeyboardRefMap.Add("4", "K_4");
	g_KeyboardRefMap.Add("5", "K_5");
	g_KeyboardRefMap.Add("6", "K_6");
	g_KeyboardRefMap.Add("7", "K_7");
	g_KeyboardRefMap.Add("8", "K_8");
	g_KeyboardRefMap.Add("9", "K_9");
	g_KeyboardRefMap.Add("0", "K_0");
	g_KeyboardRefMap.Add("-", "K_MINUS");
	g_KeyboardRefMap.Add("=", "K_EQUALS");
	g_KeyboardRefMap.Add("BACKSPACE", "K_BACKSPACE");
	g_KeyboardRefMap.Add("TAB", "K_TAB");
	g_KeyboardRefMap.Add("Q", "K_Q");
	g_KeyboardRefMap.Add("W", "K_W");
	g_KeyboardRefMap.Add("E", "K_E");
	g_KeyboardRefMap.Add("R", "K_R");
	g_KeyboardRefMap.Add("T", "K_T");
	g_KeyboardRefMap.Add("Y", "K_Y");
	g_KeyboardRefMap.Add("U", "K_U");
	g_KeyboardRefMap.Add("I", "K_I");
	g_KeyboardRefMap.Add("O", "K_O");
	g_KeyboardRefMap.Add("P", "K_P");
	g_KeyboardRefMap.Add("(", "K_LBRACKET");
	g_KeyboardRefMap.Add(")", "K_RBRACKET");
	g_KeyboardRefMap.Add("RETURN", "K_RETURN");
	g_KeyboardRefMap.Add("LCTRL", "K_LCTRL");
	g_KeyboardRefMap.Add("A", "K_A");
	g_KeyboardRefMap.Add("S", "K_S");
	g_KeyboardRefMap.Add("D", "K_D");
	g_KeyboardRefMap.Add("F", "K_F");
	g_KeyboardRefMap.Add("G", "K_G");
	g_KeyboardRefMap.Add("H", "K_H");
	g_KeyboardRefMap.Add("J", "K_J");
	g_KeyboardRefMap.Add("K", "K_K");
	g_KeyboardRefMap.Add("L", "K_L");
	g_KeyboardRefMap.Add(":", "K_COLON");
	g_KeyboardRefMap.Add("'", "K_APOSTROPHE");
	g_KeyboardRefMap.Add("#", "K_GRAVE");
	g_KeyboardRefMap.Add("LSHIFT", "K_LSHIFT");
	g_KeyboardRefMap.Add("\\", "K_BACKSLASH");
	g_KeyboardRefMap.Add("Z", "K_Z");
	g_KeyboardRefMap.Add("X", "K_X");
	g_KeyboardRefMap.Add("C", "K_C");
	g_KeyboardRefMap.Add("V", "K_V");
	g_KeyboardRefMap.Add("B", "K_B");
	g_KeyboardRefMap.Add("N", "K_N");
	g_KeyboardRefMap.Add("M", "K_M");
	g_KeyboardRefMap.Add(",", "K_,");
	g_KeyboardRefMap.Add(".", "K_.");
	g_KeyboardRefMap.Add("/", "K_FORWARDSLASH");
	g_KeyboardRefMap.Add("RSHIFT", "K_RSHIFT");
	g_KeyboardRefMap.Add("*", "K_MULTIPLY");
	g_KeyboardRefMap.Add(" ", "K_SPACE");
	g_KeyboardRefMap.Add("CAPS", "K_CAPS");
}



SyncBool Platform::CreateVirtualDevice(TRefRef InstanceRef, TRefRef DeviceTypeRef)
{	
	// Check to see if the virtual device alreay exists
	TPtr<TInputDevice>& pDevice = g_pInputSystem->GetInstance(InstanceRef, FALSE);
	
	if(!pDevice)
		IPod::CreateDevice(InstanceRef, DeviceTypeRef, TRUE);
	
	return SyncTrue;
}

SyncBool Platform::RemoveVirtualDevice(TRefRef InstanceRef)
{
	// Remove the virtual device
	//TODO: Call a removedevice routine that also broadcasts a message to say the device has been removed
	
	//TEST - Remove the keyboard object.  Need a nice way to tie this to the destruction of the virtual device object
	IPod::DestroyVirtualKeyboard();
	
	g_pInputSystem->RemoveInstance(InstanceRef);
}




// Create Ipod input device
Bool Platform::IPod::CreateDevice(TRefRef InstanceRef, TRefRef DeviceTypeRef, Bool bVirtual)
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
Bool Platform::IPod::InitialiseDevice(TPtr<TInputDevice> pDevice, TRefRef DeviceTypeRef, Bool bVirtual)
{
	// Intitialise a virtual device?
	if(bVirtual)
		return InitialiseVirtualDevice(pDevice, DeviceTypeRef);
	
	// Initialise the physical device
	return InitialisePhysicalDevice(pDevice, DeviceTypeRef);
}


Bool Platform::IPod::InitialiseVirtualDevice(TPtr<TInputDevice> pDevice, TRefRef DeviceTypeRef)
{
	if(DeviceTypeRef == TLInput::KeyboardRef)
	{
		if(CreateVirtualKeyboard())
		{
			// Now add a sensor for each keyboard button we can process
			for(u32 uIndex = 0; uIndex < g_KeyboardRefMap.GetSize(); uIndex++)
			{				
				const TLKeyArray::TPair<TRef, TRef>& Pair= g_KeyboardRefMap.GetPairAt(uIndex);
				TRef KeyRef = Pair.m_Key;
				
				TPtr<TInputSensor>& pSensor = pDevice->AttachSensor(KeyRef, Button);
				
				if(pSensor.IsValid())
				{
					// Add the default button ref 'BNxxx'
					TRef refLabel = GetDefaultButtonRef(uIndex);
					pSensor->AddLabel(refLabel);

					// Add the ID ref which for the virtual keyboard is the letter of the key because the letter is what is passed to us via the obj-c callbacks
					refLabel = Pair.m_Item;
					pSensor->AddLabel(refLabel);
					
				}
				
				
			}
			
			return TRUE;
		}
	}
	
	return FALSE;
}

Bool Platform::IPod::InitialisePhysicalDevice(TPtr<TInputDevice> pDevice, TRefRef DeviceTypeRef)
{
	// Create four 'buttons' and 'axis' sensors to be able to send data from
	// and for actions to be mapped to
	
	// Add button inputs
	u32 uIndex = 0;
	
	u32 uUniqueID = 0;
	
	TString stringLabel;
	TRef refLabel;
	
	
	for(uIndex = 0; uIndex < IPod::MAX_CURSOR_POSITIONS; uIndex++)
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
	for(uIndex = 0; uIndex < IPod::MAX_CURSOR_POSITIONS; uIndex++)
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


SyncBool Platform::Update()		
{	

			
	return SyncTrue;	
}

SyncBool Platform::Shutdown()	
{	
	// Shutdown the mac hardware

	//Clear the touch data array
	IPod::g_TouchData.Empty(TRUE);
	IPod::g_AccelerationData.Empty(TRUE);
	
	IPod::g_ActiveTouchObjects.Empty(TRUE);
	IPod::g_TouchObjects.Empty(TRUE);
	
	// Cleanup if the keyboard is active
	IPod::DestroyVirtualKeyboard();
	
	IPod::g_KeyboardRefMap.Empty(TRUE);
	IPod::g_KeyboardKeyArray.Empty(TRUE);
	
	return SyncTrue;
}

SyncBool Platform::EnumerateDevices()
{ 
	TRef InstanceRef = "IPOD";

	// Check to see if the single ipod input device alreay exists
	TPtr<TInputDevice>& pDevice = g_pInputSystem->GetInstance(InstanceRef, FALSE);
	
	if(!pDevice)
		IPod::CreateDevice(InstanceRef, /*TLInput::TrackpadRef*/ TLInput::MouseRef, FALSE);
	
	return SyncTrue; 
}

void Platform::RemoveAllDevices() 
{
}



Bool Platform::UpdateDevice(TLInput::TInputDevice& Device)
{
	//TODO: Add proper device type checking for virtual and physical devices
	if(Device.GetDeviceType() == /*TLInput::TrackpadRef*/ TLInput::MouseRef)
	{
		// Physical device
		return IPod::UpdatePhysicalDevice(Device);
	}
	else
	{
		// Virtual device
		return IPod::UpdateVirtualDevice(Device);
	}	
	
	return FALSE;
}


Bool Platform::IPod::UpdatePhysicalDevice(TLInput::TInputDevice& Device)
{
#ifdef ENABLE_INPUTSYSTEM_TRACE
	TLDebug_Print("INPUT: Begin update");
#endif
		
#ifdef ENABLE_INPUTSYSTEM_TRACE
	TLDebug_Print("INPUT: End update");
#endif			
	
	return TRUE;
}



int2 Platform::GetCursorPosition(u8 uIndex)
{
	//TODO: Return cursor pos
	
	return int2(0,0);
}


// On Ipod set the cursor position
void Platform::Mac::SetCursorPosition(u8 uIndex, int2 uPos)
{
	//TODO: Set cursor pos
}

void TLInput::Platform::IPod::ProcessVirtualKey(TRefRef KeyRef)
{
	//TODO: Need to be able to handle other virtual device inputs
	// Add the key to the keyboard array
	IPod::g_KeyboardKeyArray.Add(KeyRef);
}

// TEMP test routine
void TLInput::Platform::TestVibrateDevice()
{
	IPod::VibrateDevice();
}

