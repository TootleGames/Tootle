/*
 *  TControl.cpp
 *  TootleGui
 *
 *  Created by Graham Reeves on 14/01/2010.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#include "TControl.h"
#include "TWindow.h"


//------------------------------------------------------
//	
//------------------------------------------------------
TLGui::TControl::TControl(TWindow& Parent,TRefRef ControlRef) :
	m_ControlRef	( ControlRef )
{
	Parent.AddChild( this );
}

