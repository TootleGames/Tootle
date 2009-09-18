#pragma once


#import <Cocoa/Cocoa.h>

#include <TootleRender/Mac/MacView.h> // dependancy on the TootleOpenGLView
#include <TootleRender/Mac/MacWindowDelegate.h> // dependancy on the TootleWindowDelegate


@interface TootleNSApplicationDelegate : NSObject
{
	//NSWindow*	window;
    TootleOpenGLView*	glView;
	BOOL		init;
	BOOL		shutdown;
	NSTimer*	timer;
}
- (void) onTimer:(NSTimer*)timer;
//-(void) applicationDidFinishLaunching:(NSNotification *)notification;

//@property (nonatomic, retain) IBOutlet NSWindow *window;
@property (nonatomic, retain) IBOutlet TootleOpenGLView *glView;

@end



