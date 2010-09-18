#import "IPodCore.h"
#import "IPodDebug.h"
#import "IPodString.h"

#include "../TLCore.h"
#include "../TLTypes.h"
#include "../TString.h"
#include "../TCoreManager.h"
#include "../TLTime.h"


//	for the app code
#import <TootleAsset/TAsset.h>
#import <TootleInput/Ipod/IPodInput.h>
#import <QuartzCore/QuartzCore.h>
//#import <OpenGLES/EAGLDrawable.h>	//	gr: work out what uses this and move it!

#import <UIKit/UIDevice.h>

#import <IPodAlert.h>

//	Constant for the number of times per second (Hertz) to sample acceleration.
#define ACCELEROMETER_FREQUENCY     30


namespace TLCore
{
	void RegisterManagers_Engine(TPtr<TCoreManager>& pCoreManager);
	void RegisterManagers_Game(TPtr<TCoreManager>& pCoreManager);

	extern TPtr<TCoreManager>		g_pCoreManager;	
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
	Data.ExportData("CPUCount", 1);	

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




void TLCore::Platform::OpenWebURL(TString& urlstr)
{
	NSString* NSAddress = TLString::ConvertToUnicharString(urlstr);
	
	NSURL* url = [NSURL URLWithString: NSAddress];
	[[UIApplication sharedApplication] openURL:url];
	
	//[[UIApplication sharedApplication] openURL:[NSURL URLWithString: @"http://www.google.co.uk"]];
	
	[NSAddress release];

}


//--------------------------------------------------
//	platform thread/process sleep
//--------------------------------------------------
void TLCore::Platform::Sleep(u32 Millisecs)
{
	float SleepSecs = Millisecs / 1000.f;
	[NSThread sleepForTimeInterval:SleepSecs];
}




