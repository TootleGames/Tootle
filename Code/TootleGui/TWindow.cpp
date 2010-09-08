/*
 *  TWindow.cpp
 *  TootleGui
 *
 *  Created by Graham Reeves on 14/01/2010.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#include "TWindow.h"
#include <TootleInput/TDevice.h>
#include <TootleInput/TLInput.h>


//------------------------------------------------------
//	base window type for an OS window.
//------------------------------------------------------
TLGui::TWindow::TWindow(TRefRef WindowRef) :
	m_WindowRef	( WindowRef )
{
	CreateInputDevices();
}


//------------------------------------------------------
//
//------------------------------------------------------
void TLGui::TWindow::OnSizeChanged() const
{
	//	gr: still relevant? the screen's get their size from the canvas' dimensions
	//	when [client] size of the window changes, broadcast a message in order to update the screens
	TLDebug_Break("Todo");
}



//------------------------------------------------------
//	if Down=Wait then the input is a mouse move
//------------------------------------------------------
bool TLGui::TWindow::OnMouseInput(u32 ButtonIndex,const Type2<u16>& Position,SyncBool Down)
{
	//	push mouse input onto our mouse device
	TLInput::TInputDevice* pMouse = TLInput::GetDevice( m_Mouse );
	if ( !pMouse )
		return false;

	//	update last mouse pos. used for deprecated mouse-cursor pos access in user code... remove!
	m_LastMousePos = Position;

	//	get sensor that was triggered
	TRef ButtonSensor = TLInput::GetDefaultButtonRef( ButtonIndex );
	if ( !ButtonSensor.IsValid() )
		return false;

	TLInput::TInputData InputData;
	Type2<u16> WindowSize = GetSize();

	//	push mouse pos 
	InputData.m_SensorRef = TLInput::GetDefaultAxisRef( ButtonIndex, 'x' );
	InputData.m_fData = (float)Position.x / (float)WindowSize.x;
	pMouse->PushInputData( InputData );

	InputData.m_SensorRef = TLInput::GetDefaultAxisRef( ButtonIndex, 'y' );
	InputData.m_fData = (float)Position.y / (float)WindowSize.y;
	pMouse->PushInputData( InputData );

	//	push new button data if the button state changed
	if ( Down != SyncWait )
	{
		InputData.m_SensorRef = ButtonSensor;
		InputData.m_fData = (Down==SyncTrue) ? 1.f : 0.f;
		pMouse->PushInputData( InputData );
	}

	return true;
}


//------------------------------------------------------
//	key input
//------------------------------------------------------
bool TLGui::TWindow::OnKeyInput(TRef KeySensor,bool Down)
{
	TLDebug_Break("Todo");
	return false;
}



//-------------------------------------------------------
//	create window-orientated input devices
//-------------------------------------------------------
bool TLGui::TWindow::CreateInputDevices()
{
	//	create a mouse device to push mouse input to
	//	no input manager, probably not included in the app, so no need to create devices!
	if ( !TLInput::g_pInputSystem )
		return false;
	
	//	create mouse
	TLInput::TInputDevice* pMouse = TLInput::g_pInputSystem->CreateDevice( "Mouse", "Mouse" );
	if ( !pMouse )
		return false;
	
	m_Mouse = pMouse->GetDeviceRef();

	//	add axis' and L/R/M buttons
	pMouse->AttachSensor( TLInput::GetDefaultButtonRef(0), TLInput::Button );
	pMouse->AttachSensor( TLInput::GetDefaultAxisRef(0,'x'), TLInput::Axis );
	pMouse->AttachSensor( TLInput::GetDefaultAxisRef(0,'y'), TLInput::Axis );
	pMouse->AttachSensor( TLInput::GetDefaultButtonRef(1), TLInput::Button );
	pMouse->AttachSensor( TLInput::GetDefaultAxisRef(1,'x'), TLInput::Axis );
	pMouse->AttachSensor( TLInput::GetDefaultAxisRef(1,'y'), TLInput::Axis );
	pMouse->AttachSensor( TLInput::GetDefaultButtonRef(2), TLInput::Button );
	pMouse->AttachSensor( TLInput::GetDefaultAxisRef(2,'x'), TLInput::Axis );
	pMouse->AttachSensor( TLInput::GetDefaultAxisRef(2,'y'), TLInput::Axis );
	
	TLInput::g_pInputSystem->OnDeviceCreated( *pMouse );

	return true;
}

