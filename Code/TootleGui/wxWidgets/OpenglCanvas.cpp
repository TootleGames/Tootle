/*
 *  OpenglCanvas.cpp
 *  TootleGui
 *
 *  Created by Graham Reeves on 14/01/2010.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */
#if !defined(TL_ENABLE_WX)
#error Should only be built in wx only build
#endif // TL_ENABLE_WX


#include "OpenglCanvas.h"
#include "Window.h"

//	globally shared context
TPtr<wxGLContext> gpContext = NULL;


//------------------------------------------------------
//	
//------------------------------------------------------
TPtr<TLGui::TOpenglCanvas> TLGui::CreateOpenglCanvas(TWindow& Parent,TRefRef Ref)
{
	TPtr<TLGui::TOpenglCanvas> pControl = new wx::OpenglCanvas( Parent, Ref );
	return pControl;
}


wx::OpenglCanvas::OpenglCanvas(TLGui::TWindow& Parent,TRefRef ControlRef) :
	TLGui::TOpenglCanvas	( Parent, ControlRef ),
	wxGLCanvas				( GetWindow(Parent), GetID(ControlRef), NULL, wxDefaultPosition, wxDefaultSize, wxFULL_REPAINT_ON_RESIZE )
{
}


//------------------------------------------------------
//	set as current opengl canvas. If this fails we cannot draw to it
//------------------------------------------------------
Bool wx::OpenglCanvas::BeginRender()
{
	if ( !gpContext )
	{
		// Create the OpenGL context for the first window which needs it:
		// subsequently created windows will all share the same context.
		gpContext = new wxGLContext( this );
		gpContext->SetCurrent( *this );
		
		//	setup default state of the context
		glEnable( GL_SCISSOR_TEST );
		glDisable( GL_TEXTURE_2D );
		glDisable( GL_CULL_FACE ); 
	}
	
	gpContext->SetCurrent(*this);
	
	return TRUE;
}

//------------------------------------------------------
//	flip buffer at end of render
//------------------------------------------------------
void wx::OpenglCanvas::EndRender()
{
	SwapBuffers();
}

