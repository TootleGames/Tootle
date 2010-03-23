/*------------------------------------------------------

	

-------------------------------------------------------*/
#pragma once
#include "../TOpenglCanvas.h"

#include <TootleCore/TLTypes.h>

#import <UIKit/UIKit.h>
#import <QuartzCore/QuartzCore.h>
#import <OpenGLES/ES1/glext.h>




namespace TLGui
{
	namespace Platform
	{
		class OpenglCanvas;
	}
}

//	forward declarations
@class EAGLView;


class TLGui::Platform::OpenglCanvas : public TLGui::TOpenglCanvas
{
public:
	OpenglCanvas(TLGui::TWindow& Parent,TRefRef ControlRef);
	
	virtual Bool	BeginRender();
	virtual void	EndRender();
		
protected:
	EAGLView*	m_pView;	//	cocoa opengl view
};


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
	
	// Send image flag for auto emailing of images
	Bool	bSendImage;
}

@property (nonatomic, retain) EAGLContext *context;
@property GLuint viewRenderbuffer;
@property GLuint viewFramebuffer;
@property GLuint depthRenderbuffer;
@property GLint backingWidth;
@property GLint backingHeight;
@property Bool	bSendImage;

- (id) initWithFrame:(CGRect)frame; //These also set the current context
- (id) initWithFrame:(CGRect)frame pixelFormat:(GLuint)format;
- (id) initWithFrame:(CGRect)frame pixelFormat:(GLuint)format depthFormat:(GLuint)depth preserveBackbuffer:(BOOL)retained;

- (BOOL) createFramebuffer;
- (void) destroyFramebuffer;

- (UIImage *)createImageFromView;
- (void)saveViewToPhotoLibrary;
- (void)saveViewToPhotoLibraryAndSetupEmail;
- (void)saveImageToPhotoLibrary:(UIImage*) image;
- (NSString*)saveViewToFile;
- (NSString*)saveImageToFile:(UIImage*)image;
- (void)saveImageToFileAndSetupEmail:(UIImage*)image;


- (void)image:(UIImage *)image didFinishSavingWithError:(NSError *)error contextInfo:(void *)contextInfo;


@end
