
#include "TLInput.h"
#include <TootleCore/TEventChannel.h>

namespace TLInput
{
	TPtr<TInputManager>	g_pInputSystem = NULL;	// The input system

	const float INPUT_DEVICE_CHECK_TIME = 3.0f;			// Device check interval time in seconds
}

using namespace TLInput;


TInputManager::TInputManager(TRef refManagerID) :
	TManager(refManagerID),
	m_fDeviceCheckTimer(0.0f)
{
}


SyncBool TInputManager::Initialise()
{
	// If we have the event channel manager then register some event channels
	if(TLMessaging::g_pEventChannelManager)
	{
		TLMessaging::g_pEventChannelManager->RegisterEventChannel(this, GetManagerID(), "ACTION");
		TLMessaging::g_pEventChannelManager->RegisterEventChannel(this, GetManagerID(), "DEVICE");

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
		TPtr<TInputDevice> pDevice = ElementAtConst(uIndex);

		// Update the hardware device - data from buttons etc and information about device itself  
		if(TLInput::Platform::UpdateDevice(pDevice))
		{
			// No update the generic device information - process the sensors etc
			pDevice->Update();
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
		TPtr<TLInput::TInputDevice> pDevice = ElementAt(uIndex);

		// Send a message to say the device is being removed
		TPtr<TLMessaging::TMessage> pMessage = new TLMessaging::TMessage("Input");

		if(pMessage.IsValid())
		{
			pMessage->AddChannelID("DEVICE");								// device information message
			pMessage->AddChildAndData("STATE", "REMOVED");			// state change
			pMessage->AddChildAndData("DEVID", pDevice->GetDeviceID());		// device ID
			pMessage->AddChildAndData("TYPE", "UNKNOWN");				// device type

			PublishMessage(pMessage);
		}
	}

	Empty();

	Platform::RemoveAllDevices();
}



TInputDevice* TInputManager::CreateObject(TRefRef InstanceRef,TRefRef TypeRef)
{
	// Create the generic device object
	return new TInputDevice(InstanceRef);
}



void TInputManager::ProcessMessage(TPtr<TLMessaging::TMessage>& pMessage)
{
	if(pMessage->GetMessageRef() == "SCREEN")
	{
		TRef change;
		TRef test = "NEW";

		if(pMessage->Read(change))
		{
			if(change == test)
				RemoveAllDevices();
		}

		// No need to pass this sort of message on...
		return;
	}

	// Relay message to all subscribers
	PublishMessage(pMessage);

	// Process base manager messages
	TManager::ProcessMessage(pMessage);
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
		TPtr<TInputDevice> pDevice = ElementAt(uIndex);

		refArray.Add(pDevice->GetDeviceID());
	}

	return TRUE;
}



