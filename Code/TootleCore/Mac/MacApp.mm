#include "../TLTime.h"
#include "../TCoreManager.h"
#import "MacApp.h"
#include "../TLCore.h"
#include "../TPtr.h"

#include <OpenGL/gl.h>


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

@synthesize window;
@synthesize glView;

-(void) applicationDidFinishLaunching:(NSNotification *)notification
{
	TLDebug_Print("applicationDidFinishLaunching");
			
	/*
	/////////////////////////////////////
	// Create the window and view
	/////////////////////////////////////
	CGRect					rect = [[UIScreen mainScreen] applicationFrame];
	
	//Create a full-screen window
	window = [[NSWindow alloc] initWithFrame:[[UIScreen mainScreen] bounds]];
	
	// Use a red window for debug so we can see any glitchy views of it
	//	gr: always pink background
	[window setBackgroundColor:[NSColor magentaColor]];
	
	//Create the OpenGL drawing view and add it to the window
	glView = [[TootleOpenGLView alloc] initWithFrame:CGRectMake(rect.origin.x, rect.origin.y, rect.size.width, rect.size.height)]; // - kPaletteHeight 
	[window addSubview:glView];
	//Show the window
	[window makeKeyAndVisible];	
	//////////////////////////////////////
	*/
	
	
	//	 do very basic init
	TLCore::g_pCoreManager = new TLCore::TCoreManager("CORE");
	
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
	[window release];
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


@implementation TootleOpenGLView


static void drawAnObject ()
{
	
    glColor3f(1.0f, 0.85f, 0.35f);
    glBegin(GL_TRIANGLES);	
    {
        glVertex3f(  0.0,  0.6, 0.0);
        glVertex3f( -0.2, -0.3, 0.0);
        glVertex3f(  0.2, -0.3 ,0.0);		
    }

    glEnd();

}

-(void) drawRect: (NSRect) bounds
{
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);
    drawAnObject();
    glFlush();
}


@end











