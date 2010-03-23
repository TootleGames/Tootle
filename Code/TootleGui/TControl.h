/*
 *  TControl.h
 *  TootleGui
 *
 *  Created by Graham Reeves on 14/01/2010.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#pragma once
#include <TootleCore/TRef.h>
#include "TLGui.h"


//	forward declaration
namespace TLGui
{
	class TWindow;
}


//------------------------------------------------------
//	base control type for an OS control
//------------------------------------------------------
class TLGui::TControl
{
public:
	TControl(TWindow& Parent,TRefRef ControlRef);
	
	inline Bool		operator==(TRefRef ControlRef) const		{	return m_ControlRef == ControlRef;	}
	
private:
	TRef	m_ControlRef;
};

