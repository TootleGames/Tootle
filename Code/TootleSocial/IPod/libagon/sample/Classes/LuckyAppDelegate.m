//  LuckyAppDelegate.m

//  Copyright Aptocore ApS 2009. All rights reserved.


#import "LuckyAppDelegate.h"
#import "LuckyViewController.h"
#import "AGON.h"

@implementation LuckyAppDelegate

@synthesize window;
@synthesize viewController;

- (void)applicationDidFinishLaunching:(UIApplication *)application {  
	// Enable logging from inside of AGON.
	AgonShowLogs(YES);
	
	// Initialize AGON with application secret key. All other 
	// information is stored inside the AgonPackage.bundle.
	
	

	AgonCreate(@"D60D14380A8FD445B53E40E0F11B0E6A9F502493"); // App secret that matches the DevDB environment.
	//AgonCreate(@"4C36675F28439D39E3F988E0FF3113829D74433C"); // App secret that matches the TestDB environment.
	
	
	// Enable retain count asserts inside libagon. You should not enable this
	// if your game uses a custom event loop (e.g. using the new event loop
	// structure in current Unity beta release). This only has any effect in
	// debug builds, since asserts are not enabled in release builds.
	AgonEnableRetainCountAsserts(YES);
	
	window = [[UIWindow alloc] initWithFrame:[UIScreen mainScreen].bounds];
	viewController = [[LuckyViewController alloc] init];
	[window addSubview:viewController.view];
	[window makeKeyAndVisible];
	
	// Tint the AGON views with colors matching our application 
	//AgonSetStartBackgroundTint([UIColor colorWithRed:0.3 green:0.4 blue:0.3 alpha:1]);
	//AgonSetEndBackgroundTint([UIColor colorWithRed:0.5 green:0.5 blue:0.5 alpha:1]);

	// Green
	//AgonSetStartBackgroundTint([UIColor colorWithRed:80/255.0 green:115/255.0 blue:10/255.0 alpha:1]);
	//AgonSetEndBackgroundTint([UIColor colorWithRed:0/255.0 green:40/255.0 blue:0/255.0 alpha:1]);
	
	// Red/purple
	//AgonSetStartBackgroundTint([UIColor colorWithRed:90/255.0 green:20/255.0 blue:90/255.0 alpha:1]);
	//AgonSetEndBackgroundTint([UIColor colorWithRed:180/255.0 green:0/255.0 blue:40/255.0 alpha:1]);

	// Grey/Blue
	AgonSetStartBackgroundTint([UIColor colorWithRed:118/255.0 green:118/255.0 blue:118/255.0 alpha:1]);
	AgonSetEndBackgroundTint([UIColor colorWithRed:121/255.0 green:163/255.0 blue:164/255.0 alpha:1]);
	
}

- (void)applicationWillTerminate:(UIApplication *)application {
	// Free AGON resources.
	AgonDestroy();
	[viewController release];
	[window release];
}

- (void)dealloc {
	[super dealloc];
}


@end
