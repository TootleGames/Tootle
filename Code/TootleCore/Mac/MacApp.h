#pragma once

/*
 [19/02/09] DB - Removed from Mac build

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
*/

#import <Cocoa/Cocoa.h>


@interface TootleOpenGLView : NSOpenGLView
{
	
}

- (void) drawRect: (NSRect) bounds;

@end


@interface TootleNSApplicationDelegate : NSObject
{
	NSWindow*	window;
    TootleOpenGLView*	glView;
	BOOL		init;
	BOOL		shutdown;
	NSTimer*	timer;
}
- (void) onTimer:(NSTimer*)timer;
//-(void) applicationDidFinishLaunching:(NSNotification *)notification;


@property (nonatomic, retain) IBOutlet NSWindow *window;
@property (nonatomic, retain) IBOutlet TootleOpenGLView *glView;

@end



