/*
 *  MacWindowDelegate.mm
 *  TootleRender
 *
 *  Created by Duane Bradbury on 14/09/2009.
 *  Copyright 2009 Tootle. All rights reserved.
 *
 */

#include "MacWindowDelegate.h"

#import <Cocoa/Cocoa.h>

@implementation TootleWindowDelegate

- (void)windowWillClose:(NSNotification *)notification
{
	// Tell the app to exit
	NSApplication* app = [NSApplication sharedApplication];
	
	// Send terminate message to the application
	SEL selector = @selector( terminate: );
	
	if ( [app respondsToSelector:selector] )
	{
		[app performSelector:selector];
	}		

	if(m_pWindow)
		[m_pWindow setDelegate:nil];
	
	// release delegate
	[self release];
}

-(void) setWindow:(NSWindow*)window
{
	m_pWindow = window;
}



@end
