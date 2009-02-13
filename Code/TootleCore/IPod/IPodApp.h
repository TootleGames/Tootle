#pragma once

#import <UIKit/UIKit.h>
#import <TootleRender/Ipod/IPodView.h>



@interface OpenglESAppAppDelegate : NSObject <UIAccelerometerDelegate> {
//@interface OpenglESAppAppDelegate : NSObject <UIApplicationDelegate> {
    UIWindow*	window;
    EAGLView*	glView;
	BOOL		init;
	BOOL		shutdown;
	NSTimer*	timer;
}

- (void) onTimer:(NSTimer*)timer;

@property (nonatomic, retain) IBOutlet UIWindow *window;
@property (nonatomic, retain) IBOutlet EAGLView *glView;

@end



namespace TLCore
{
	namespace Platform
	{
		extern OpenglESAppAppDelegate* g_pIPodApp;
	}
}



