#include "../TLTime.h"
#include "../TCoreManager.h"
#import "IPodApp.h"
#include "../TLCore.h"
#include "../TPtr.h"

#import <TootleAsset/TAsset.h>
#import <TootleInput/Ipod/IPodInput.h>

#import <QuartzCore/QuartzCore.h>
#import <OpenGLES/EAGLDrawable.h>


// Constant for the number of times per second (Hertz) to sample acceleration.
#define ACCELEROMETER_FREQUENCY     30



namespace TLCore
{
	extern TPtr<TCoreManager>		g_pCoreManager;

	void RegisterManagers_Engine(TPtr<TCoreManager>& pCoreManager);
	void RegisterManagers_Game(TPtr<TCoreManager>& pCoreManager);
}


/*

@implementation eaglAppDelegate


- (void)applicationDidFinishLaunching:(UIApplication *)application
{	
	timer = [NSTimer scheduledTimerWithTimeInterval:1.0/24 target:self selector:@selector(onTimer:) userInfo:nil repeats:YES];
		
	//	go to the tootle main loop
	TLCore::TootMain();
}


- (void)dealloc 
{
	[super dealloc];
}


- (void) onTimer:(NSTimer*)timer
{
	//	add an update timestamp to the update time queue
	TLTime::TTimestamp UpdateTimerTime( TRUE );
	
	if ( TLCore::g_pCoreManager )
		TLCore::g_pCoreManager->AddTimeStep( UpdateTimerTime );
}


@end
 */



OpenglESAppAppDelegate* g_pIpodApp;



@implementation OpenglESAppAppDelegate

@synthesize window;
@synthesize glView;


// IPod Initialisation
- (void)applicationDidFinishLaunching:(UIApplication *)application 
{
	TLDebug_Print("applicationDidFinishLaunching");

	g_pIpodApp = self;

	//	inhibit the sleep/screen dimming
	[[UIApplication sharedApplication] setIdleTimerDisabled:TRUE];

	
	/////////////////////////////////////
	// Create the window and view
	/////////////////////////////////////
	CGRect					rect = [[UIScreen mainScreen] applicationFrame];
	
	//Create a full-screen window
	window = [[UIWindow alloc] initWithFrame:[[UIScreen mainScreen] bounds]];

	// Use a red window for debug so we can see any glitchy views of it
	//	gr: always pink background
	[window setBackgroundColor:[UIColor magentaColor]];

	//Create the OpenGL drawing view and add it to the window
	glView = [[EAGLView alloc] initWithFrame:CGRectMake(rect.origin.x, rect.origin.y, rect.size.width, rect.size.height)]; // - kPaletteHeight 
	[window addSubview:glView];
	//Show the window
	[window makeKeyAndVisible];	
	//////////////////////////////////////
	
	//	 do very basic init
	TLCore::g_pCoreManager = new TLCore::TCoreManager("CORE");
	
	// Register the engine managers
	TLCore::RegisterManagers_Engine( TLCore::g_pCoreManager );
	TLCore::RegisterManagers_Game( TLCore::g_pCoreManager );
	
	//	start update timer
	init = FALSE;
	shutdown = FALSE;
	


	//	add an update timestamp to the update time queue
	TLTime::TTimestamp UpdateTimerTime( TRUE );
	if ( TLCore::g_pCoreManager )
		TLCore::g_pCoreManager->AddTimeStep( UpdateTimerTime );
	
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
		UIApplication* app = [UIApplication sharedApplication];
		SEL selector = @selector( terminate );
		
		if ( [app respondsToSelector:selector] )
		{
			[app performSelector:selector];
		}		
		
		return;
	}
	
	init = TRUE;

	/*
	// When not in debug mode wait for the logo to load so that we don't get the window appearing in
	// betwen the default.png image and the first render of the game window
	// This needs looking into further using debug to see why this gap occurs
#ifndef _DEBUG
	// Update until the 'logo' file is loaded
	Bool logoloaded = FALSE;
	do
	//for(u32 uIndex = 0; uIndex < 100; uIndex++)
	{
		Result = TLCore::g_pCoreManager->UpdateLoop();
		
		if(Result == SyncTrue)
		{
			
			// Send terminate message to the application
			UIApplication* app = [UIApplication sharedApplication];
			SEL selector = @selector( terminate );
			
			if ( [app respondsToSelector:selector] )
			{
				[app performSelector:selector];
			}		
		}
		
		TPtr<TLAsset::TAsset>& pAsset = TLAsset::GetAsset("logo", TRUE);
		
		if(pAsset)
			logoloaded = TRUE;
	
	} while(!logoloaded);
#endif
	 */
	
/*
	SyncBool Result = TLCore::g_pCoreManager->InitialiseLoop();
	if ( Result == SyncTrue )
		init = TRUE;
*/	
	
	timer = [NSTimer scheduledTimerWithTimeInterval:1.f/100.f target:self selector:@selector(onTimer:) userInfo:nil repeats:YES];
	
	//	[glView startAnimation];
	
	// Setup the accelerometer
	// Configure and start the accelerometer
    [[UIAccelerometer sharedAccelerometer] setUpdateInterval:(1.0 / ACCELEROMETER_FREQUENCY)];
    [[UIAccelerometer sharedAccelerometer] setDelegate:self];

	// Force an update and render.  This fixes the glitch between the Default.png file and the very first frame of the logo
	TLCore::g_pCoreManager->ForceUpdate();
	TLCore::g_pCoreManager->ForceRender();
}



// Ipod Shutdown
- (void)applicationWillTerminate:(UIApplication *)application 
{
	TLDebug_Print("applicationWillTerminate");

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
}


// Sleep Mode
- (void)applicationWillResignActive:(UIApplication *)application 
{
	//TODO: Enter a sleep mode with minimal updates occuring
	// switch off render etc
	TLCore::g_pCoreManager->Enable(FALSE);
}

// Active mode
- (void)applicationDidBecomeActive:(UIApplication *)application 
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
		//	add an update timestamp to the update time queue
		TLTime::TTimestamp UpdateTimerTime( TRUE );
		if ( TLCore::g_pCoreManager )
			TLCore::g_pCoreManager->AddTimeStep( UpdateTimerTime );
		
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
				UIApplication* app = [UIApplication sharedApplication];
				SEL selector = @selector( terminate );
				
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


// UIAccelerometerDelegate method, called when the device accelerates.
- (void)accelerometer:(UIAccelerometer *)accelerometer didAccelerate:(UIAcceleration *)acceleration {
    // Update the accelerometer graph view
    //[graphView updateHistoryWithX:acceleration.x Y:acceleration.y Z:acceleration.z];
	
	
	TPtr<TLInput::Platform::IPod::TAccelerationData> pAccelerationData = new TLInput::Platform::IPod::TAccelerationData();
	
	if(pAccelerationData)
	{
		pAccelerationData->vAcceleration = float3(acceleration.x, acceleration.y, acceleration.z);
		
		TLInput::Platform::IPod::ProcessAcceleration(pAccelerationData);
	}
}


@end














