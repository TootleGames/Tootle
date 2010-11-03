/*
 *  OpenglCanvas.cpp
 *  TootleGui
 *
 *  Created by Graham Reeves on 14/01/2010.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */
#include "OpenglCanvas.h"
#include "Window.h"
#include <TootleRender/TLRender.h>




Qt::TOpenglCanvas::TOpenglCanvas(TRefRef ControlRef) :
	TLGui::TOpenglCanvas	( ControlRef )
{
}


bool Qt::TOpenglCanvas::Initialise(TLGui::TWindow& Parent)
{
	QGLWidget::setParent( static_cast<Qt::TWindow*>( &Parent ) );

	//	catch mouse move's (this can be captured by parent)
	setMouseTracking( true );

	return TLGui::TOpenglCanvas::Initialise( Parent );
}


//------------------------------------------------------
//	set as current opengl canvas. If this fails we cannot draw to it
//------------------------------------------------------
Bool Qt::TOpenglCanvas::BeginRender()
{
	MakeCurrent();
	
	return TRUE;
}

//------------------------------------------------------
//	flip buffer at end of render
//------------------------------------------------------
void Qt::TOpenglCanvas::EndRender()
{
	QGLWidget::swapBuffers();
}


//------------------------------------------------------
//	init opengl state
//------------------------------------------------------
void Qt::TOpenglCanvas::initializeGL()					
{
	//TLRender::Opengl::Init();	
}	


