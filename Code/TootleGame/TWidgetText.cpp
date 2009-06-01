/*
 *  TWidgetText.cpp
 *  TootleGame
 *
 *  Created by Duane Bradbury on 28/05/2009.
 *  Copyright 2009 Tootle. All rights reserved.
 *
 */

#include "TWidgetText.h"

#include <TootleInput/TLInput.h>
#include <TootleInput/TUser.h>
#include <TootleCore/TEventChannel.h>

using namespace TLGui;


TWidgetText::TWidgetText(TRefRef RenderTargetRef,TRefRef RenderNodeRef,TRefRef UserRef,TRefRef ActionOutDown,TRefRef ActionOutUp) : 
	TLInput::TInputInterface( RenderTargetRef, RenderNodeRef, UserRef, ActionOutDown, ActionOutUp ),
	m_bEditing(FALSE)
{
}

SyncBool TWidgetText::Initialise()
{
	SyncBool Result = TInputInterface::Initialise();
	
	if(Result == SyncTrue)
	{
		//TODO: Subscribe to the input event channel so we get notified when devices are created/removed
		TLMessaging::g_pEventChannelManager->SubscribeTo(this, "INPUT", "DeviceChanged");
	}
	
	return Result;
}

void TWidgetText::Shutdown()
{
	TInputInterface::Shutdown();
}


void TWidgetText::OnClickEnd(const TClick& Click)
{
	if(m_bEditing)
		EndEditing();
	else
		BeginEditing();
}


void TWidgetText::BeginEditing()
{
	m_bEditing = TRUE;

	// For platforms that support it bring up the virtual keyboard
	if(!TLInput::g_pInputSystem->CreateVirtualDevice("VDev1", "keyboard"))
	{
		// If failed then get hold of the physical keyboard device
		// and subscribe to it's buttons
		
	}
}

void TWidgetText::EndEditing()
{
	m_bEditing = FALSE;

	// For platforms that support it remove the virtual keyboard
	if(!TLInput::g_pInputSystem->RemoveVirtualDevice("VDev1"))
	{
		// If failed get hold of the physical keyboard device
		// and unsubscribe from the buttons		
	}
}


void TWidgetText::ProcessMessage(TLMessaging::TMessage& Message)
{
	
	TRefRef MessageRef = Message.GetMessageRef();
	
	if(MessageRef == STRef(D,e,v,i,c))
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
	else if(MessageRef == STRef(A,c,t,i,o))
	{
		// Action occured - check for a keyboard intput
		TRef ActionRef;
		Message.Read(ActionRef);
		
		char character;
		
		if(TLInput::g_pInputSystem->GetSupportedInputCharacter(ActionRef, character))
		{
		   m_Text.Append(character);
		   
		   OnTextChange();
			return;
		}
		else
		{
			// failed to find action ref as a supported text character
			// Try the special types such as backspace and return
			if(ActionRef == TRef("k_backspace"))
			{

				// Special case for backspace to delete a character
				s32 sIndex = m_Text.GetCharGetLastIndex();
				
				if(sIndex != -1)
				{
					m_Text.RemoveCharAt(sIndex, 1);
					
					OnTextChange();
				}
				return;
			}
			else if(ActionRef == TRef("k_return"))
			{
				// Special case for return
				EndEditing();
				return;
			}
		}
		   
		/*
		
		// Now check some chars we can't put into static tref's, like numbers and space
		static TFixedArray<char, 11> refarray;
		refarray.Add('0');
		refarray.Add('1');
		refarray.Add('2');
		refarray.Add('3');
		refarray.Add('4');
		refarray.Add('5');
		refarray.Add('6');
		refarray.Add('7');
		refarray.Add('8');
		refarray.Add('9');
		refarray.Add(' ');
		
		for(u32 uIndex = 0; uIndex < refarray.GetSize(); uIndex++)
		{
			TString str;
			ActionRef.GetString(str);
			
			if(refarray.ElementAt(uIndex) == str.GetCharAt(0))
			{
				// Use the ref as a string - only ok if we have actions 'a', 'b' 'c' etc for each keyboard key.  
				// This may change to 'k_a' etc at some point to match the sensor refs in which case we would 
				// need a character lookup 'k_a'->'a' for example but we can use one array for all platforms if so.
				
				m_Text.Append(str,1);
				
				OnTextChange();
				return;				
			}
		}
		 */

		// Message will filter through to super class for other action processing so reset the message read pos.
		Message.ResetReadPos();
	}
	
	
	// super class process message
	TInputInterface::ProcessMessage(Message);
}


void TWidgetText::OnTextChange()
{
	TLMessaging::TMessage Message("SetString");
	Message.ExportDataString("String", m_Text );
	PublishMessage( Message );	
}

void TWidgetText::OnInputDeviceAdded(TRefRef DeviceRef, TRefRef DeviceTypeRef)
{
	TPtr<TLUser::TUser>	pGlobalUser = TLUser::g_pUserManager->GetUser("Global");
	
	if(!pGlobalUser.IsValid())
	{
		TLDebug_Break("Failed to find global user");
		return;
	}
	
	if(DeviceTypeRef == STRef(k,e,y,b,o))
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
				if(pGlobalUser->AddAction("SIMPLE", KeyRef))
				{
					// The keyref is the same as one of the sensor labels so we can use the same ref. 
					pGlobalUser->MapAction(KeyRef, DeviceRef, KeyRef);
					pGlobalUser->MapActionCondition(KeyRef, TLInput::LessThan, 1.0f); // Release
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
		if(pGlobalUser->AddAction("SIMPLE", "k_backspace"))
		{
			pGlobalUser->MapAction("k_backspace", DeviceRef, "k_backspace");
			pGlobalUser->MapActionCondition("k_backspace", TLInput::LessThan, 1.0f); // Release
		}

		if(pGlobalUser->AddAction("SIMPLE", "k_return"))
		{
			pGlobalUser->MapAction("k_return", DeviceRef, "k_return");
			pGlobalUser->MapActionCondition("k_return", TLInput::LessThan, 1.0f); // Release
		}
		///////////////////////////////////////////////////////
		
	}
}

void TWidgetText::OnInputDeviceRemoved(TRefRef DeviceRef, TRefRef DeviceTypeRef)
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
