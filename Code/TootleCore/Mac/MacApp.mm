#include "../TLTime.h"
#include "../TCoreManager.h"
#import "MacApp.h"
#include "../TLCore.h"
#include "../TPtr.h"




namespace TLCore
{
	namespace Platform
	{
		extern TString						g_AppExe;
	}
	
	void RegisterManagers_Engine(TPtr<TCoreManager>& pCoreManager);
	void RegisterManagers_Game(TPtr<TCoreManager>& pCoreManager);
	
	extern TPtr<TCoreManager>		g_pCoreManager;
	
}
		

@implementation TootleNSApplicationDelegate

//@synthesize window;
@synthesize glView;

-(void) applicationDidFinishLaunching:(NSNotification *)notification
{
	TLDebug_Print("applicationDidFinishLaunching");

	//	 do very basic init
	TLCore::g_pCoreManager = new TLCore::TCoreManager("CORE");
	
	NSApplication* app = [NSApplication sharedApplication];	
	/////////////////////////////////////
	// Create the window and view
	/////////////////////////////////////
	NSRect					rect = [[NSScreen mainScreen] frame];
	 
	NSArray* windowlist = [app windows];
	NSWindow* window = nil;
	
	if([windowlist count] > 0)
	{
		window = [windowlist objectAtIndex:0];
	}
	
	
	if(window == nil)
	{
		//fail
	}
	 //Create a full-screen window
	 //NSWindow* window = [[NSWindow alloc] initWithFrame:[[NSScreen mainScreen] frame]];
	 
	//Show the window
	[window makeKeyAndOrderFront:nil];	
	
	// Set window for application
	//[app setWindow:window];
	
	 // Use a red window for debug so we can see any glitchy views of it
	 //	gr: always pink background
	[window setBackgroundColor:[NSColor magentaColor]];
	
	TootleWindowDelegate* pWindowDelegate = [TootleWindowDelegate alloc];
	
	if(pWindowDelegate)
	{
		[window setDelegate:pWindowDelegate];
		[pWindowDelegate setWindow:window];
	}
	
	//Create the OpenGL drawing view and add it to the window programatically
	// NOTE: Only needed if the window is not setup in a project .nib file
	
	glView = [[TootleOpenGLView alloc] initWithFrame:rect]; // - kPaletteHeight
	//[[NSApp mainWindow] addSubview:glView];
	 
	if(!glView)
	{
		TLDebug_Print("Application failed to initialise view");
		TLDebug_Print("Sending terminate message to application");
		
		// Send terminate message to the application
		NSApplication* app = [NSApplication sharedApplication];
		SEL selector = @selector( terminate: );
		
		if ( [app respondsToSelector:selector] )
		{
			[app performSelector:selector];
		}		
		return;
	}
	

	//NSView* contentview = [[NSApp mainWindow] contentView];
	//[contentview addSubview:glView];
	[window setContentView:glView];
//	[glView setWindow:window];
	
	// Now we are part of the window we can setup the opengl context
	// NOTE: Has to be done AFTER the view is added to the window
	[glView initOpenGLContext];	
	
	

	
	// finsihed with the window now so we can release it
	//[window release];

	//////////////////////////////////////

	
	// Register the engine managers
	TLCore::RegisterManagers_Engine( TLCore::g_pCoreManager );
	TLCore::RegisterManagers_Game( TLCore::g_pCoreManager );
	
	//	start update timer
	init = FALSE;
	shutdown = FALSE;

	//	mark as ready for first update
	if ( TLCore::g_pCoreManager )
		TLCore::g_pCoreManager->SetReadyForUpdate();

	//	do init loop
	SyncBool Result = SyncWait;
	
	do
	{
		Result = TLCore::g_pCoreManager->InitialiseLoop();
		
	}while(Result == SyncWait);
	
	// Failed to initialise??
	if(Result == SyncFalse)
	{
		TLDebug_Print("Application failed to initialise");
		TLDebug_Print("Sending terminate message to application");
		
		// Send terminate message to the application
		NSApplication* app = [NSApplication sharedApplication];
		SEL selector = @selector( terminate: );
		
		if ( [app respondsToSelector:selector] )
		{
			[app performSelector:selector];
		}		
		
		return;
	}
	
	init = TRUE;
	
	timer = [NSTimer scheduledTimerWithTimeInterval:1.f/100.f target:self selector:@selector(onTimer:) userInfo:nil repeats:YES];
		
	
	// Force an update and render.  This fixes the glitch between the Default.png file and the very first frame of the logo
	TLCore::g_pCoreManager->ForceUpdate();
	TLCore::g_pCoreManager->ForceRender();
	
	///////////////////////////////////////////////////////////////////
	// Calculate time it took to go through the initialisation sequence
	///////////////////////////////////////////////////////////////////
	TLTime::TTimestampMicro InitialiseTime(TRUE);	
	
	TLCore::g_pCoreManager->StoreTimestamp("TSInitTime", InitialiseTime);
	
	TLTime::TTimestampMicro StartTime;
	
	if(TLCore::g_pCoreManager->RetrieveTimestamp("TSStartTime", StartTime))
	{	
		s32 Secs, MilliSecs, MicroSecs;
		StartTime.GetTimeDiff(InitialiseTime, Secs, MilliSecs, MicroSecs);
		
		TTempString time;
		time.Appendf("%d.%d:%d Seconds", Secs, MilliSecs, MicroSecs);
		TLDebug_Print("App finished launching");
		TLDebug_Print(time.GetData());
	}
	///////////////////////////////////////////////////////////////////
	
	
}

- (void)applicationWillTerminate:(NSApplication *)application 
{
	TLDebug_Print("applicationWillTerminate");
	
	///////////////////////////////////////////////////////////////////
	// Calculate total run time
	///////////////////////////////////////////////////////////////////
	TLTime::TTimestampMicro EndTime(TRUE);
	
	TLCore::g_pCoreManager->StoreTimestamp("TSEndTime", EndTime);
	
	TLTime::TTimestampMicro InitTime;
	
	if(TLCore::g_pCoreManager->RetrieveTimestamp("TSInitTime", InitTime))
	{	
		s32 Secs, MilliSecs, MicroSecs;
		InitTime.GetTimeDiff(EndTime, Secs, MilliSecs, MicroSecs);
		
		TTempString time;
		time.Appendf("%d.%d:%d Seconds", Secs, MilliSecs, MicroSecs);
		TLDebug_Print("App shutting down");
		TLDebug_Print("Total run time:");
		TLDebug_Print(time.GetData());
	}
	///////////////////////////////////////////////////////////////////
	
	
	shutdown = TRUE;
	
	// Do shutdown loop
	SyncBool Result = SyncWait;
	
	do
	{
		Result = TLCore::g_pCoreManager->UpdateShutdown();
	}
	while( Result == SyncWait );
	
	// Destroy the core manager
	TLCore::g_pCoreManager = NULL;
	
	TLCore::Platform::g_AppExe.Empty(TRUE);
	
}


// Sleep Mode
- (void)applicationWillResignActive:(NSApplication *)application 
{
	//TODO: Enter a sleep mode with minimal updates occuring
	// switch off render etc
	TLCore::g_pCoreManager->Enable(FALSE);
}

// Active mode
- (void)applicationDidBecomeActive:(NSApplication *)application 
{	
	//TODO: Exit from sleep mode - re-enable render etc.
	TLCore::g_pCoreManager->Enable(TRUE);	
}


- (void)dealloc {
	//[window release];
	[glView release];
	[super dealloc];
}

- (void) onTimer:(NSTimer*)timer
{
	// If enabled go through the update loop
	if(TLCore::g_pCoreManager->IsEnabled())
	{
		//	mark core as ready for another update
		if ( TLCore::g_pCoreManager )
			TLCore::g_pCoreManager->SetReadyForUpdate();
		
		if ( init == NO )
		{
			SyncBool Result = TLCore::g_pCoreManager->InitialiseLoop();
			if ( Result == SyncTrue )
				init = TRUE;		
		}
		else if( shutdown == FALSE )
		{
			//	do update loop
			SyncBool Result = TLCore::g_pCoreManager->UpdateLoop();
			
			if(Result == SyncTrue)
			{
				TLDebug_Print("Sending terminate message to application");
				
				
				// Send terminate message to the application
				NSApplication* app = [NSApplication sharedApplication];
				SEL selector = @selector( terminate: );
				
				if ( [app respondsToSelector:selector] )
				{
					[app performSelector:selector];
				}		
			}
		}	
		else
		{
			// Do nothing - shutting down
		}
	}
}


@end













