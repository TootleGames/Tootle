#include "IPodOpenglCanvas.h"
#include "IPodWindow.h"

//	mac cocoa includes, NSApp, NSArray etc
#include <TootleInput/IPod/IPodInput.h>
#include <TootleCore/TLMemory.h> // TEMP
#import "IPodWindowDelegate.h"

//	gr: once the opengl context has been created we need to initialise opengl
//	this will move to the rasteriser, but will still need to be called from here.
#include <TootleRender/TLRender.h>

//	keep a list of touch associated refs
TKeyArray<UITouch*,TRef> g_TouchTable;

//	get the ref for this touch, or create a new one if it's new touch data
TRef GetTouchRef(UITouch* pTouch)
{
	//	find existing 
	const TRef* pExistingRef = g_TouchTable.Find( pTouch );
	if ( !pExistingRef )
	{
		TLDebug_Print("No touch ref allocated");
		return TRef();
	}

	return *pExistingRef;
}

TRef AllocTouchRef(UITouch* pTouch)
{
	//	assert if it already exists (must not have been free'd)
	const TRef* pExistingRef = g_TouchTable.Find( pTouch );
	if ( pExistingRef )
	{
		TLDebug_Break("Touch ref for this touch already exists");
		return *pExistingRef;
	}
	
	//	make up a new ref
	//	return TLRef::GetValidTRef( (u32)pTouch );
	static TRef g_UniqueRef;
	g_UniqueRef.Increment();
	g_TouchTable.Add( pTouch, g_UniqueRef );
	return g_UniqueRef;
}

void FreeTouchRef(UITouch* pTouch)
{
	//	remove the existing key
	g_TouchTable.Remove( pTouch );
}


//------------------------------------------------------
//	
//------------------------------------------------------
TPtr<TLGui::TOpenglCanvas> TLGui::CreateOpenglCanvas(TWindow& Parent,TRefRef Ref)
{
	TPtr<TLGui::TOpenglCanvas> pControl = new TLGui::Platform::OpenglCanvas( Parent, Ref );
	return pControl;
}



TLGui::Platform::OpenglCanvas::OpenglCanvas(TLGui::TWindow& Parent,TRefRef ControlRef) :
	TLGui::TOpenglCanvas	( ControlRef ),
	m_pView					( NULL )
{
	//Create the OpenGL drawing view and add it to the window programatically
	// NOTE: Only needed if the window is not setup in a project .nib file

	//	gr: we want the canvas to be the size of the client area of the window
	int2 Size = Parent.GetSize();
	CGRect CanvasRect;
	CanvasRect.origin.x = 0;
	CanvasRect.origin.y = 0;
	CanvasRect.size.width = Size.x;
	CanvasRect.size.height = Size.y;
	
	m_pView = [[IPodGLView alloc] initWithFrame:CanvasRect]; // - kPaletteHeight
	if ( !m_pView )
	{
		TLDebug_Break("Failed to alloc EAGLView");
		return;
	}
	
	//	get NSWindow 
	Platform::Window& PlatformWindow = static_cast<Platform::Window&>( Parent );
	UIWindow* pWindow = PlatformWindow.m_pWindow;

	//	set the view on the parent window
	[pWindow addSubview:m_pView];

	// Now we are part of the window we can setup the opengl context
	// NOTE: Has to be done AFTER the view is added to the window
	//	gr: this has gone... did I delete it?
//	[m_pView initOpenGLContext];
	
	//	context has been initialised (successfully?) so init opengl	
	//	todo:
}


//------------------------------------------------------
//	set as current opengl canvas. If this fails we cannot draw to it
//------------------------------------------------------
Bool TLGui::Platform::OpenglCanvas::BeginRender()
{
	//	not initialised properly, don't draw
	if ( !m_pView )
		return false;

	//	set current context
	[EAGLContext setCurrentContext:m_pView.context];
	glBindFramebufferOES(GL_FRAMEBUFFER_OES, m_pView.viewFramebuffer);

	return true;
}

//------------------------------------------------------
//	flip buffer at end of render
//------------------------------------------------------
void TLGui::Platform::OpenglCanvas::EndRender()
{
	//	flip buffers
	if ( !m_pView )
	{
		TLDebug_Break("No view to flip, should have failed BeginRender");
		return;
	}
	
	//	gr: flush flips on the mac...
	EAGLContext* context = [m_pView context];
	//[context flushBuffer];
    glBindRenderbufferOES(GL_RENDERBUFFER_OES, m_pView.viewRenderbuffer);
    [context presentRenderbuffer:GL_RENDERBUFFER_OES];

	/*
	// Take a screenshot if flagged to do so.  
	// I'm not sure this is the right place but...
	if(GetFlag(Flag_TakeScreenshot))
	{
		[glView saveViewToPhotoLibrary];
		//[glView saveViewToPhotoLibraryAndSetupEmail];
		m_Flags.Set(Flag_TakeScreenshot, FALSE);
	}
*/
}


//------------------------------------------------------
//	canvas(renderable area) size
//------------------------------------------------------
Type2<u16> TLGui::Platform::OpenglCanvas::GetSize() const
{
	if ( !m_pView )
		return Type2<u16>(0,0);
	
	CGRect ViewFrame = [m_pView frame];
	return Type2<u16>( ViewFrame.size.width, ViewFrame.size.height );
}




#define USE_DEPTH_BUFFER 1



@implementation IPodGLView

@synthesize context;
@synthesize viewRenderbuffer;
@synthesize viewFramebuffer;
@synthesize depthRenderbuffer;
@synthesize backingWidth;
@synthesize backingHeight;
@synthesize bSendImage;


// You must implement this method
+ (Class)layerClass {
    return [CAEAGLLayer class];
}


//The GL view is stored in the nib file. When it's unarchived it's sent -initWithCoder:
- (id)initWithCoder:(NSCoder*)coder {
    
    if ((self = [super initWithCoder:coder])) {
        // Get the layer
        CAEAGLLayer *eaglLayer = (CAEAGLLayer *)self.layer;
        
        eaglLayer.opaque = YES;
        eaglLayer.drawableProperties = [NSDictionary dictionaryWithObjectsAndKeys:
                                        [NSNumber numberWithBool:NO], kEAGLDrawablePropertyRetainedBacking, kEAGLColorFormatRGBA8, kEAGLDrawablePropertyColorFormat, nil];
        
        context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES1];
        
        if (!context || ![EAGLContext setCurrentContext:context]) {
            [self release];
            return nil;
        }
	}
	
	// Enable multitouch support
	[self setMultipleTouchEnabled:YES];
	
	self.bSendImage = FALSE;
    return self;
}


- (id) initWithFrame:(CGRect)frame
{
	return [self initWithFrame:frame pixelFormat:GL_RGB565_OES depthFormat:0 preserveBackbuffer:NO];
}

- (id) initWithFrame:(CGRect)frame pixelFormat:(GLuint)format 
{
	return [self initWithFrame:frame pixelFormat:format depthFormat:0 preserveBackbuffer:NO];
}

- (id) initWithFrame:(CGRect)frame pixelFormat:(GLuint)format depthFormat:(GLuint)depth preserveBackbuffer:(BOOL)retained
{
	if((self = [super initWithFrame:frame])) {
		CAEAGLLayer*			eaglLayer = (CAEAGLLayer*)[self layer];
		
		eaglLayer.opaque = YES;
		eaglLayer.drawableProperties = [NSDictionary dictionaryWithObjectsAndKeys:
										[NSNumber numberWithBool:NO], kEAGLDrawablePropertyRetainedBacking, kEAGLColorFormatRGBA8, kEAGLDrawablePropertyColorFormat, nil];
		
		//_format = format;
		//_depthFormat = depth;
		
		context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES1];
        if (!context || ![EAGLContext setCurrentContext:context]) {
            [self release];
            return nil;
		}
		
		if(![self createFramebuffer]) {
			[self release];
			return nil;
		}
	}
	
	// Enable multitouch support
	[self setMultipleTouchEnabled:YES];
	
	self.bSendImage = FALSE;
	
	return self;
}


- (void)layoutSubviews 
{
	//  [EAGLContext setCurrentContext:context];
	//  [self destroyFramebuffer];
	//  [self createFramebuffer];
}


- (BOOL)createFramebuffer {
    
	TLDebug_Print("Creating frame buffer");
	
    glGenFramebuffersOES(1, &viewFramebuffer);
    glGenRenderbuffersOES(1, &viewRenderbuffer);
    
    glBindFramebufferOES(GL_FRAMEBUFFER_OES, viewFramebuffer);
    glBindRenderbufferOES(GL_RENDERBUFFER_OES, viewRenderbuffer);
    [context renderbufferStorage:GL_RENDERBUFFER_OES fromDrawable:(CAEAGLLayer*)self.layer];
    glFramebufferRenderbufferOES(GL_FRAMEBUFFER_OES, GL_COLOR_ATTACHMENT0_OES, GL_RENDERBUFFER_OES, viewRenderbuffer);
    
    glGetRenderbufferParameterivOES(GL_RENDERBUFFER_OES, GL_RENDERBUFFER_WIDTH_OES, &backingWidth);
    glGetRenderbufferParameterivOES(GL_RENDERBUFFER_OES, GL_RENDERBUFFER_HEIGHT_OES, &backingHeight);
    
    if (USE_DEPTH_BUFFER)
	{
        glGenRenderbuffersOES(1, &depthRenderbuffer);
        glBindRenderbufferOES(GL_RENDERBUFFER_OES, depthRenderbuffer);
        glRenderbufferStorageOES(GL_RENDERBUFFER_OES, GL_DEPTH_COMPONENT16_OES, backingWidth, backingHeight);
        glFramebufferRenderbufferOES(GL_FRAMEBUFFER_OES, GL_DEPTH_ATTACHMENT_OES, GL_RENDERBUFFER_OES, depthRenderbuffer);
    }
    
    if(glCheckFramebufferStatusOES(GL_FRAMEBUFFER_OES) != GL_FRAMEBUFFER_COMPLETE_OES) 
	{
		TLDebug_Break("Failed to create frame buffer");
		
        NSLog(@"failed to make complete framebuffer object %x", glCheckFramebufferStatusOES(GL_FRAMEBUFFER_OES));
        return NO;
    }
	
	TLDebug_Print("Frame buffer created successfully");
	
    return YES;
}


- (void)destroyFramebuffer {
    
	if ( viewFramebuffer != 0 )
	{
		glDeleteFramebuffersOES(1, &viewFramebuffer);
		viewFramebuffer = 0;
	}
	
	if ( viewRenderbuffer != 0 )
	{
		glDeleteRenderbuffersOES(1, &viewRenderbuffer);
		viewRenderbuffer = 0;
	}
    
    if ( depthRenderbuffer != 0 ) 
	{
        glDeleteRenderbuffersOES(1, &depthRenderbuffer);
        depthRenderbuffer = 0;
    }
}



- (void)dealloc {
	
    if ([EAGLContext currentContext] == context) {
        [EAGLContext setCurrentContext:nil];
    }
    
    [context release];  
    [super dealloc];
}



// Pass on the touch began event to the input system for handling
- (void)touchesBegan:(NSSet*)touches withEvent:(UIEvent*)event {
	
	// Essentially we copy the data from each touch event into our own custom object and pass it onto the input system
	
	// The 'touch' object is unique and persistent whilst a finger is touching the screen
	// so we can use that to determine an index for the touch object
	
	// [13/03/09] DB - OK the alltouches set will give us literally that - all touch objects not 
	// just the ones that triggered the event.  So as we only want the ones that triggered the event
	// we use the touches set instead.  if we use the alltouches set we need to check the touchphase
	// otherwise we could send a touch object that is not ending as if it was ended.
    //NSSet *alltouches = [event allTouches];
	//for (UITouch *touch in alltouches)
	
    for (UITouch *touch in touches)
    {
		CGPoint currentTouchPosition = [touch locationInView:self]; 
		CGPoint previousTouchPosition = [touch previousLocationInView:self]; 
		
#pragma message("todo: change this code. increment a global, or something. I've had this assert if I tap the screen enough because of dupe refs")
		// Use the address of the ipod touch obejct for the ID masked to ensure it's valid
		TRef TouchRef = AllocTouchRef( touch );
		/*
		 #ifdef _DEBUG
		 TString str;
		 str.Appendf("Event Touch Begin: %d, (%d)", TouchRef.GetData(), touch );
		 TLDebug_Print(str);	
		 #endif
		 */
		
		
		TLInput::Platform::IPod::TTouchData TouchData(TouchRef);
		
		TouchData.uCurrentPos = int2((s32)currentTouchPosition.x, (s32)currentTouchPosition.y);
		TouchData.fTimestamp = (float) [touch timestamp ];
		TouchData.uTapCount = [touch tapCount];
		
		TouchData.uPhase = TLInput::Platform::IPod::TTouchData::Begin;
		TouchData.uPreviousPos = int2((s32)currentTouchPosition.x, (s32)currentTouchPosition.y);	
		
		TLInput::Platform::IPod::ProcessTouchBegin(TouchData);
	}
}


// Pass on the touch moved event to the input system for handling
- (void)touchesMoved:(NSSet *)touches withEvent:(UIEvent *)event 
{ 
	// Essentially we copy the data from each touch event into our own custom object and pass it onto the input system
	
	// The 'touch' object is unique and persistent whilst a finger is touching the screen
	// so we can use that to determine an index for the touch object
	
	// [13/03/09] DB - OK the alltouches set will give us literally that - all touch objects not 
	// just the ones that triggered the event.  So as we only want the ones that triggered the event
	// we use the touches set instead.  if we use the alltouches set we need to check the touchphase
	// otherwise we could send a touch object that is not ending as if it was ended.
    //NSSet *alltouches = [event allTouches];
	//for (UITouch *touch in alltouches)
	
    for (UITouch *touch in touches)
    {
		CGPoint currentTouchPosition = [touch locationInView:self]; 
		CGPoint previousTouchPosition = [touch previousLocationInView:self]; 
		
		// Use the address of the ipod touch obejct for the ID masked to ensure it's valid
		TRef TouchRef = GetTouchRef( touch );
		TLInput::Platform::IPod::TTouchData TouchData(TouchRef);
		/*
		 #ifdef _DEBUG
		 TString str;
		 str.Appendf("Event Touch Move: %d, (%d)", TouchRef.GetData(), touch );
		 TLDebug_Print(str);	
		 #endif
		 */
		
		
		TouchData.uCurrentPos = int2((s32)currentTouchPosition.x, (s32)currentTouchPosition.y);
		TouchData.fTimestamp = (float) [touch timestamp ];
		TouchData.uTapCount = [touch tapCount];
		
		TouchData.uPhase = TLInput::Platform::IPod::TTouchData::Move;
		TouchData.uPreviousPos = int2((s32)previousTouchPosition.x, (s32)previousTouchPosition.y);
		
		TLInput::Platform::IPod::ProcessTouchMoved(TouchData);
	}
}


// Pass on the touch end event to the input system for handling
- (void)touchesEnded:(NSSet*)touches withEvent:(UIEvent*)event {
	
	// Essentially we copy the data from each touch event into our own custom object and pass it onto the input system
	
	// The 'touch' object is unique and persistent whilst a finger is touching the screen
	// so we can use that to determine an index for the touch object	
	
	// [13/03/09] DB - OK the alltouches set will give us literally that - all touch objects not 
	// just the ones that triggered the event.  So as we only want the ones that triggered the event
	// we use the touches set instead.  if we use the alltouches set we need to check the touchphase
	// otherwise we could send a touch object that is not ending as if it was ended.
    //NSSet *alltouches = [event allTouches];
	//for (UITouch *touch in alltouches)
	
    for (UITouch *touch in touches)
    {
		CGPoint currentTouchPosition = [touch locationInView:self]; 
		CGPoint previousTouchPosition = [touch previousLocationInView:self]; 
		
		// Use the address of the ipod touch obejct for the ID masked to ensure it's valid
		TRef TouchRef = GetTouchRef( touch );
		
		/*
		 #ifdef _DEBUG
		 TString str;
		 str.Appendf("Event Touch End: %d, (%d)", TouchRef.GetData(), touch );
		 TLDebug_Print(str);	
		 #endif
		 */
		
		TLInput::Platform::IPod::TTouchData TouchData(TouchRef);
		
		TouchData.uCurrentPos = int2((s32)currentTouchPosition.x, (s32)currentTouchPosition.y);
		TouchData.fTimestamp = (float) [touch timestamp ];
		TouchData.uTapCount = [touch tapCount];
		
		TouchData.uPhase = TLInput::Platform::IPod::TTouchData::End;
		TouchData.uPreviousPos = int2((s32)previousTouchPosition.x, (s32)previousTouchPosition.y);
		
		TLInput::Platform::IPod::ProcessTouchEnd(TouchData);
		
		FreeTouchRef( touch );
	}
}

// Pass on the touch cancelled event to the input system for handling
- (void)touchesCancelled:(NSSet*)touches withEvent:(UIEvent*)event {
	
	// Essentially we copy the data from each touch event into our own custom object and pass it onto the input system
	
	// The 'touch' object is unique and persistent whilst a finger is touching the screen
	// so we can use that to determine an index for the touch object
	
	// [13/03/09] DB - OK the alltouches set will give us literally that - all touch objects not 
	// just the ones that triggered the event.  So as we only want the ones that triggered the event
	// we use the touches set instead.  if we use the alltouches set we need to check the touchphase
	// otherwise we could send a touch object that is not ending as if it was ended.
    //NSSet *alltouches = [event allTouches];
	//for (UITouch *touch in alltouches)
	
    for (UITouch *touch in touches)
    {
		CGPoint currentTouchPosition = [touch locationInView:self]; 
		CGPoint previousTouchPosition = [touch previousLocationInView:self]; 
		
		// Use the address of the ipod touch obejct for the ID masked to ensure it's valid
		TRef TouchRef = GetTouchRef( touch );
		/*
		 #ifdef _DEBUG
		 TString str;
		 str.Appendf("Event Touch Cancel: %d, (%d)", TouchRef.GetData(), touch );
		 TLDebug_Print(str);	
		 #endif
		 */
		
		TLInput::Platform::IPod::TTouchData TouchData(TouchRef);
		
		TouchData.uCurrentPos = int2((s32)currentTouchPosition.x, (s32)currentTouchPosition.y);
		TouchData.fTimestamp = (float) [touch timestamp ];
		TouchData.uTapCount = [touch tapCount];
		
		
		TouchData.uPhase = TLInput::Platform::IPod::TTouchData::Cancel;
		TouchData.uPreviousPos = int2((s32)previousTouchPosition.x, (s32)previousTouchPosition.y);
		
		TLInput::Platform::IPod::ProcessTouchEnd(TouchData);	
	}
}



/*
 #define HORIZ_SWIPE_DRAG_MIN 12 
 #define VERT_SWIPE_DRAG_MAX 4 
 - (void)touchesBegan:(NSSet *)touches withEvent:(UIEvent *)event 
 { 
 UITouch *touch = [touches anyObject]; 
 startTouchPosition = [touch locationInView:self]; 
 } 
 
 - (void)touchesMoved:(NSSet *)touches withEvent:(UIEvent *)event 
 { 
 UITouch *touch = [touches anyObject]; 
 CGPoint currentTouchPosition = [touch locationInView:self]; 
 
 // If the swipe tracks correctly. 
 if (fabsf(startTouchPosition.x - currentTouchPosition.x) >= 
 HORIZ_SWIPE_DRAG_MIN && 
 fabsf(startTouchPosition.y - currentTouchPosition.y) <= 
 VERT_SWIPE_DRAG_MAX) 
 { 
 // It appears to be a swipe. 
 if (startTouchPosition.x < currentTouchPosition.x) 
 [self myProcessRightSwipe:touches withEvent:event]; 
 else 
 [self myProcessLeftSwipe:touches withEvent:event]; 
 
 Handling Multi-Touch Events 81 
 2008-10-15 | © 2008 Apple Inc. All Rights Reserved. 
 
 } 
 else 
 { 
 // Process a non-swipe event. 
 } 
 } 
 */

/*
 ////////////////////////////////////////////////////////
 // Screenshot and store in photo library
 // 
 // The following apparently works but appears to be undocumented and doesn't work for me:
 // 
 // See http://my.safaribooksonline.com/9780596518554/miscellaneous_hacks_and_rec
 // UIApplication* app = [UIApplication sharedApplication]; 
 // [app _dumpScreenContents:nil];
 //
 // The following works OK for non-openGL views
 // Code from http://discussions.apple.com/thread.jspa?messageID=7983518&
 // 
 ////////////////////////////////////////////////////////
 
 - (UIImage *) saveViewToPhotoLibrary {
 
 CGRect screenRect = [[UIScreen mainScreen] bounds];
 UIGraphicsBeginImageContext(screenRect.size);
 
 CGContextRef ctx = UIGraphicsGetCurrentContext();
 [[UIColor blackColor] set];
 CGContextFillRect(ctx, screenRect);
 
 //	UIView* subview = glView;
 //    [subview.layer renderInContext:ctx];
 //[window.layer renderInContext:ctx];
 [self.layer renderInContext:ctx];
 
 
 UIImage *screenImage = UIGraphicsGetImageFromCurrentImageContext();
 
 // Save to photo album
 UIImageWriteToSavedPhotosAlbum(screenImage, nil, nil, nil);
 
 UIGraphicsEndImageContext();	
 
 return screenImage;
 }
 
 ////////////////////////////////////////////////////////
 
 */


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
- (void)image:(UIImage *)image didFinishSavingWithError:(NSError *)error contextInfo:(void *)contextInfo {
	
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

-(void)saveImageToFileAndSetupEmail:(UIImage*)image
{
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
}


-(void)saveViewToPhotoLibrary
{
	UIImage *image = [self createImageFromView];
	
	[self saveImageToPhotoLibrary :image];	
}

-(void)saveViewToPhotoLibraryAndSetupEmail
{
	self.bSendImage = TRUE;
	[self saveViewToPhotoLibrary];
}



-(void)saveImageToPhotoLibrary:(UIImage*) image
{
	UIImageWriteToSavedPhotosAlbum(image, self, (SEL)@selector(image:didFinishSavingWithError:contextInfo:), nil); // add callback for finish saving
}

-(NSString*) saveViewToFile
{
	UIImage *image = [self createImageFromView];
	
	return [self saveImageToFile: image];
}

-(NSString*) saveImageToFile:(UIImage*)image;
{
	// Get the tmp directory location
	NSString *tmpdir = NSTemporaryDirectory();
	
	// Create the final filename by appending the fixed name
	NSString *filename = [tmpdir stringByAppendingString: @"tmpimage.jpg"];
	
	// Get the JPG from image
	NSData* data = UIImagePNGRepresentation(image);
	
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
}


-(UIImage*)createImageFromView 
{
	CGRect rect = [[UIScreen mainScreen] bounds];
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
	
	UIImage *image = [[UIImage alloc] initWithCGImage:imageRef]; // change this to manual alloc/init instead of autorelease
	CGImageRelease(imageRef); // YOU CAN RELEASE THIS NOW
	
	return image;
}
////////////////////////////////////////////////////////


@end



