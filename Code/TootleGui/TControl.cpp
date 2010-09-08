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
TLGui::TControl::TControl(TRefRef ControlRef) :
	m_ControlRef	( ControlRef ),
	m_pParent		( NULL )
{
}


//------------------------------------------------------
//	initialise - this is seperated from the constructor so we can use virtual functions
//------------------------------------------------------
bool TLGui::TControl::Initialise(TWindow& Parent)
{
	Parent.AddChild( *this );
	m_pParent = &Parent;
	return true;
}

