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

	
	[NSApplication sharedApplication];
	[NSApp setDelegate: [TootleNSApplicationDelegate new]];
	
	int retVal = NSApplicationMain(argc,  (const char **) argv);
	return retVal;

/*	
	[TootleNSApplication sharedApplication];
	
    [NSBundle loadNibNamed:@"MainMenu" owner:NSApp];
	
    [NSApp run];
	
	return 0;
*/
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



