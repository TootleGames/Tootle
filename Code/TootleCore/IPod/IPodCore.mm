#import "IPodCore.h"
#import "IPodApp.h"

#include "../TLCore.h"
#include "../TLTypes.h"
#include "../TString.h"
#include "../TCoreManager.h"

#import <UIKit/UIKit.h>


namespace TLCore
{
	namespace Platform
	{
		TString						g_AppExe;
	}
	
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
	TPtr<TLMessaging::TMessage> pMessage = new TLMessaging::TMessage("Quit");
	
	if(pMessage.IsValid())
	{
		TLCore::g_pCoreManager->QueueMessage(pMessage);
	}
	else
		TLDebug_Break("Unable to send quit message");
	
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
	
	//	gr: kernal debugger... but is there one for the iphone?
	//Debugger();
	
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






void* TLMemory::Platform::MemAlloc(u32 Size)								
{	
	return malloc( Size );
}		


void TLMemory::Platform::MemDealloc(void* pMem)							
{
	free( pMem );	
}


void TLMemory::Platform::MemCopy(void* pDest,const void* pSrc,u32 Size)	
{
	memcpy( pDest, pSrc, Size );	
}


void TLMemory::Platform::MemMove(void* pDest,const void* pSrc,u32 Size)	
{
	memmove( pDest, pSrc, Size );
}

