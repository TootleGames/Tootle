#pragma once

#import <UIKit/UIKit.h>
#import <TootleRender/Ipod/IPodView.h>

//	g++ defines __thumb__ 1 when building thumb code...
#if __thumb__
	#warning Compiling in Thumb mode
#endif

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



