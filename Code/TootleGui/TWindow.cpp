/*
 *  TWindow.cpp
 *  TootleGui
 *
 *  Created by Graham Reeves on 14/01/2010.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#include "TWindow.h"




//------------------------------------------------------
//	base window type for an OS window.
//------------------------------------------------------
TLGui::TWindow::TWindow(TRefRef WindowRef) :
	m_WindowRef	( WindowRef )
{
}


//------------------------------------------------------
//	when [client] size of the window changes, broadcast a message 
//------------------------------------------------------
void TLGui::TWindow::OnSizeChanged() const
{
	TLDebug_Break("Todo");
}
