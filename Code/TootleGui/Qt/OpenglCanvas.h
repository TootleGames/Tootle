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





namespace Qt
{
	class TOpenglCanvas;
}


class Qt::TOpenglCanvas : public TLGui::TOpenglCanvas, public QGLWidget, public Qt::TWidgetWrapper
{
public:
	TOpenglCanvas(TRefRef ControlRef);

	//	gui virtual
	virtual bool		Initialise(TLGui::TWindow& Parent);
	virtual Bool		BeginRender();
	virtual void		EndRender();
	virtual Type2<u16>	GetSize() const					{	return m_Size;	}	//	canvas(renderable area) size
	virtual bool		MakeCurrent()					{	QGLWidget::makeCurrent();	return true;	}	//	make this canvas' context 

	//	widget virtual
	//	The initialization of OpenGL rendering state, etc. should be done by overriding the initializeGL() function, rather than in the constructor of your QGLWidget subclass.
	virtual void		initializeGL();					//	init opengl state
	virtual void		resizeGL(int width, int height)	{	m_Size = Type2<u16>( width, height );	OnResized();	}
	
protected:
	virtual QWidget&	GetWidget()						{	return *this;	}

protected:
	Type2<u16>			m_Size;							//	size - same as QGLWidget::getSize() ?
};

