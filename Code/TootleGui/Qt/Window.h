/*
 *  TWxWindow.h
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
#include "TLQt.h"
#include "../TWindow.h"


namespace Qt
{
	class Window;
}

/*

//-------------------------------------------------------
//	don't need to use a specific window widget
//-------------------------------------------------------
class Qt::Window : public TLGui::TWindow, public QWidget
{
public:
	Window(TRefRef WindowRef);
	
	//	gui virtual
	virtual void		Show()		{	QWidget::show();	}
};
*/

