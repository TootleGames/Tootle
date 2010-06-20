/*
 *  OpenglCanvas.h
 *  TootleGui
 *
 *  Created by Graham Reeves on 14/01/2010.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */
#if !defined(TL_ENABLE_QT)
#error Should only be built in QT only build
#endif // TL_ENABLE_QT

#pragma once
#include "../TOpenglCanvas.h"
#include "TLQt.h"
#include <QtOpengl/QGLWidget>



/*

namespace Qt
{
	class OpenglCanvas;
}


class Qt::OpenglCanvas : public TLGui::TOpenglCanvas, public QGLWidget
{
public:
	OpenglCanvas(TLGui::TWindow& Parent,TRefRef ControlRef);

	//	gui virtual
	virtual Bool	BeginRender();
	virtual void	EndRender();

	//	widget virtual
	//	The initialization of OpenGL rendering state, etc. should be done by overriding the initializeGL() function, rather than in the constructor of your QGLWidget subclass.
	virtual void		initializeGL();					//	init opengl state
	virtual void		resizeGL(int width, int height)	{	}//OnResized( int2(width,height) );	}
}

*/
