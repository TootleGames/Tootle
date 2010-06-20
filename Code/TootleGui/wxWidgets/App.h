/*
 *  TWxWindow.h
 *  TootleGui
 *
 *  Created by Graham Reeves on 14/01/2010.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */
#if !defined(TL_ENABLE_WX)
#error Should only be built in wx only build
#endif // TL_ENABLE_WX

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
    static void		OnIdle(wxIdleEvent& Event);
	static void		OnTimer(wxTimerEvent& Event);
    virtual bool    OnInit();

private:
	TPtr<wxTimer>	m_pUpdateTimer;
};

