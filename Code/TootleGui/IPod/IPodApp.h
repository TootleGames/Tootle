/*------------------------------------------------------

	

-------------------------------------------------------*/
#pragma once
#include "../TApp.h"
#include "IPodGui.h"


namespace TLGui
{
	namespace Platform
	{
		class App;
	}
}


@interface TAppDelegate : NSObject <UIAccelerometerDelegate>
//@interface OpenglESAppAppDelegate : NSObject <UIApplicationDelegate> 
{
	BOOL		m_HasInitialised;
	BOOL		m_HasShutdown;
	NSTimer*	m_pTimer;
	TPtr<TLGui::Platform::App>	m_pApp;
}

- (void) onTimer:(NSTimer*)timer;
- (void) Terminate;

@end



class TLGui::Platform::App : public TLGui::TApp
{
public:
	App()			{}
	
	Bool			Init()			{	return true;	}
	SyncBool		Update()		{	return SyncWait;	}
	SyncBool		Shutdown()		{	return SyncTrue;	}
};





