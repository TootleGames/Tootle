/*
 *  TOpenglCanvas.h
 *  TootleGui
 *
 *  Created by Graham Reeves on 14/01/2010.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */
#pragma once
#include "TControl.h"




class TLGui::TOpenglCanvas : public TLGui::TControl
{
public:
	TOpenglCanvas(TWindow& Parent,TRefRef ControlRef);

	virtual Bool	BeginRender()=0;		//	set as current opengl canvas before rendering. If this fails we cannot draw to it
	virtual void	EndRender()=0;			//	flip buffers after drawing
};

