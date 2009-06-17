
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

			if(refState == STRef(A,D,D,E,D))
			{
				// New device
				OnInputDeviceAdded( DeviceRef, DeviceTypeRef );
			}
			else if(refState == STRef(R,E,M,O,V))
			{
				// Device removed
				OnInputDeviceRemoved(DeviceRef, DeviceTypeRef);
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

	if(DeviceTypeRef == STRef(k,e,y,b,o))
	{
		MapDeviceActions_Keyboard(DeviceRef, pGlobalUser);
	}
	else if(DeviceTypeRef == STRef(M,O,U,S,E))
	{

#ifdef TL_TARGET_IPOD
		MapDeviceActions_TouchPad(DeviceRef, pGlobalUser);
#else
		MapDeviceActions_Mouse(DeviceRef, pGlobalUser);
#endif
	}
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

	
#ifdef TEST_WIDGET_ONE_ACTION	
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
#endif //TEST_WIDGET_ONE_ACTION
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


void TWidgetManager::MapDeviceActions_Keyboard(TRefRef DeviceRef, TPtr<TLUser::TUser> pUser)
{
	/*
	// Subscribe to the keyboard buttons
	// Get the device.  This should *always* be valid if the ID is valid.
	TPtr<TLInput::TInputDevice> pDevice = TLInput::g_pInputSystem->FindPtr(DeviceRef);
	
	if(!pDevice)
	{
		TLDebug_Break("Invalid device ref");
		return;
	}
	
	///////////////////////////////////////////////////////
	// Needs moving into the device class so we can subscribe 
	// something to all sensors of a specific class
	///////////////////////////////////////////////////////
	u32 NumberOfButtons = pDevice->GetSensorCount(TLInput::Button);
	
	for(u32 uIndex = 0; uIndex < NumberOfButtons; uIndex++)
	{
		TRef ButtonRef = TLInput::GetDefaultButtonRef(uIndex);

		TPtr<TLInput::TInputSensor>& pSensor = pDevice->GetSensor(ButtonRef);
		
		if(pSensor.IsValid())
			SubscribeTo(pSensor);
	}
	///////////////////////////////////////////////////////
	*/
			
	// Get a list of the supported characters 
	TArray<TRef> RefArray;
	
	if(TLInput::g_pInputSystem->BuildArrayOfSupportInputCharacterRefs(RefArray))
	{
		for(u32 uIndex = 0; uIndex < RefArray.GetSize(); uIndex++)
		{
			TRef KeyRef = RefArray.ElementAt(uIndex);
			
			// Map actions for each key of the keyboard we will support use of
			if(pUser->AddAction("SIMPLE", KeyRef))
			{
				// The keyref is the same as one of the sensor labels so we can use the same ref. 
				pUser->MapAction(KeyRef, DeviceRef, KeyRef);
				pUser->MapActionCondition(KeyRef, TLInput::LessThan, 1.0f); // Release
			}
		}			
	}
	/*
	 // VERY SPECIAL CHARACTERS. PROBABLY NEED A LOOKUP FOR THE CHARS FOR THESE TO WORK CORRECTLY
	pGlobalUser->AddAction("SIMPLE", "-");
	pGlobalUser->MapAction("-", DeviceRef, "k_minus");
	pGlobalUser->MapActionCondition("-", TLInput::LessThan, 1.0f); // Release
	
	pGlobalUser->AddAction("SIMPLE", "/");
	pGlobalUser->MapAction("/", DeviceRef, "k_forwardslash");
	pGlobalUser->MapActionCondition("/", TLInput::LessThan, 1.0f); // Release
	
	pGlobalUser->AddAction("SIMPLE", ":");
	pGlobalUser->MapAction(":", DeviceRef, "k_colon");
	pGlobalUser->MapActionCondition(":", TLInput::LessThan, 1.0f); // Release
	
	pGlobalUser->AddAction("SIMPLE", ";");
	pGlobalUser->MapAction(";", DeviceRef, "k_semicolon");
	pGlobalUser->MapActionCondition(";", TLInput::LessThan, 1.0f); // Release
	
	pGlobalUser->AddAction("SIMPLE", "(");
	pGlobalUser->MapAction("(", DeviceRef, "k_lbracket");
	pGlobalUser->MapActionCondition("(", TLInput::LessThan, 1.0f); // Release
	
	pGlobalUser->AddAction("SIMPLE", ")");
	pGlobalUser->MapAction(")", DeviceRef, "k_rbracket");
	pGlobalUser->MapActionCondition(")", TLInput::LessThan, 1.0f); // Release
	
	pGlobalUser->AddAction("SIMPLE", "£");
	pGlobalUser->MapAction("£", DeviceRef, "k_sterling");
	pGlobalUser->MapActionCondition("£", TLInput::LessThan, 1.0f); // Release
	
	pGlobalUser->AddAction("SIMPLE", "&");
	pGlobalUser->MapAction("&", DeviceRef, "k_ampersand");
	pGlobalUser->MapActionCondition("&", TLInput::LessThan, 1.0f); // Release
	
	pGlobalUser->AddAction("SIMPLE", "@");
	pGlobalUser->MapAction("@", DeviceRef, "k_at");
	pGlobalUser->MapActionCondition("@", TLInput::LessThan, 1.0f); // Release
	
	pGlobalUser->AddAction("SIMPLE", "\"");
	pGlobalUser->MapAction("\"", DeviceRef, "k_quote");
	pGlobalUser->MapActionCondition("\"", TLInput::LessThan, 1.0f); // Release
	
	pGlobalUser->AddAction("SIMPLE", ".");
	pGlobalUser->MapAction(".", DeviceRef, "k_period");
	pGlobalUser->MapActionCondition(".", TLInput::LessThan, 1.0f); // Release
	
	pGlobalUser->AddAction("SIMPLE", ",");
	pGlobalUser->MapAction(",", DeviceRef, "k_comma");
	pGlobalUser->MapActionCondition(",", TLInput::LessThan, 1.0f); // Release
	
	pGlobalUser->AddAction("SIMPLE", "?");
	pGlobalUser->MapAction("?", DeviceRef, "k_question");
	pGlobalUser->MapActionCondition("?", TLInput::LessThan, 1.0f); // Release
	
	pGlobalUser->AddAction("SIMPLE", "!");
	pGlobalUser->MapAction("!", DeviceRef, "k_exclamation");
	pGlobalUser->MapActionCondition("!", TLInput::LessThan, 1.0f); // Release
	
	pGlobalUser->AddAction("SIMPLE", "'");
	pGlobalUser->MapAction("'", DeviceRef, "k_apostrophe");
	pGlobalUser->MapActionCondition("'", TLInput::LessThan, 1.0f); // Release
	
	pGlobalUser->AddAction("SIMPLE", "+");
	pGlobalUser->MapAction("+", DeviceRef, "k_plus");
	pGlobalUser->MapActionCondition("+", TLInput::LessThan, 1.0f); // Release
	
	pGlobalUser->AddAction("SIMPLE", "*");
	pGlobalUser->MapAction("*", DeviceRef, "k_multiply");
	pGlobalUser->MapActionCondition("*", TLInput::LessThan, 1.0f); // Release
	
	pGlobalUser->AddAction("SIMPLE", "=");
	pGlobalUser->MapAction("=", DeviceRef, "k_equals");
	pGlobalUser->MapActionCondition("=", TLInput::LessThan, 1.0f); // Release
*/			
	
	// Special character actions
	if(pUser->AddAction("SIMPLE", "k_backspace"))
	{
		pUser->MapAction("k_backspace", DeviceRef, "k_backspace");
		pUser->MapActionCondition("k_backspace", TLInput::LessThan, 1.0f); // Release
	}

	if(pUser->AddAction("SIMPLE", "k_return"))
	{
		pUser->MapAction("k_return", DeviceRef, "k_return");
		pUser->MapActionCondition("k_return", TLInput::LessThan, 1.0f); // Release
	}
	///////////////////////////////////////////////////////
	
}


void TWidgetManager::OnInputDeviceRemoved(TRefRef DeviceRef, TRefRef DeviceTypeRef)
{
	// Remove any added actions from the global user.
	
	TPtr<TLUser::TUser>	pGlobalUser = TLUser::g_pUserManager->GetUser("Global");
	
	if(!pGlobalUser.IsValid())
	{
		TLDebug_Break("Failed to find global user");
		return;
	}
	
	if(DeviceTypeRef == STRef(k,e,y,b,o))
	{
		// Get a list of the supported characters 
		TArray<TRef> RefArray;
		
		if(TLInput::g_pInputSystem->BuildArrayOfSupportInputCharacterRefs(RefArray))
		{
			for(u32 uIndex = 0; uIndex < RefArray.GetSize(); uIndex++)
			{
				TRef KeyRef = RefArray.ElementAt(uIndex);
				
				// Map actions for each key of the keyboard we will support use of
				if(!pGlobalUser->RemoveAction(KeyRef))
				{
					TLDebug_Break("Action being removed from user that doesn't exist.");
				}
			}			
		}
		
		// Remove special actions
		if(!pGlobalUser->RemoveAction("k_backspace"))
		{
			TLDebug_Break("Action being removed from user that doesn't exist.");
		}
		
		if(!pGlobalUser->RemoveAction("k_return"))
		{
			TLDebug_Break("Action being removed from user that doesn't exist.");
		}
		
	}		
}
