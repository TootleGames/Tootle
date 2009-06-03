#include "TLInput.h"
#include <TootleCore/TEventChannel.h>
#include <TootleCore/TLTime.h>

#include <ctype.h> // for tolower routine

namespace TLInput
{
	TPtr<TInputManager>	g_pInputSystem = NULL;	// The input system

	const float INPUT_DEVICE_CHECK_TIME = 10.0f;			// Device check interval time in seconds
		
}

using namespace TLInput;



TRef TLInput::GetDefaultAxisRef(u32 uObjectIndex)
{
	u32 uLetterIndex = uObjectIndex%3;					// Get value of 0, 1 or 2 to represent x, y, or z
	u32 uAxisIndex = (uObjectIndex / 3);				// Axis index increments every 3rd one starting at 0, so AXX0, AXY0, AXZ0 etc

	if ( uLetterIndex == 0 )		return GetDefaultAxisRef( uAxisIndex, 'x' );
	else if ( uLetterIndex == 1 )	return GetDefaultAxisRef( uAxisIndex, 'y' );
	else							return GetDefaultAxisRef( uAxisIndex, 'z' );
}


TRef TLInput::GetDefaultAxisRef(u32 AxisIndex,const char AxisChar)
{
	if ( AxisIndex > 99 )
	{
		TLDebug_Break("Ref and explicit integer usage limitations mean we can't have more than 99 axis'. We CAN do AXXaa, AXXab, etc though for 41^2 axis' though");
		return TRef();
	}

//	TTempString stringLabel = "AXX";
//	stringLabel.Appendf("%02d",AxisIndex);
	//	gr: faster method, not sure if speed is an issue though?
	TFixedArray<char,5> RefStringChars(5);
	RefStringChars[0] = 'A';
	RefStringChars[1] = 'x';
	RefStringChars[2] = AxisChar;
	RefStringChars[3] = '0' + (AxisIndex / 10);
	RefStringChars[4] = '0' + (AxisIndex % 10);

	//	make AXX01 AXX1 etc
	if ( AxisIndex < 10 )
	{
		RefStringChars[3] = RefStringChars[4];
		RefStringChars.RemoveAt(4);
	}

	//	convert array of chars to ref
	TRef AxisRef;
	AxisRef.Set( RefStringChars );

	return AxisRef;
}

TRef TLInput::GetDefaultPOVRef(u32 uObjectIndex)
{
	TRef LabelRef;

	TString stringLabel = "POV";
	stringLabel.Appendf("%d", uObjectIndex);
	LabelRef = stringLabel;

	return LabelRef;
}

TRef TLInput::GetDefaultButtonRef(u32 uObjectIndex)
{
	TRef LabelRef;

	TString stringLabel = "BN";
	stringLabel.Appendf("%d", uObjectIndex);
	LabelRef = stringLabel;

	return LabelRef;
}


//---------------------------------------------
//	fetch device with this ref
//---------------------------------------------
TPtr<TLInput::TInputDevice>& TLInput::GetDevice(TRefRef DeviceRef)
{
	if ( !g_pInputSystem )
		return TLPtr::GetNullPtr<TLInput::TInputDevice>();

	//	get device
	TPtr<TLInput::TInputDevice>& pDevice = g_pInputSystem->GetInstance( DeviceRef );
	
	return pDevice;
}

//---------------------------------------------
//	find the (first) device of this type
//---------------------------------------------
TPtr<TLInput::TInputDevice>& TLInput::GetDeviceOfType(TRefRef DeviceType)
{
	if ( !g_pInputSystem )
		return TLPtr::GetNullPtr<TLInput::TInputDevice>();

	//	loop through devices until we find a matching one
	TPtrArray<TLInput::TInputDevice>& Devices = g_pInputSystem->GetInstanceArray();
	for ( u32 d=0;	d<Devices.GetSize();	d++ )
	{
		TPtr<TLInput::TInputDevice>& pDevice = Devices[d];
		if ( pDevice->GetDeviceType() == DeviceType )
			return pDevice;
	}
	
	return TLPtr::GetNullPtr<TLInput::TInputDevice>();
}


//---------------------------------------------
//	get an unused ref for a device
//---------------------------------------------
TRef TLInput::GetFreeDeviceRef(TRef BaseRef)
{
	if ( !g_pInputSystem )
		return TRef();

	if ( !BaseRef.IsValid() )
		BaseRef.Increment();

	while ( TRUE )
	{
		//	get device with this ref
		TPtr<TLInput::TInputDevice>& pDevice = g_pInputSystem->GetInstance( BaseRef );
		if ( !pDevice )
			break;

		//	try next
		BaseRef.Increment();
	}

	return BaseRef;
}



TInputManager::TInputManager(TRef refManagerID) :
	TManager(refManagerID),
	m_fDeviceCheckTimer(0.0f),
	m_bEnabled(TRUE) 
{
}


SyncBool TInputManager::Initialise()
{
	// If we have the event channel manager then register some event channels
	if(TLMessaging::g_pEventChannelManager)
	{
		TLMessaging::g_pEventChannelManager->RegisterEventChannel(this, GetManagerRef(), TRef_Static(A,c,t,i,o));
		TLMessaging::g_pEventChannelManager->RegisterEventChannel(this, GetManagerRef(), "DeviceChanged");

		InitSupportedTextCharacters();
		
		return Platform::Init();
	}

	return SyncWait;
}

/*
 For now use an array of all possible supported text characters from KEYBOARD input. At some stage this may need to be a language specific list (for special characters)
 or even a list generated via an xml file or possibly using key ranges instead.  Also, not sure whether we should use unicode here or UTF8 or something?
*/
void TInputManager::InitSupportedTextCharacters()
{
	m_SupportedTextCharacters.Add("k_a",'a');
	m_SupportedTextCharacters.Add("k_b",'b');
	m_SupportedTextCharacters.Add("k_c",'c');
	m_SupportedTextCharacters.Add("k_d",'d');
	m_SupportedTextCharacters.Add("k_e",'e');
	m_SupportedTextCharacters.Add("k_f",'f');
	m_SupportedTextCharacters.Add("k_g",'g');
	m_SupportedTextCharacters.Add("k_h",'h');
	m_SupportedTextCharacters.Add("k_i",'i');
	m_SupportedTextCharacters.Add("k_j",'j');
	m_SupportedTextCharacters.Add("k_k",'k');
	m_SupportedTextCharacters.Add("k_l",'l');
	m_SupportedTextCharacters.Add("k_m",'m');
	m_SupportedTextCharacters.Add("k_n",'n');
	m_SupportedTextCharacters.Add("k_o",'o');
	m_SupportedTextCharacters.Add("k_p",'p');
	m_SupportedTextCharacters.Add("k_q",'q');
	m_SupportedTextCharacters.Add("k_r",'r');
	m_SupportedTextCharacters.Add("k_s",'s');
	m_SupportedTextCharacters.Add("k_t",'t');
	m_SupportedTextCharacters.Add("k_u",'u');
	m_SupportedTextCharacters.Add("k_v",'v');
	m_SupportedTextCharacters.Add("k_w",'w');
	m_SupportedTextCharacters.Add("k_x",'x');
	m_SupportedTextCharacters.Add("k_y",'y');
	m_SupportedTextCharacters.Add("k_z",'z');
	m_SupportedTextCharacters.Add("k_0",'0');
	m_SupportedTextCharacters.Add("k_1",'1');
	m_SupportedTextCharacters.Add("k_2",'2');
	m_SupportedTextCharacters.Add("k_3",'3');
	m_SupportedTextCharacters.Add("k_4",'4');
	m_SupportedTextCharacters.Add("k_5",'5');
	m_SupportedTextCharacters.Add("k_6",'6');
	m_SupportedTextCharacters.Add("k_7",'7');
	m_SupportedTextCharacters.Add("k_8",'8');
	m_SupportedTextCharacters.Add("k_9",'9');	
	m_SupportedTextCharacters.Add("k_space",' ');
}

Bool TInputManager::BuildArrayOfSupportInputCharacterRefs(TArray<TRef>& array)
{
	u32 uSize = m_SupportedTextCharacters.GetSize();
	
	if(0 == uSize)
		return FALSE;

	// preallocate array size
	array.SetAllocSize(uSize);
	
	for(u32 uIndex = 0; uIndex < uSize; uIndex++)
	{
		TRef KeyRef = m_SupportedTextCharacters.GetKeyAt(uIndex);
		array.Add(KeyRef);
	}
	
	return TRUE;
}


Bool TInputManager::IsSupportedInputCharacter(const char& character)
{
	// Convert to lower case character
	const char lowercharacter = tolower(character);
	
	// Find the character in our supported characters lookup-table (language specific?)
	const TRef* pKey = m_SupportedTextCharacters.FindKey(lowercharacter);
	
	// Character found?  If not then it's not supported.
	if(NULL == pKey)
		return FALSE;

	// Character supported
	return TRUE;
}

Bool TInputManager::GetSupportedInputCharacter(TRefRef CharacterRef, char& character)
{
	char* pChar = m_SupportedTextCharacters.Find(CharacterRef);

	// Not found
	if(!pChar)
		return FALSE;

	// set the character
	character = *pChar;

	// return success
	return TRUE;
}

Bool TInputManager::GetSupportedInputCharacterRef(TRef& CharacterRef, const char& character)
{
	const char lowercharacter = tolower(character);

	const TRef* pRef = m_SupportedTextCharacters.FindKey(lowercharacter);
	
	// Not found
	if(!pRef)
		return FALSE;
	
	// set the character ref
	CharacterRef = *pRef;
	
	// return success
	return TRUE;
}



void TInputManager::OnEventChannelAdded(TRefRef refPublisherID, TRefRef refChannelID)
{
	if(refPublisherID == "CORE")
	{
		// Subscribe to the update messages
		if(refChannelID == TLCore::UpdateRef)
			TLMessaging::g_pEventChannelManager->SubscribeTo(this, refPublisherID, refChannelID); 
	}
	else if(refPublisherID == "SCREENMANAGER")
	{
		// Subscribe to the update messages
		if((refChannelID == "OnWindowChanged") ||
			(refChannelID == "ScreenChanged"))
		{
			TLMessaging::g_pEventChannelManager->SubscribeTo(this, refPublisherID, refChannelID);
		}
	}

	
	// Super event channel routine
	TManager::OnEventChannelAdded(refPublisherID, refChannelID);
}


SyncBool TInputManager::Update(float fTimeStep)
{
	if(!IsEnabled())
		return SyncTrue;

	TLTime::TScopeTimer Timer( TRef_Static(I,n,p,u,t) );

	// Need to test for new devices and initialise them as required
	m_fDeviceCheckTimer -= fTimeStep;

	if(m_fDeviceCheckTimer < 0.0f)
	{
		CheckForDeviceChanges();
		m_fDeviceCheckTimer = INPUT_DEVICE_CHECK_TIME;
	}

	// Update device info
	u32 uNumberOfDevices = GetSize();

	for(u32 uIndex = 0; uIndex < uNumberOfDevices; uIndex++)
	{
		TInputDevice& Device = *ElementAt(uIndex);

		// Update the hardware device - data from buttons etc and information about device itself  
		if ( TLInput::Platform::UpdateDevice( Device ) )
		{
			// No update the generic device information - process the sensors etc
			Device.Update();
		}
	}

	return Platform::Update();
}


void TInputManager::CheckForDeviceChanges()
{
	// OK we need to get the platform specific stuff to check for any changes to the device states
	// i.e. new devices that have been attached and current devices that have been dettached
	Platform::EnumerateDevices();
	
	// Now check for changes to virtual devices
	if(m_VirtualDeviceRequests.GetSize())
	{
		for(u32 uIndex = 0; uIndex < m_VirtualDeviceRequests.GetSize(); uIndex++)
		{
			TVirtualDeviceRequest& request = m_VirtualDeviceRequests.ElementAt(uIndex);
			
			// Process create or remove request
			if(request.m_bCreate)
			{
				if(!TLInput::Platform::CreateVirtualDevice(request.m_DeviceInstanceRef, request.m_DeviceTypeRef))
				{
					TLDebug_Break("Failed to create virtual device");
				}
			}
			else
			{
				TPtr<TLInput::TInputDevice> pDevice = GetInstance(request.m_DeviceInstanceRef);

				if(pDevice)
				{
					// Send a message to say the device is being removed
					TLMessaging::TMessage Message("DeviceChanged");
					Message.ExportData("State", TRef("Removed"));			// state change
					Message.ExportData("DEVID", pDevice->GetDeviceRef() );		// device ref 
					Message.ExportData("TYPE", pDevice->GetDeviceType() );				// device type
					
					PublishMessage(Message);
					
					if(!TLInput::Platform::RemoveVirtualDevice(request.m_DeviceInstanceRef))
					{
						TLDebug_Break("Failed to remove virtual device");
					}
				}
				else
				{
					TLDebug_Break("Failed to get virtual device instance for removal");
				}
			}
		}
		
		m_VirtualDeviceRequests.Empty(TRUE);
	}
}

Bool TInputManager::CreateVirtualDevice(TRefRef InstanceRef, TRefRef DeviceTypeRef)
{
	// Check to see if the device already exists.  If so return false.
	
	TPtr<TLInput::TInputDevice> pDevice = GetInstance(InstanceRef);

	if(pDevice)
	{
		TLDebug_Print("Virtual device already exists");
		return FALSE;
	}
	
	// TODO: Check to see if the device has already been requested.
	// If so return false.
	
	// Add request to queue
	TVirtualDeviceRequest request;
	
	request.m_DeviceInstanceRef = InstanceRef;
	request.m_DeviceTypeRef = DeviceTypeRef;
	request.m_bCreate = TRUE;
	
	m_VirtualDeviceRequests.Add(request);

	// Set timer to force an update of the devices list
	m_fDeviceCheckTimer = 0.0f;

	//return TLInput::Platform::CreateVirtualDevice(InstanceRef, DeviceTypeRef);
	return TRUE;
}

Bool TInputManager::RemoveVirtualDevice(TRefRef InstanceRef)
{
	// TODO: Check device exists and return false if not.
	TPtr<TLInput::TInputDevice> pDevice = GetInstance(InstanceRef);
	
	if(!pDevice)
	{
		TLDebug_Print("Virtual device does not exist");
		return FALSE;
	}
	
	// TODO: Check to see if the removal/creation has already been requested
	
	// Add request to queue
	TVirtualDeviceRequest request;
	
	request.m_DeviceInstanceRef = InstanceRef;
	request.m_bCreate = FALSE;
	
	m_VirtualDeviceRequests.Add(request);
			
	// Set timer to force an update of the devices list
	m_fDeviceCheckTimer = 0.0f;
	
	//return TLInput::Platform::RemoveVirtualDevice(InstanceRef);
	return TRUE;
}



SyncBool TInputManager::Shutdown()
{
	RemoveAllDevices();

	return Platform::Shutdown();
}

void TInputManager::RemoveAllDevices()
{
	for(u32 uIndex = 0; uIndex <  GetSize(); uIndex++)
	{
		TLInput::TInputDevice& Device = *ElementAt(uIndex);

		// Send a message to say the device is being removed
		TLMessaging::TMessage Message("DeviceChanged");
		Message.ExportData("State", TRef("Removed"));			// state change
		Message.ExportData("DEVID", Device.GetDeviceRef() );		// device ref 
		Message.ExportData("TYPE", Device.GetDeviceType() );				// device type

		PublishMessage(Message);
	}

	Empty();

	Platform::RemoveAllDevices();
}



TInputDevice* TInputManager::CreateObject(TRefRef InstanceRef,TRefRef TypeRef)
{
	// Create the generic device object
	return new TInputDevice(InstanceRef,TypeRef);
}



void TInputManager::ProcessMessage(TLMessaging::TMessage& Message)
{
	TRef MessageRef = Message.GetMessageRef();

	if(MessageRef == "ScreenChanged")
	{
		TRef State;

		if(Message.ImportData("State", State))
		{
			if(State == "Added")
				RemoveAllDevices();
		}

		// No need to pass this sort of message on...
		return;
	}
	else if(MessageRef == "OnWindowChanged")
	{
		TRef State;

		if(Message.ImportData("State", State))
		{
			if(State == "Deactivate")
				m_bEnabled = FALSE;
			else
				m_bEnabled = TRUE;

			return;
		}
	}

	// Relay message to all subscribers
	PublishMessage(Message);

	// Process base manager messages
	TManager::ProcessMessage(Message);
}



/*
	Fills an array with the device ID's currently available
*/
Bool TInputManager::GetDeviceIDs(TArray<TRef>& refArray)
{
	if(GetSize() == 0)
		return FALSE;

	for(u32 uIndex = 0; uIndex < GetSize(); uIndex++)
	{
		TInputDevice& Device = *ElementAt(uIndex);

		refArray.Add( Device.GetDeviceRef() );
	}

	return TRUE;
}

// TEMP test routine.  Vibration will eventually be handled via the device effects system
void TLInput::TempTriggerVibration()
{
	Platform::TestVibrateDevice();
}




