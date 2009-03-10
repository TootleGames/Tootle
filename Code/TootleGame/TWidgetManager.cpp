
#include "TWidgetManager.h"

#include <TootleCore/TEventChannel.h>
#include <TootleInput/TLInput.h>


namespace TLGui
{
	TPtr<TWidgetManager>	g_pWidgetManager = NULL;
}


using namespace TLGui;


void TWidgetManager::OnEventChannelAdded(TRefRef refPublisherID, TRefRef refChannelID)
{
	if(refPublisherID == "INPUT")
	{
		// Subscribe to the input device messages
		if(refChannelID == "DeviceChanged")
			TLMessaging::g_pEventChannelManager->SubscribeTo(this, refPublisherID, refChannelID); 
		
	}
	
	// Super event channel routine
	TManager::OnEventChannelAdded(refPublisherID, refChannelID);
}

//-----------------------------------------------------------
//	process messages
//-----------------------------------------------------------
void TWidgetManager::ProcessMessage(TLMessaging::TMessage& Message)
{
	TRefRef MessageRef = Message.GetMessageRef();

	if(MessageRef == "DeviceChanged")
	{
		// Device message form the input system
		// Check for if the device has been added or removed
		TRef refState;
		if(Message.ImportData("State", refState))
		{
			TRef DeviceRef, DeviceTypeRef;
			if(!Message.ImportData("DEVID", DeviceRef))
				return;

			if(!Message.ImportData("TYPE", DeviceTypeRef))
				return;

			if(refState == TRef("ADDED"))
			{
				// New device
				OnInputDeviceAdded( DeviceRef, DeviceTypeRef );
			}
			else if(refState == TRef("REMOVED"))
			{
				// Device removed
				// Remove any users that are using the device		
			}

			// return - no need to pass this message on
			return;
		}
	}

	TManager::ProcessMessage(Message);
}

void TWidgetManager::OnInputDeviceAdded(TRefRef DeviceRef, TRefRef DeviceTypeRef)
{
	TPtr<TLUser::TUser>	pGlobalUser = TLUser::g_pUserManager->GetUser("Global");
	
	if(!pGlobalUser.IsValid())
	{
		TLDebug_Break("Failed to find global user");
		return;
	}

#ifdef TL_TARGET_IPOD
	MapDeviceActions_TouchPad(DeviceRef, pGlobalUser);
#else
	if(DeviceTypeRef == "MOUSE")
	{
		MapDeviceActions_Mouse(DeviceRef, pGlobalUser);
	}
#endif
}

#ifdef TL_TARGET_IPOD

void TWidgetManager::MapDeviceActions_TouchPad(TRefRef DeviceRef, TPtr<TLUser::TUser> pUser)
{
	// Get the device.  This should *always* be valid if the ID is valid.
	TPtr<TLInput::TInputDevice> pDevice = TLInput::g_pInputSystem->FindPtr(DeviceRef);

	if(!pDevice)
	{
		TLDebug_Break("Invalid device ref");
		return;
	}

	/////////////////////////////////////////////////////////////////////////
	// GUI SYSTEM ACTIONS
	/////////////////////////////////////////////////////////////////////////

	// BClick action.  This is a generic click/select action.  
	// Says a 'click' has started

	// Begin click
	if(pUser->AddAction("SIMPLE", "BClick"))	
	{
		//	subscribe to all of the button sensors
		u32 NumberOfButtons = pDevice->GetSensorCount(TLInput::Button);

		for ( u32 s=0; s < NumberOfButtons ;s++ )
		{
			TRef ButtonRef = TLInput::GetDefaultButtonRef(s);

			// Map to all buttons
			pUser->MapAction("BClick", DeviceRef, ButtonRef);
		}

		// Only when 'pressed' do we want to know about it
		pUser->MapActionCondition("BClick", TLInput::GreaterThan, 0.0f);
	}

	// EClick action.  This is a generic click/select action.  
	// Says a 'click' has ended

	// End click
	if(pUser->AddAction("SIMPLE", "EClick"))	
	{
		//	subscribe to all of the button sensors
		u32 NumberOfButtons = pDevice->GetSensorCount(TLInput::Button);

		for ( u32 s=0; s < NumberOfButtons ;s++ )
		{
			TRef ButtonRef = TLInput::GetDefaultButtonRef(s);

			// Map to all buttons
			pUser->MapAction("EClick", DeviceRef, ButtonRef);
		}

		// Only when 'released' do we want to know about it
		pUser->MapActionCondition("EClick", TLInput::LessThan, 1.0f);	
	}
}

#else

void TWidgetManager::MapDeviceActions_Mouse(TRefRef DeviceRef, TPtr<TLUser::TUser> pUser)
{
	// Get the device.  This should *always* be valid if the ID is valid.
	TPtr<TLInput::TInputDevice> pDevice = TLInput::g_pInputSystem->FindPtr(DeviceRef);

	if(!pDevice)
	{
		TLDebug_Break("Invalid device ref");
		return;
	}


	/////////////////////////////////////////////////////////////////////////
	// GUI SYSTEM ACTIONS
	/////////////////////////////////////////////////////////////////////////

	// BClick action.  This is a generic click/select action.  
	// Says a 'click' has started

	// Begin click
	if(pUser->AddAction("SIMPLE", "BClick"))	
	{
		//	subscribe to all of the button sensors
		u32 NumberOfButtons = pDevice->GetSensorCount(TLInput::Button);

		for ( u32 s=0; s < NumberOfButtons ;s++ )
		{
			TRef ButtonRef = TLInput::GetDefaultButtonRef(s);

			// Map to all buttons
			pUser->MapAction("BClick", DeviceRef, ButtonRef);
		}

		// Only when 'pressed' do we want to know about it
		pUser->MapActionCondition("BClick", TLInput::GreaterThan, 0.0f);
	}

	// EClick action.  This is a generic click/select action.  
	// Says a 'click' has ended

	// End click
	if(pUser->AddAction("SIMPLE", "EClick"))	
	{
		//	subscribe to all of the button sensors
		u32 NumberOfButtons = pDevice->GetSensorCount(TLInput::Button);

		for ( u32 s=0; s < NumberOfButtons ;s++ )
		{
			TRef ButtonRef = TLInput::GetDefaultButtonRef(s);

			// Map to all buttons
			pUser->MapAction("EClick", DeviceRef, ButtonRef);
		}

		// Only when 'released' do we want to know about it
		pUser->MapActionCondition("EClick", TLInput::LessThan, 1.0f);
	}


}


#endif