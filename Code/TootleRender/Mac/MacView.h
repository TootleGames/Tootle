/*
 *  MacView.h
 *  TootleRender
 *
 *  Created by Duane Bradbury on 08/09/2009.
 *  Copyright 2009 Tootle. All rights reserved.
 *
 */

#pragma once

#include <TootleCore/TLTypes.h>

#import <Cocoa/Cocoa.h>
#import <QuartzCore/QuartzCore.h>
#import <OpenGL/glext.h>


/*
 This class wraps the CAEAGLLayer from CoreAnimation into a convenient UIView subclass.
 The view content is basically an EAGL surface you render your OpenGL scene into.
 Note that setting the view non-opaque will only work if the EAGL surface has an alpha channel.
 */
@interface TootleOpenGLView : NSOpenGLView {
    
@private
    /* The pixel dimensions of the backbuffer */
    GLint backingWidth;
    GLint backingHeight;
        
    /* OpenGL names for the renderbuffer and framebuffers used to render to this view */
    GLuint viewRenderbuffer, viewFramebuffer;
    
    /* OpenGL name for the depth buffer that is attached to viewFramebuffer, if it exists (0 if it does not exist) */
    GLuint depthRenderbuffer;
	
	// Send image flag for auto emailing of images
	Bool	bSendImage;
}

@property GLuint viewRenderbuffer;
@property GLuint viewFramebuffer;
@property GLuint depthRenderbuffer;
@property GLint backingWidth;
@property GLint backingHeight;
@property Bool	bSendImage;

- (id) initWithFrame:(NSRect)frame; //These also set the current context
- (id) initWithFrame:(NSRect)frame pixelFormat:(GLuint)format;
- (id) initWithFrame:(NSRect)frame pixelFormat:(GLuint)format depthFormat:(GLuint)depth preserveBackbuffer:(BOOL)retained;

-(void)initOpenGLContext;

- (BOOL) createFramebuffer;
- (void) destroyFramebuffer;

- (NSImage *)createImageFromView;
- (void)saveViewToPhotoLibrary;
- (void)saveViewToPhotoLibraryAndSetupEmail;
- (void)saveImageToPhotoLibrary:(NSImage*) image;
- (NSString*)saveViewToFile;
- (NSString*)saveImageToFile:(NSImage*)image;
- (void)saveImageToFileAndSetupEmail:(NSImage*)image;


- (void)image:(NSImage *)image didFinishSavingWithError:(NSError *)error contextInfo:(void *)contextInfo;

/*
// Input events
- (void)mouseDown:(NSEvent *)theEvent;
- (void)mouseDragged:(NSEvent *)theEvent;
- (void)mouseUp:(NSEvent *)theEvent;

- (void)mouseEntered:(NSEvent *)theEvent;
- (void)mouseExited:(NSEvent *)theEvent;

- (void)keyUp:(NSEvent *)theEvent;
- (void)keyDown:(NSEvent *)theEvent;
*/

@end
