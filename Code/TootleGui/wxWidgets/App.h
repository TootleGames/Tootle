/*
 *  TWxWindow.h
 *  TootleGui
 *
 *  Created by Graham Reeves on 14/01/2010.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#pragma once
#include "TLWx.h"
#include "../TApp.h"


namespace wx
{
	class App;
}

class wx::App : public TLGui::TApp, public wxApp
{
public:
    void			OnIdle(wxIdleEvent& Event);
	void			OnTimer(wxTimerEvent& Event);
    virtual bool    OnInit();

private:
	TPtr<wxTimer>	m_pUpdateTimer;
};

