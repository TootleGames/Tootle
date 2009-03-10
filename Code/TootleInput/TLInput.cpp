
#include "TLInput.h"
#include <TootleCore/TEventChannel.h>

namespace TLInput
{
	TPtr<TInputManager>	g_pInputSystem = NULL;	// The input system

	const float INPUT_DEVICE_CHECK_TIME = 10.0f;			// Device check interval time in seconds
}

using namespace TLInput;



TRef TLInput::GetDefaultAxisRef(u32 uObjectIndex)
{
	TRef LabelRef;

	TString stringLabel = "AX";

	u32 uLetterIndex = uObjectIndex%3;					// Get value of 0, 1 or 2 to represent x, y, or z
	u32 uAxisIndex = (uObjectIndex / 3);				// Axis index increments every 3rd one starting at 0, so AXX0, AXY0, AXZ0 etc
	if(uLetterIndex == 0)
		stringLabel.Appendf("X%d", uAxisIndex);
	else if(uLetterIndex == 1)
		stringLabel.Appendf("Y%d", uAxisIndex);
	else if(uLetterIndex == 2)
		stringLabel.Appendf("Z%d", uAxisIndex);

	LabelRef = stringLabel;

	return LabelRef;
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
		TLMessaging::g_pEventChannelManager->RegisterEventChannel(this, GetManagerRef(), "Action");
		TLMessaging::g_pEventChannelManager->RegisterEventChannel(this, GetManagerRef(), "DeviceChanged");

		return Platform::Init();
	}

	return SyncWait;
}

void TInputManager::OnEventChannelAdded(TRefRef refPublisherID, TRefRef refChannelID)
{
	if(refPublisherID == "CORE")
	{
		// Subscribe to the update messages
		if(refChannelID == TLCore::UpdateRef)
			TLMessaging::g_pEventChannelManager->SubscribeTo(this, refPublisherID, refChannelID); 
	}
	
	// Super event channel routine
	TManager::OnEventChannelAdded(refPublisherID, refChannelID);
}


SyncBool TInputManager::Update(float fTimeStep)
{
	if(!IsEnabled())
		return SyncTrue;

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
		Message.AddChildAndData("State", "REMOVED");			// state change
		Message.AddChildAndData("DEVID", Device.GetDeviceRef() );		// device ref 
		Message.AddChildAndData("TYPE", Device.GetDeviceType() );				// device type

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



