/*
 *  MacWindowDelegate.h
 *  TootleRender
 *
 *  Created by Duane Bradbury on 14/09/2009.
 *  Copyright 2009 Tootle. All rights reserved.
 *
 */
#pragma once

#import <Appkit/NSWindow.h>

@interface TootleWindowDelegate : NSObject
{
	@private
	NSWindow* m_pWindow;
}

-(void) setWindow:(NSWindow*)window;


@end
