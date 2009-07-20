#import "IPodCore.h"
#import "IPodApp.h"

#include "../TLCore.h"
#include "../TLTypes.h"
#include "../TString.h"
#include "../TCoreManager.h"
#include "../TLTime.h"


//	for the app code
#import <TootleAsset/TAsset.h>
#import <TootleInput/Ipod/IPodInput.h>
#import <QuartzCore/QuartzCore.h>
#import <OpenGLES/EAGLDrawable.h>

#import <UIKit/UIDevice.h>

#import <IPodAlert.h>

//	Constant for the number of times per second (Hertz) to sample acceleration.
#define ACCELEROMETER_FREQUENCY     30


namespace TLCore
{
	namespace Platform
	{
		TString				g_AppExe;
		
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
	TLCore::Platform::g_AppExe.Append("/myapp.app");

//	int retVal = UIApplicationMain(argc, argv, nil, [OpenglESAppAppDelegate class]);	//	
	int retVal = UIApplicationMain(argc, argv, nil, @"OpenglESAppAppDelegate");	//	[OpenglESAppAppDelegate class]
//	int retVal = UIApplicationMain(argc, argv, nil, nil);
	
	return retVal;
}


//--------------------------------------------------
//	platform init
//--------------------------------------------------
SyncBool TLCore::Platform::Init()
{
	
	return SyncTrue;
}


// Populate the binary tree with hardware specific information
void TLCore::Platform::QueryHardwareInformation(TBinaryTree& Data)
{
	TLDebug_Print("Device Information:");

	/////////////////////////////////////////////////////////////
	// Device ID, OS and type
	/////////////////////////////////////////////////////////////	

	// iPod and iPhone have only one ARM processor
	// Add this to the data so it can be used at a later date
	Data.ExportData("CPU#", 1);	

	// Write the UDID
	NSString* pNSString = [[UIDevice currentDevice] uniqueIdentifier];		// a string unique to each device based on various hardware info.
	const char * pString = [pNSString UTF8String];
	TTempString devicedata(pString);	
	Data.ExportData("UDID", devicedata);
	
	TLDebug_Print(devicedata);

	// Write the type of device
	pNSString = [[UIDevice currentDevice] model];					// @"iPhone", @"iPod Touch"
	pString = [pNSString UTF8String];	
	devicedata = pString;	
	Data.ExportData("Type", devicedata);

	TLDebug_Print(devicedata);
	
	// Write the OS
	pNSString = [[UIDevice currentDevice] systemName];				// @"iPhone OS"
	pString = [pNSString UTF8String];	
	devicedata = pString;	
	Data.ExportData("OS", devicedata);

	TLDebug_Print(devicedata);

	// Write the OS version
	pNSString = [[UIDevice currentDevice] systemVersion];			// @"2.0"
	pString = [pNSString UTF8String];	
	devicedata = pString;	
	Data.ExportData("OSVer", devicedata);

	TLDebug_Print(devicedata);
	
	
	/////////////////////////////////////////////////////////////
	TLDebug_Print("End Device Information");
}

void TLCore::Platform::QueryLanguageInformation(TBinaryTree& Data)
{
	TLDebug_Print("Language Information:");

	/////////////////////////////////////////////////////////////
	// Langauge
	/////////////////////////////////////////////////////////////
	NSUserDefaults* defs = [NSUserDefaults standardUserDefaults];
	
	NSArray* languages = [defs objectForKey:@"AppleLanguages"];
	
	NSString* preferredLang = [languages objectAtIndex:0];

	//Convert the string to a TRef version so we don't need to pass a string around
	const char* pString = [preferredLang UTF8String];

	TTempString languagestr(pString);
	TLDebug_Print(languagestr);

	// Test the preferredLang and store our TRef version in the data tree
	// Default to english
	TRef LanguageRef = "eng";
	
	// Go through the language string and return an appropriate TRef of the specified language.
	// This will test *all* languages we will possibly support.
	if(languagestr == "en")				// English
		LanguageRef = "eng";
	else if(languagestr == "fr")		// French
		LanguageRef = "fre";
	else if(languagestr == "ge")		// German
		LanguageRef = "ger";
	else if(languagestr == "sp")		// Spanish
		LanguageRef = "spa";
	else if(languagestr == "it")		// Italian
		LanguageRef = "ita";
	else if(languagestr == "nl")		// Netherlands
		LanguageRef = "ned";
	else if(languagestr == "ja")		// Japanese
		LanguageRef = "jap";
	else
	{
		TLDebug_Print("Hardware langauge not supported - defaulting to english");
	}
	
	// Export the language selected to the data - actual language selection will be done 
	// via the core manager
	Data.ExportData("Language", LanguageRef);

	/////////////////////////////////////////////////////////////

	TLDebug_Print("End Language Information");
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
/*
	// Create an alert so that we can skip debugger or breaking
	NSString* errornsstring = [[NSString alloc] initWithCString:String.GetData() ];
		
	AlertsViewController *alertsViewController = [[AlertsViewController alloc] init];
	[alertsViewController dialogueOKCancelAction: errornsstring];
	
	// Wait for the error dialogue to be dismissed
	SyncBool Res = SyncWait;
	
	do
	{
		Res = [alertsViewController dialogueResult];
	} while(Res == SyncWait);
	
	[errornsstring release];
	[alertsViewController release];	
*/

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
	
	
	//assert(FALSE);
	
	
	//	gr: new method, untested, see https://devforums.apple.com/message/99580
	/*
	__builtin_trap() is one option. If the debugger is running then you'll stop in the debugger, 
	otherwise you'll crash with a crash log. But there's no guarantee that you can tell the debugger 
	to continue running your program after that - the compiler thinks __builtin_trap() halts the process 
	so it may optimize away any code after it.

	asm("trap") or asm("int3") is an architecture-specific option. The behavior is the same as __builtin_trap(), 
	except the compiler doesn't optimize around it so you should be able to continue running afterwards.
	*/
	//__builtin_trap();
	//asm("int3");		//	note: same as PC version
	
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


//--------------------------------------------------
//	platform thread/process sleep
//--------------------------------------------------
void TLCore::Platform::Sleep(u32 Millisecs)
{
	float SleepSecs = Millisecs / 1000.f;
	[NSThread sleepForTimeInterval:SleepSecs];
}







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



// Ipod Shutdown
- (void)applicationWillTerminate:(UIApplication *)application 
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


