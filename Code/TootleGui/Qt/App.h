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
#include "../TApp.h"

#include <QTCore/QTimer>


namespace TLGui
{
	namespace Platform
	{
		class App;
	}
}



class TLGui::Platform::App : public TLGui::TApp, public QApplication
{
public:
	App(int argc, char *argv[]);
	~App()			{}
	
	bool			Init();			//	init the app
	SyncBool		Update()		{	return SyncWait;	}
	void			Shutdown();		//	cleanup

protected:
	virtual void	timerEvent(QTimerEvent* TimerEvent);		//	timer callback
};

