#include "../TLTime.h"
#include "../TCoreManager.h"
#import "IPodApp.h"
#include "../TLCore.h"
#include "../TPtr.h"


OpenglESAppAppDelegate* TLCore::Platform::g_pIPodApp = NULL;








void TLCore::Platform::OpenWebURL(TString& urlstr)
{
	NSString* NSAddress = [[NSString alloc] initWithCString:urlstr.GetData() ];
	NSURL* url = [NSURL URLWithString: NSAddress];
	[[UIApplication sharedApplication] openURL:url];
	
	//[[UIApplication sharedApplication] openURL:[NSURL URLWithString: @"http://www.google.co.uk"]];
	
	[NSAddress release];

}










