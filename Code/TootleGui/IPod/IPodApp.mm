#include "IPodApp.h"
#include <TootleCore/TCoreManager.h>
#include <TootleCore/TLCore.h>
#include <TootleCore/TLTime.h>
#include <TootleInput/IPod/IPodInput.h>



@implementation TAppDelegate


-(void) applicationDidFinishLaunching:(NSNotification *)notification
{
	//	init
	m_HasInitialised = FALSE;
	m_HasShutdown = FALSE;

	TLDebug_Print("applicationDidFinishLaunching");

	//	inhibit the sleep/screen dimming
	[[UIApplication sharedApplication] setIdleTimerDisabled:TRUE];

	//	allocate an app
	m_pApp = new TLGui::Platform::App();
	
	//	do init loop
	Bool InitResult = m_pApp->Init();
	InitResult = InitResult ? TLCore::TootInit() : false;

	//	setup the update timer
	//	gr: normally this would go in the App Init (this is where it's done on the PC) but makes the code a little awkward because of the target/selector stuff 
	float UpdateInterval = TLTime::GetUpdateTimeSecondsf();
	m_pTimer = [NSTimer scheduledTimerWithTimeInterval:UpdateInterval target:self selector:@selector(onTimer:) userInfo:nil repeats:YES];
	
	// Setup the accelerometer
	// Configure and start the accelerometer
	float AccellInterval = TLTime::GetUpdateTimeMilliSecsf();
    [[UIAccelerometer sharedAccelerometer] setUpdateInterval:AccellInterval];
    [[UIAccelerometer sharedAccelerometer] setDelegate:self];
		
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
	//	gr: in IOS 4, this function is called when we press home. But I kinda need the test game to close so I can try a different one.
	[self Terminate];
/*
	//TODO: Enter a sleep mode with minimal updates occuring
	// switch off render etc
	TLCore::g_pCoreManager->Enable(FALSE);
 */
}

// Active mode
- (void)applicationDidBecomeActive:(NSNotification *)aNotification 
{	
	//TODO: Exit from sleep mode - re-enable render etc.
	TLCore::g_pCoreManager->Enable(TRUE);	
}

- (void) Terminate
{
	UIApplication* app = [UIApplication sharedApplication];
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


// UIAccelerometerDelegate method, called when the device accelerates.
- (void)accelerometer:(UIAccelerometer *)accelerometer didAccelerate:(UIAcceleration *)acceleration 
{
    // Update the accelerometer graph view
    //[graphView updateHistoryWithX:acceleration.x Y:acceleration.y Z:acceleration.z];
	
	float3 AccelData(acceleration.x, acceleration.y, acceleration.z);

	TLInput::Platform::IPod::ProcessAcceleration( AccelData );
}


@end

