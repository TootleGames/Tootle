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
#include "../TWindow.h"


namespace wx
{
	class Window;
	FORCEINLINE wxWindow* GetWindow(TLGui::TWindow& Window);
}

class wx::Window : public TLGui::TWindow, public wxFrame
{
public:
	Window(TRefRef WindowRef);

	virtual Bool			IsVisible() const					{	return wxFrame::IsShown();	}
	virtual void			Show()								{	wxFrame::Show();	}

	virtual void			SetSize(const int2& WidthHeight)	{	wxFrame::SetClientSize( WidthHeight.x, WidthHeight.y );	}
	virtual int2			GetSize()							{	return int2( wxFrame::GetClientSize().x, wxFrame::GetClientSize().y );	}
	virtual void			SetPosition(const int2& xy)			{	wxFrame::Move( xy.x, xy.y );	}

private:
	void					OnMouseDown(wxMouseEvent& Event);	//	mouse button down on window
	void					OnMouseUp(wxMouseEvent& Event);		//	mouse button up on window
	void					OnMouseMove(wxMouseEvent& Event);	//	mouse moved on window
};


FORCEINLINE wxWindow* wx::GetWindow(TLGui::TWindow& Window)
{
	wx::Window& Win = static_cast<wx::Window&>( Window );
	return &Win;
}
