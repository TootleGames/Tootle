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
	TControl(TRefRef ControlRef);
	virtual ~TControl()	{}

	virtual bool			Initialise(TWindow& Parent);			//	initialise - this is seperated from the constructor so we can use virtual functions
	bool					HasInitialised() const					{	return m_pParent != NULL;	}
	TRefRef					GetRef() const							{	return m_ControlRef;	}
	TWindow&				GetParentWindow()						{	return *m_pParent;	}
	
	inline bool				operator==(TRefRef ControlRef) const	{	return m_ControlRef == ControlRef;	}
	
private:
	TRef			m_ControlRef;
	TWindow*		m_pParent;			//	should we allow this? currently just used to see if the control has been initialised
};

