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
	if(TLInput::g_pInputSystem->CreateVirtualDevice("VDev1", "keyboard"))
	{
		TLInput::g_pInputSystem->SetVirtualKeyboardText(m_Text);
	}
	else
	{
		// If failed we can ignore because there is most likely a keyboard device already which will 
		// have the appropriate actions mapped
		TLDebug_Print("Failed to create virtual device");

	}
}

void TWidgetText::EndEditing()
{
	m_bEditing = FALSE;

	// For platforms that support it remove the virtual keyboard
	if(!TLInput::g_pInputSystem->RemoveVirtualDevice("VDev1"))
	{
		// If failed we can safely ignore as there will most likely be one permanent
		// keyboard that won;t change and hence will have the appropriate actions already mapped
		// as required
		TLDebug_Print("Failed to remove virtual device");
	}
}


void TWidgetText::ProcessMessage(TLMessaging::TMessage& Message)
{
	
	TRefRef MessageRef = Message.GetMessageRef();
	
	if(m_bEditing && (MessageRef == STRef(A,c,t,i,o)))
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

