/*------------------------------------------------------

	

-------------------------------------------------------*/
#pragma once
#include "../TApp.h"




#if !defined(TL_ENABLE_WX)

#import <Cocoa/Cocoa.h>


namespace TLGui
{
	namespace Platform
	{
		class App;
	}
}


@interface TootleNSApplicationDelegate : NSObject <NSApplicationDelegate> 
{
	BOOL		m_HasInitialised;
	BOOL		m_HasShutdown;
	NSTimer*	m_pTimer;
	TPtr<TLGui::Platform::App>	m_pApp;
}
- (void) onTimer:(NSTimer*)timer;
- (void) Terminate;
//-(void) applicationDidFinishLaunching:(NSNotification *)notification;


@end



class TLGui::Platform::App : public TLGui::TApp
{
public:
	App()			{}
	
	Bool			Init()			{	return true;	}
	SyncBool		Update()		{	return SyncWait;	}
	SyncBool		Shutdown()		{	return SyncTrue;	}
};


#endif // !TL_ENABLE_WX



