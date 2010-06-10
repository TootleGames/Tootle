/*
 *  IPadApp.h
 *  TootleGui
 *
 *  Created by Duane Bradbury on 14/09/2009.
 *  Copyright 2009 Tootle. All rights reserved.
 *
 */

#pragma once
#include "../TApp.h"
#include "IPadGui.h"


namespace TLGui
{
	namespace Platform
	{
		class App;
	}
}


@interface   TAppDelegate : NSObject <UIAccelerometerDelegate> 
//@interface OpenglESAppAppDelegate : NSObject <UIApplicationDelegate> 
{
	BOOL		m_HasInitialised;
	BOOL		m_HasShutdown;
	NSTimer*	m_pTimer;
	TPtr<TLGui::Platform::App>	m_pApp;
}

- (void) onTimer:(NSTimer*)timer;
- (void) Terminate;

+ (void) RedundantJustToEnsureSymbolExported;

@end 


class TLGui::Platform::App : public TLGui::TApp
{
public:
	App()			{}
	
	Bool			Init()			{	return true;	}
	SyncBool		Update()		{	return SyncWait;	}
	SyncBool		Shutdown()		{	return SyncTrue;	}
};





