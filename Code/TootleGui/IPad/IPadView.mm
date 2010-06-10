/*
 *  MacView.mm
 *  TootleRender
 *
 *  Created by Duane Bradbury on 08/09/2009.
 *  Copyright 2009 Tootle. All rights reserved.
 *
 */

#include "IPadView.h"

#include <TootleCore/TLCore.h>
#include <TootleCore/TString.h>

#include <OpenGL/gl.h>
#include <OpenGL/glext.h>

//	gr: once the opengl context has been created we need to initialise opengl
//	this will move to the rasteriser, but will still need to be called from here.
#include <TootleRender/TLRender.h>


#define USE_DEPTH_BUFFER 1


@implementation TootleOpenGLView

@synthesize viewRenderbuffer;
@synthesize viewFramebuffer;
@synthesize depthRenderbuffer;
@synthesize backingWidth;
@synthesize backingHeight;
@synthesize bSendImage;



//The GL view is stored in the nib file. When it's unarchived it's sent -initWithCoder:
- (id)initWithCoder:(NSCoder*)coder {
    
    if ((self = [super initWithCoder:coder])) 
	{
		/*
        // Get the layer
        CAEAGLLayer *eaglLayer = (CAEAGLLayer *)self.layer;
        
        eaglLayer.opaque = YES;
        eaglLayer.drawableProperties = [NSDictionary dictionaryWithObjectsAndKeys:
                                        [NSNumber numberWithBool:NO], kEAGLDrawablePropertyRetainedBacking, kEAGLColorFormatRGBA8, kEAGLDrawablePropertyColorFormat, nil];
        
        context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGL];
        
        if (!context || ![EAGLContext setCurrentContext:context]) {
            [self release];
            return nil;
        }
		*/
	}
	
	self.bSendImage = FALSE;
    return self;
}


- (id) initWithFrame:(NSRect)frame
{
	//return [self initWithFrame:frame pixelFormat:GL_RGB565 depthFormat:0 preserveBackbuffer:NO];
	return [self initWithFrame:frame pixelFormat:0 depthFormat:0 preserveBackbuffer:NO];
}

- (id) initWithFrame:(NSRect)frame pixelFormat:(GLuint)format 
{
	return [self initWithFrame:frame pixelFormat:format depthFormat:0 preserveBackbuffer:NO];
}

- (id) initWithFrame:(NSRect)frame pixelFormat:(GLuint)format depthFormat:(GLuint)depth preserveBackbuffer:(BOOL)retained
{
	NSOpenGLPixelFormatAttribute attribs[] = {
		//NSOpenGLPFAWindow,
		
		// Specifying "NoRecovery" gives us a context that cannot fall back to the software renderer.  This makes the View-based context a compatible with the fullscreen context, enabling us to use the "shareContext" feature to share textures, display lists, and other OpenGL objects between the two.
        NSOpenGLPFANoRecovery,
		
        // Attributes Common to FullScreen and non-FullScreen
        NSOpenGLPFAColorSize, 24,
        NSOpenGLPFADepthSize, 16,

		NSOpenGLPFAAccelerated,
		NSOpenGLPFADoubleBuffer,
		//NSOpenGLPFASingleRenderer,
		
		(NSOpenGLPixelFormatAttribute)0
	};
	
	//NSOpenGLPixelFormat *pixelformat = [[self class] defaultPixelFormat ];
	NSOpenGLPixelFormat *pixelformat = [[NSOpenGLPixelFormat alloc] initWithAttributes:attribs];
	
	self = [super initWithFrame:frame pixelFormat:pixelformat];
	if(!self) 
	{
		[pixelformat release];
		return nil;
	}
	
		/*
		CAEAGLLayer*			eaglLayer = (NSOpenGLLayer*)[self layer];
		
		eaglLayer.opaque = YES;
		eaglLayer.drawableProperties = [NSDictionary dictionaryWithObjectsAndKeys:
										[NSNumber numberWithBool:NO], kEAGLDrawablePropertyRetainedBacking, kEAGLColorFormatRGBA8, kEAGLDrawablePropertyColorFormat, nil];
		
		//_format = format;
		//_depthFormat = depth;
		
		
		 
		 */
				

	
/*
 if(![self createFramebuffer]) 
 {
		[self release];
		return nil;
 }
*/
	
	self.bSendImage = FALSE;
	[pixelformat release];

	return self;
}


-(void)initOpenGLContext
{
	// Setup the OpenGL render context
	NSOpenGLContext* context = [[NSOpenGLContext alloc] initWithFormat:[super pixelFormat]  shareContext:nil];
	
	[self setOpenGLContext:context];
	[context setView:self];
	
	[context makeCurrentContext];
	[self prepareOpenGL];

	//	gr: release?? is this a smart pointer? looks ominous to me
	[context release];

	//	context has been initialised (successfully?) so init opengl
	TLRender::Opengl::Init();
}


- (void)layoutSubviews 
{
	//  [EAGLContext setCurrentContext:context];
	//  [self destroyFramebuffer];
	//  [self createFramebuffer];
}


- (BOOL)createFramebuffer {
    
	TLDebug_Print("Creating frame buffer");
	
    glGenFramebuffersEXT(1, &viewFramebuffer);
    glGenRenderbuffersEXT(1, &viewRenderbuffer);
    
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, viewFramebuffer);
    glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, viewRenderbuffer);
	
	// Note: should be window width & height?
	glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_RGBA, 1024, 1024);
    //[[self openGLContext] renderbufferStorage:GL_RENDERBUFFER_EXT fromDrawable:(CAEAGLLayer*)self.layer];

	//GLuint maxbuffers;
	//glGetIntergeriEXT(GL_MAX_COLOR_ATTACHMENTS_EXT, &maxbuffers);

	glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_RENDERBUFFER_EXT, viewRenderbuffer);
    
	
    glGetRenderbufferParameterivEXT(GL_RENDERBUFFER_EXT, GL_RENDERBUFFER_WIDTH_EXT, &backingWidth);
    glGetRenderbufferParameterivEXT(GL_RENDERBUFFER_EXT, GL_RENDERBUFFER_HEIGHT_EXT, &backingHeight);
    
	
    if (USE_DEPTH_BUFFER)
	{
        glGenRenderbuffersEXT(1, &depthRenderbuffer);
        glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, depthRenderbuffer);
        glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT16, backingWidth, backingHeight);
        glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, depthRenderbuffer);
    }

    
    if(glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT) != GL_FRAMEBUFFER_COMPLETE_EXT) 
	{
		TLDebug_Print("Failed to create frame buffer");
		
        NSLog(@"failed to make complete framebuffer object %x", glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT));
        return NO;
    }
	
	TLDebug_Print("Frame buffer created successfully");
	
    return YES;
}


- (void)destroyFramebuffer {
    
	if ( viewFramebuffer != 0 )
	{
		glDeleteFramebuffersEXT(1, &viewFramebuffer);
		viewFramebuffer = 0;
	}
	
	if ( viewRenderbuffer != 0 )
	{
		glDeleteRenderbuffersEXT(1, &viewRenderbuffer);
		viewRenderbuffer = 0;
	}
    
    if ( depthRenderbuffer != 0 ) 
	{
        glDeleteRenderbuffersEXT(1, &depthRenderbuffer);
        depthRenderbuffer = 0;
    }
}



- (void)dealloc {
	
    if ([NSOpenGLContext currentContext] == [self openGLContext])	
    {
        [NSOpenGLContext clearCurrentContext];
    }
    
    [self clearGLContext];  
    [super dealloc];
}



// Pass on the touch began event to the input system for handling
- (void)touchesBegan:(NSSet*)touches withEvent:(NSEvent*)event {
	
}


// Pass on the touch moved event to the input system for handling
- (void)touchesMoved:(NSSet *)touches withEvent:(NSEvent *)event 
{ 
}


// Pass on the touch end event to the input system for handling
- (void)touchesEnded:(NSSet*)touches withEvent:(NSEvent*)event {
}

// Pass on the touch cancelled event to the input system for handling
- (void)touchesCancelled:(NSSet*)touches withEvent:(NSEvent*)event {
}





////////////////////////////////////////////////////////
// Take screenshot of OpenGL view
// Code from http://www.bit-101.com/blog/?p=1861
//
// Other possible useful sources:
// http://discussions.apple.com/thread.jspa?messageID=8443971
// http://discussions.apple.com/thread.jspa?messageID=8818069&#8818069
////////////////////////////////////////////////////////




// callback for CGDataProviderCreateWithData
void releaseData(void *info, const void *data, size_t dataSize) 
{
	//NSLog(@”releaseData\n”);
	free((void*)data); // free the
}

// callback for UIImageWriteToSavedPhotosAlbum
- (void)image:(NSImage *)image didFinishSavingWithError:(NSError *)error contextInfo:(void *)contextInfo {
	
	// if flagged to send the image then save it to a temp 'cache' file and
	// attach it to an email. Means an extra file is used but you can't get the
	// filename of the image saved in the photo library
	if(self.bSendImage)
	{
		[self saveImageToFileAndSetupEmail :image];
	}
	
	//NSLog(@”Save finished\n”);
	[image release]; // release image
}

-(void)saveImageToFileAndSetupEmail:(NSImage*)image
{
	/*
	NSString* filename = [self saveImageToFile :image];
	
	TString tsfilename = [filename UTF8String];
	// Now setup an email using the image details
	//TString urlstr = "http://www.google.com";
	
	// TEST email address
#ifdef USE_TEST_EMAIL 
	TString urlstr = "mailto:duane@dibely.com";
#else	
	TString urlstr = "mailto:duane@dibely.com?subject=Be My Valentine&body=Roses are Red&attachment=";
	urlstr.Append(tsfilename);
#endif
	
	TLCore::Platform::OpenWebURL(urlstr);
	*/
}


-(void)saveViewToPhotoLibrary
{
	NSImage *image = [self createImageFromView];
	
	[self saveImageToPhotoLibrary :image];	
}

-(void)saveViewToPhotoLibraryAndSetupEmail
{
	self.bSendImage = TRUE;
	[self saveViewToPhotoLibrary];
}



-(void)saveImageToPhotoLibrary:(NSImage*) image
{
	//NSImageWriteToSavedPhotosAlbum(image, self, (SEL)@selector(image:didFinishSavingWithError:contextInfo:), nil); // add callback for finish saving
}

-(NSString*) saveViewToFile
{
	NSImage *image = [self createImageFromView];
	
	return [self saveImageToFile: image];
}

-(NSString*) saveImageToFile:(NSImage*)image;
{
	return NULL;
	/*
	// Get the tmp directory location
	NSString *tmpdir = NSTemporaryDirectory();
	
	// Create the final filename by appending the fixed name
	NSString *filename = [tmpdir stringByAppendingString: @"tmpimage.jpg"];
	
	// Get the JPG from image
	NSData* data = NSImagePNGRepresentation(image);
	
	// Save to the file
	Bool success = [data writeToFile:filename atomically:false];
	
	if(success)
	{
		TLDebug_Print("Image saved to file");
	}
	
	// Cleanup
	//[data release];
	//[tmpdir release];
	
	return filename;
	 */
}


-(NSImage*)createImageFromView 
{
	NSRect rect = [[NSScreen mainScreen] frame];
	int width = rect.size.width;
	int height = rect.size.height;
	
	NSInteger myDataLength = width * height * 4;
	GLubyte *buffer = (GLubyte *) malloc(myDataLength);
	GLubyte *buffer2 = (GLubyte *) malloc(myDataLength);
	glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
	for(int y = 0; y <height; y++) 
	{
		for(int x = 0; x <width * 4; x++) 
		{
			buffer2[int((height - 1 - y) * width * 4 + x)] = buffer[int(y * 4 * width + x)];
		}
	}
	free(buffer); // YOU CAN FREE THIS NOW
	
	CGDataProviderRef provider = CGDataProviderCreateWithData(NULL, buffer2, myDataLength, releaseData);
	int bitsPerComponent = 8;
	int bitsPerPixel = 32;
	int bytesPerRow = 4 * width;
	CGColorSpaceRef colorSpaceRef = CGColorSpaceCreateDeviceRGB();
	CGBitmapInfo bitmapInfo = kCGBitmapByteOrderDefault;
	CGColorRenderingIntent renderingIntent = kCGRenderingIntentDefault;
	CGImageRef imageRef = CGImageCreate(width, height, bitsPerComponent, bitsPerPixel, bytesPerRow, colorSpaceRef, bitmapInfo, provider, NULL, NO, renderingIntent);
	
	CGColorSpaceRelease(colorSpaceRef); // YOU CAN RELEASE THIS NOW
	CGDataProviderRelease(provider); // YOU CAN RELEASE THIS NOW
	
	NSImage *image = [[NSImage alloc] initWithCGImage:imageRef]; // change this to manual alloc/init instead of autorelease
	CGImageRelease(imageRef); // YOU CAN RELEASE THIS NOW
	
	return image;
}
////////////////////////////////////////////////////////

/*
- (BOOL) acceptsFirstResponder
{
    // We want this view to be able to receive key events.
    return YES;
}

- (void)mouseDown:(NSEvent *)theEvent
{
	TLDebug_Break("Mouse event occured");
}


- (void)mouseDragged:(NSEvent *)theEvent
{
	TLDebug_Break("Mouse event occured");

}

- (void)mouseUp:(NSEvent *)theEvent
{	
	TLDebug_Break("Mouse event occured");

}

- (void)mouseEntered:(NSEvent *)theEvent
{
	TLDebug_Break("Mouse event occured");

}

- (void)mouseExited:(NSEvent *)theEvent
{
	TLDebug_Break("Mouse event occured");

}

- (void)keyUp:(NSEvent *)theEvent
{
	TLDebug_Break("Keyboard event occured");
}

- (void)keyDown:(NSEvent *)theEvent
{
	TLDebug_Break("Keyboard event occured");
}
 */


@end
