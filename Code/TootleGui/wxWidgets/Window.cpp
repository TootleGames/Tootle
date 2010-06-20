/*
 *  TWxWindow.cpp
 *  TootleGui
 *
 *  Created by Graham Reeves on 14/01/2010.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */
#if !defined(TL_ENABLE_WX)
#error Should only be built in wx only build
#endif // TL_ENABLE_WX


#include "Window.h"
#include <TootleInput/TLInput.h>


//------------------------------------------------------
//	
//------------------------------------------------------
TPtr<TLGui::TWindow> TLGui::CreateGuiWindow(TRefRef Ref)
{
	TPtr<TLGui::TWindow> pWindow = new wx::Window( Ref );
	return pWindow;
}




wx::Window::Window(TRefRef WindowRef) :
	TLGui::TWindow		( WindowRef ),
	wxFrame				( NULL, GetID(WindowRef), GetString( WindowRef ) )
{	
	Bind( wxEVT_LEFT_DOWN, &wx::Window::OnMouseDown, this );
	Bind( wxEVT_RIGHT_DOWN, &wx::Window::OnMouseDown, this );
	Bind( wxEVT_MIDDLE_DOWN, &wx::Window::OnMouseDown, this );
	Bind( wxEVT_AUX1_DOWN, &wx::Window::OnMouseDown, this );
	Bind( wxEVT_AUX2_DOWN, &wx::Window::OnMouseDown, this );
	
	Bind( wxEVT_LEFT_UP, &wx::Window::OnMouseUp, this );
	Bind( wxEVT_RIGHT_UP, &wx::Window::OnMouseUp, this );
	Bind( wxEVT_MIDDLE_UP, &wx::Window::OnMouseUp, this );
	Bind( wxEVT_AUX1_UP, &wx::Window::OnMouseUp, this );
	Bind( wxEVT_AUX2_UP, &wx::Window::OnMouseUp, this );
	
	Bind( wxEVT_MOTION, &wx::Window::OnMouseMove, this );
}

//-------------------------------------------------------
//	mouse button down on window
//-------------------------------------------------------
void wx::Window::OnMouseDown(wxMouseEvent& Event)
{
	//	get a mouse input device
	TLInput::TInputDevice* pMouseDevice = TLInput::GetDeviceOfType( TLInput::MouseRef );
	if ( !pMouseDevice )
		return;
	
	//	push event onto the mouse device
	TLInput::TInputData InputData;
}


//-------------------------------------------------------
//	mouse button up on window
//-------------------------------------------------------
void wx::Window::OnMouseUp(wxMouseEvent& Event)
{
}

//-------------------------------------------------------
//	mouse moved on window
//-------------------------------------------------------
void wx::Window::OnMouseMove(wxMouseEvent& Event)
{
	
}

