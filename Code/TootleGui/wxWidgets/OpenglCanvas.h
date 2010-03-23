/*
 *  OpenglCanvas.h
 *  TootleGui
 *
 *  Created by Graham Reeves on 14/01/2010.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */


#pragma once
#include "../TOpenglCanvas.h"

#include "TLWx.h"
#include "wx/glcanvas.h"


#if !wxUSE_GLCANVAS
	#error "OpenGL required: set wxUSE_GLCANVAS to 1 and rebuild the library"
#endif

namespace wx
{
	class OpenglCanvas;
}

class wx::OpenglCanvas : public TLGui::TOpenglCanvas, public wxGLCanvas
{
public:
	OpenglCanvas(TLGui::TWindow& Parent,TRefRef ControlRef);

	virtual Bool	BeginRender();
	virtual void	EndRender();
	
protected:
//	void			OnPaint(wxPaintEvent& Event);
};

