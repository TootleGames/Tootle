#import "MacCore.h"
#import "MacApp.h"

#include "../TLCore.h"
#include "../TLTypes.h"
#include "../TString.h"
#include "../TCoreManager.h"


//	for the app code
#import <TootleAsset/TAsset.h>
#import <TootleInput/Ipod/IPodInput.h>
#import <QuartzCore/QuartzCore.h>

#import <Cocoa/Cocoa.h>


//#import <OpenGLES/EAGLDrawable.h>

//	Constant for the number of times per second (Hertz) to sample acceleration.
#define ACCELEROMETER_FREQUENCY     30


namespace TLCore
{
	namespace Platform
	{
		TString						g_AppExe;
		
		void				GetString(TString& String, const NSString* pNSString);

	}
	
	void RegisterManagers_Engine(TPtr<TCoreManager>& pCoreManager);
	void RegisterManagers_Game(TPtr<TCoreManager>& pCoreManager);

	extern TPtr<TCoreManager>		g_pCoreManager;
}




//---------------------------------------------------
//	app entry
//---------------------------------------------------
int main(int argc, char *argv[])
{
	//	set exe
	//	gr: just have xyz.app as the exe name. the file sys prepends all directories by the app root
	//TLCore::Platform::g_AppExe = (argc == 0) ? "???" : argv[0];

	//	get the root directory that the app is in
	NSString *HomeDir = NSHomeDirectory();
	TLCore::Platform::GetString( TLCore::Platform::g_AppExe, HomeDir );

	int retVal = NSApplicationMain(argc,  (const char **) argv);

	return retVal;
}


//--------------------------------------------------
//	platform init
//--------------------------------------------------
SyncBool TLCore::Platform::Init()
{
	
	return SyncTrue;
}


void TLCore::Platform::QueryHardwareInformation(TBinaryTree& Data)	
{
}

void TLCore::Platform::QueryLanguageInformation(TBinaryTree& Data)	
{
}



//--------------------------------------------------
//	platform update
//--------------------------------------------------
SyncBool TLCore::Platform::Update()
{
	//	keep app running
	return SyncTrue;
}



//--------------------------------------------------
//	platform shutdown
//--------------------------------------------------
SyncBool TLCore::Platform::Shutdown()
{	
	return SyncTrue;
}


void TLCore::Platform::DoQuit()
{
	// Send a message to the core manager telling it to quit
	TLMessaging::TMessage Message("Quit");
	
	TLCore::g_pCoreManager->QueueMessage(Message);
}



//--------------------------------------------------
//	get the application exe
//--------------------------------------------------
const TString& TLCore::Platform::GetAppExe()
{
	return g_AppExe;
}


//--------------------------------------------------
//	platform specific debug text output
//--------------------------------------------------
void TLDebug::Platform::Print(const TString& String)
{
	NSString *logString = [[NSString alloc] initWithUTF8String: String.GetData()];
	NSLog(@"%@", logString );
	[logString release];

//	printf( String.GetData() );
//	printf("\n");
}


//--------------------------------------------------
//	return FALSE to stop app, TRUE and will attempt to continue
//--------------------------------------------------
Bool TLDebug::Platform::Break(const TString& String)
{
	Print( String );

	// Drop into debugger if possible
	// DB:	Found solution to getting the CoreServices linked *only* for the iphone simulator build
	//		In the target's info goto the Linking->Other Link Flags option and add -framework CoreServices
	//		via the cog at the bottom of the pane (add build setting condition) then select Any iPhone Simulator for the SDK option
	// and this should link correctly only on a simulator build :)
#if !TARGET_OS_IPHONE || TARGET_IPHONE_SIMULATOR
	Debugger();
#elif defined(_DEBUG)
	//	gr: hacky break for debug builds
	//	gr: removed because there's no way to get out of this in xcode if we trigger it..
	int* pNull = 0x0;
	//*pNull = 99;
#endif
	
	//	fail
	return FALSE;
}



//--------------------------------------------------
//	append ipod foundation string to TString
//--------------------------------------------------
void TLCore::Platform::GetString(TString& String, const NSString* pNSString)
{
    const char* pNSChars = [pNSString UTF8String];
	u32 NsLength = [pNSString length];

	String.Append( pNSChars, NsLength );
}





/*
 [19/02/09] DB - Removed from Mac build


@implementation OpenglESAppAppDelegate

@synthesize window;
@synthesize glView;


// IPod Initialisation
- (void)applicationDidFinishLaunching:(UIApplication *)application 
{
	TLDebug_Print("applicationDidFinishLaunching");
	
	TLCore::Platform::g_pIPodApp = self;
	
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
	
	float3 AccelData(acceleration.x, acceleration.y, acceleration.z);

	TLInput::Platform::IPod::ProcessAcceleration( AccelData );
}

@end
 
 */

