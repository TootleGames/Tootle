#include "IPodScreen.h"
#include "IPodRender.h"
#include <TootleCore/TPtr.h>
#include <TootleCore/TLTime.h>
#include <TootleCore/TCoreManager.h>

#import <TootleInput/Ipod/IPodInput.h>

#import <UIKit/UIKit.h>
#import <QuartzCore/QuartzCore.h>
#import <OpenGLES/EAGL.h>
#import <OpenGLES/ES1/gl.h>
#import <OpenGLES/ES1/glext.h>
#import <OpenGLES/EAGLDrawable.h>




namespace TLCore
{
	extern TPtr<TCoreManager>		g_pCoreManager;
}



/*
 This class wraps the CAEAGLLayer from CoreAnimation into a convenient UIView subclass.
 The view content is basically an EAGL surface you render your OpenGL scene into.
 Note that setting the view non-opaque will only work if the EAGL surface has an alpha channel.
 */
@interface EAGLView : UIView {
    
@private
    /* The pixel dimensions of the backbuffer */
    GLint backingWidth;
    GLint backingHeight;
    
    EAGLContext *context;
    
    /* OpenGL names for the renderbuffer and framebuffers used to render to this view */
    GLuint viewRenderbuffer, viewFramebuffer;
    
    /* OpenGL name for the depth buffer that is attached to viewFramebuffer, if it exists (0 if it does not exist) */
    GLuint depthRenderbuffer;
}

@property (nonatomic, retain) EAGLContext *context;
@property GLuint viewRenderbuffer;
@property GLuint viewFramebuffer;
@property GLuint depthRenderbuffer;
@property GLint backingWidth;
@property GLint backingHeight;

- (id) initWithFrame:(CGRect)frame; //These also set the current context
- (id) initWithFrame:(CGRect)frame pixelFormat:(GLuint)format;
- (id) initWithFrame:(CGRect)frame pixelFormat:(GLuint)format depthFormat:(GLuint)depth preserveBackbuffer:(BOOL)retained;

- (BOOL) createFramebuffer;
- (void) destroyFramebuffer;

@end









//--------------------------------------------------------------------------------------------









#define USE_DEPTH_BUFFER 1


@implementation EAGLView

@synthesize context;
@synthesize viewRenderbuffer;
@synthesize viewFramebuffer;
@synthesize depthRenderbuffer;
@synthesize backingWidth;
@synthesize backingHeight;


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
		TLDebug_Print("Failed to create frame buffer");

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

	// TODO: Get pos for touch and send it to the routine
	//		 Define some kind of ID for each touch - to allow for multiple presses 
	//		 in multiple locations

	UITouch *touch = [touches anyObject]; 
	CGPoint currentTouchPosition = [touch locationInView:self]; 
	
	TPtr<TLInput::Platform::IPod::TTouchData> pTouchData = new TLInput::Platform::IPod::TTouchData();
	
	if(pTouchData)
	{
		pTouchData->uIndex = 0;
		pTouchData->uPreviousPos = int2((s32)currentTouchPosition.x, (s32)currentTouchPosition.y);
		pTouchData->uCurrentPos = int2((s32)currentTouchPosition.x, (s32)currentTouchPosition.y);
		pTouchData->uPhase = TLInput::Platform::IPod::Begin;
		
		TLInput::Platform::IPod::ProcessTouchBegin(pTouchData);
	}
}


// Pass on the touch moved event to the input system for handling
- (void)touchesMoved:(NSSet *)touches withEvent:(UIEvent *)event 
{ 
	// TODO: Get previous and current pos and send it through to the input system
	UITouch *touch = [touches anyObject]; 
	CGPoint currentTouchPosition = [touch locationInView:self]; 
	CGPoint previousTouchPosition = [touch previousLocationInView:self]; 
	
	
	TPtr<TLInput::Platform::IPod::TTouchData> pTouchData = new TLInput::Platform::IPod::TTouchData();
	
	if(pTouchData)
	{
		pTouchData->uIndex = 0;
		pTouchData->uPreviousPos = int2((s32)previousTouchPosition.x, (s32)previousTouchPosition.y);
		pTouchData->uCurrentPos = int2((s32)currentTouchPosition.x, (s32)currentTouchPosition.y);
		pTouchData->uPhase = TLInput::Platform::IPod::Move;
		
		TLInput::Platform::IPod::ProcessTouchMoved(pTouchData);
	}
}


// Pass on the touch end event to the input system for handling
- (void)touchesEnded:(NSSet*)touches withEvent:(UIEvent*)event {

	//TODO: Get the previous and current positions and send it through to the input system
	UITouch *touch = [touches anyObject]; 
	CGPoint currentTouchPosition = [touch locationInView:self]; 
	CGPoint previousTouchPosition = [touch previousLocationInView:self]; 
	
	TPtr<TLInput::Platform::IPod::TTouchData> pTouchData = new TLInput::Platform::IPod::TTouchData();

	if(pTouchData)
	{
		pTouchData->uIndex = 0;
		pTouchData->uPreviousPos = int2((s32)previousTouchPosition.x, (s32)previousTouchPosition.y);
		pTouchData->uCurrentPos = int2((s32)currentTouchPosition.x, (s32)currentTouchPosition.y);
		pTouchData->uPhase = TLInput::Platform::IPod::End;
	
		TLInput::Platform::IPod::ProcessTouchEnd(pTouchData);
	}
}

// Pass on the touch cancelled event to the input system for handling
- (void)touchesCancelled:(NSSet*)touches withEvent:(UIEvent*)event {
	
	//TODO: Get the previous and current positions and send it through to the input system
	UITouch *touch = [touches anyObject]; 
	CGPoint currentTouchPosition = [touch locationInView:self]; 
	CGPoint previousTouchPosition = [touch previousLocationInView:self]; 
	
	TPtr<TLInput::Platform::IPod::TTouchData> pTouchData = new TLInput::Platform::IPod::TTouchData();
	
	if(pTouchData)
	{
		pTouchData->uIndex = 0;
		pTouchData->uPreviousPos = int2((s32)previousTouchPosition.x, (s32)previousTouchPosition.y);
		pTouchData->uCurrentPos = int2((s32)currentTouchPosition.x, (s32)currentTouchPosition.y);
		pTouchData->uPhase = TLInput::Platform::IPod::Cancel;
		
		TLInput::Platform::IPod::ProcessTouchEnd(pTouchData);
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


@end







//--------------------------------------------------------------------------------------------




#import <TootleCore/IPod/IPodApp.h>
extern OpenglESAppAppDelegate* g_pIpodApp;





TLRender::Platform::Screen::Screen(TRefRef Ref) :
	TLRender::TScreen	( Ref )
{
}


//----------------------------------------------------------
//	create window
//----------------------------------------------------------
SyncBool TLRender::Platform::Screen::Init()
{
	//	no gl view? fail!
	EAGLView *glView = g_pIpodApp.glView;
	if ( !glView )
		return SyncFalse;

/*
	// Create window
	g_pWindow = [[UIWindow alloc] initWithFrame:[[UIScreen mainScreen] bounds]];

	// mode: window with status bar
	CGRect ScreenSize = [[UIScreen mainScreen] applicationFrame];
	// mode: full screen
	//CGRect ScreenSize = [[UIScreen mainScreen] bounds];
    
    // Set up content view
    UIView* glview = [[eglView alloc] initWithFrame:ScreenSize];
	[g_pWindow addSubview:glview];
    
	// Show window
	[g_pWindow makeKeyAndVisible];

	m_Size.Left() = ScreenSize.origin.x;
	m_Size.Top() = ScreenSize.origin.y;
	m_Size.Width() = ScreenSize.size.width;
	m_Size.Height() = ScreenSize.size.height;
*/

	//	save off dimensions for our engine
	m_Size.Left() = 0;
	m_Size.Top() = 0;
	m_Size.Width() = glView.backingWidth;
	m_Size.Height() = glView.backingHeight;
	
	return TScreen::Init();
}



//----------------------------------------------------------
//	update window
//----------------------------------------------------------
SyncBool TLRender::Platform::Screen::Update()
{
	return SyncTrue;
}



void TLRender::Platform::Screen::Draw()
{
	TLRender::Platform::BeginDraw();

	EAGLView *glView = g_pIpodApp.glView;

	[EAGLContext setCurrentContext:glView.context];
	glBindFramebufferOES(GL_FRAMEBUFFER_OES, glView.viewFramebuffer);
	
	//	do inherited draw
	TScreen::Draw();

	//	unbind data
	//	gr: not needed?
//	Platform::Opengl::Unbind( TRef() );

	//	flip buffers
    glBindRenderbufferOES(GL_RENDERBUFFER_OES, glView.viewRenderbuffer);
    [glView.context presentRenderbuffer:GL_RENDERBUFFER_OES];

	TLRender::Platform::EndDraw();
}


//----------------------------------------------------------
//	clean up
//----------------------------------------------------------
SyncBool TLRender::Platform::Screen::Shutdown()
{
	return TScreen::Shutdown();
}

