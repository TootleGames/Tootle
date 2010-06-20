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



/*

Qt::OpenglCanvas::OpenglCanvas(TLGui::TWindow& Parent,TRefRef ControlRef) :
	TLGui::TOpenglCanvas	( Parent, ControlRef ),
	QGLWidget				( static_cast<Qt::TWindow&>( Parent ) )
{
}



//------------------------------------------------------
//	set as current opengl canvas. If this fails we cannot draw to it
//------------------------------------------------------
Bool Qt::OpenglCanvas::BeginRender()
{
	QGLWidget::makeCurrent();
	
	return TRUE;
}

//------------------------------------------------------
//	flip buffer at end of render
//------------------------------------------------------
void Qt::OpenglCanvas::EndRender()
{
	SwapBuffers();
}


//------------------------------------------------------
//	init opengl state
//------------------------------------------------------
void Qt::OpenglCanvas::initializeGL()					
{
	TLRender::Opengl::Init();	
}	

*/
