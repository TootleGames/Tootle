#include "MacApp.h"
#include <TootleCore/TCoreManager.h>
#include <TootleCore/TLCore.h>
#include <TootleCore/TLTime.h>
#include <TootleCore/PC/PCDebug.h>

#if defined(TL_ENABLE_WX)
#include "wxWidgets/App.h"
#endif


//	include NSApp, NSArray etc
#if !defined(TL_ENABLE_WX)
#include <TootleInput/Mac/MacInput.h>
#import <IOKit/IOKitLib.h>
#import <IOKit/hid/IOHIDManager.h>
#import <IOKit/hid/IOHIDKeys.h>
#import <Appkit/Appkit.h>
#include <TootleCore/TLMemory.h> // TEMP
#import "MacWindowDelegate.h"
#import <Cocoa/Cocoa.h>
#endif



#if !defined(TL_ENABLE_WX)

@implementation TootleNSApplicationDelegate


-(void) applicationDidFinishLaunching:(NSNotification *)notification
{
	//	init
	m_HasInitialised = FALSE;
	m_HasShutdown = FALSE;

	TLDebug_Print("applicationDidFinishLaunching");

	//	allocate an app
	m_pApp = new TLGui::Platform::App();
	
	//	do init loop
	Bool InitResult = m_pApp->Init();
	InitResult = InitResult ? TLCore::TootInit() : false;

	//	setup the update timer
	//	gr: normally this would go in the App Init (this is where it's done on the PC) but makes the code a little awkward because of the target/selector stuff 
	float UpdateInterval = TLTime::GetUpdateTimeSecondsf();
	m_pTimer = [NSTimer scheduledTimerWithTimeInterval:UpdateInterval target:self selector:@selector(onTimer:) userInfo:nil repeats:YES];
	
	
	
	//	Send terminate message to the application if we failed to init
	if ( !InitResult )
	{
		[self Terminate];
		return;
	}
	
	m_HasInitialised = TRUE;
	
	
	// Force an update and render.  This fixes the glitch between the Default.png file and the very first frame of the logo
	TLCore::g_pCoreManager->ForceUpdate();
	TLCore::g_pCoreManager->ForceRender();
}

- (void)applicationWillTerminate:(NSNotification *)aNotification 
{
	TLDebug_Print("applicationWillTerminate");
	m_HasShutdown = TRUE;

	//	kill timer
	//	gr: normally this would be done in the app shutdown
	[m_pTimer invalidate];
	m_pTimer = NULL;
	
	//	shutdown loop
	Bool ShutdownLoopResult = TLCore::TootShutdown( TRUE );
	m_pApp->Shutdown();
	m_pApp = NULL;
}


// Sleep Mode
- (void)applicationWillResignActive:(NSNotification *)aNotification 
{
	//TODO: Enter a sleep mode with minimal updates occuring
	// switch off render etc
	
	//	gr: turned this off for now cos im trying to use another tool at the same time
	//TLCore::g_pCoreManager->Enable(FALSE);
}

// Active mode
- (void)applicationDidBecomeActive:(NSNotification *)aNotification 
{	
	//TODO: Exit from sleep mode - re-enable render etc.
	TLCore::g_pCoreManager->Enable(TRUE);	
}

- (void) Terminate
{
	NSApplication* app = [NSApplication sharedApplication];
	SEL selector = @selector( terminate: );
		
	if ( [app respondsToSelector:selector] )
		[app performSelector:selector];
}

- (void) onTimer:(NSTimer*)timer
{
	//	core manager not enabled (ie. sleep) then do nothing
	if ( !TLCore::g_pCoreManager->IsEnabled() )
		return;

	//	dont do any threaded code whilst breaking
	if ( TLDebug::IsBreaking() )
		return;
	
	//	mark core as ready for another update
	if ( TLCore::g_pCoreManager )
		TLCore::g_pCoreManager->SetReadyForUpdate();
	
	//	do an update
	SyncBool UpdateResult = m_pApp->Update();
	UpdateResult = (UpdateResult == SyncWait) ? TLCore::TootUpdate() : UpdateResult;
	
	//	update okay, bail out until another timer hit
	if ( UpdateResult == SyncWait )
		return;
	
	//	update has finished, start shutting down
	[self Terminate];
}


@end


#endif //!defined(TL_ENABLE_WX)



