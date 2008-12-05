#pragma once




#import <UIKit/UIKit.h>

@class EAGLView;

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



