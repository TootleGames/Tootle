#include "IPodOpenglCanvas.h"
#include "IPodWindow.h"

//	mac cocoa includes, NSApp, NSArray etc
#include <TootleInput/IPod/IPodInput.h>
#include <TootleCore/TLMemory.h> // TEMP
#import "IPodWindowDelegate.h"




//------------------------------------------------------
//	
//------------------------------------------------------
TPtr<TLGui::TOpenglCanvas> TLGui::CreateOpenglCanvas(TWindow& Parent,TRefRef Ref)
{
	TPtr<TLGui::TOpenglCanvas> pControl = new TLGui::Platform::OpenglCanvas( Parent, Ref );
	return pControl;
}



TLGui::Platform::OpenglCanvas::OpenglCanvas(TLGui::TWindow& Parent,TRefRef ControlRef) :
	TLGui::TOpenglCanvas	( Parent, ControlRef ),
	m_pView					( NULL )
{
	//Create the OpenGL drawing view and add it to the window programatically
	// NOTE: Only needed if the window is not setup in a project .nib file

	//	gr: we want the canvas to be the size of the client area of the window
	int2 Size = Parent.GetSize();
	CGRect CanvasRect;
	CanvasRect.origin.x = 0;
	CanvasRect.origin.x = 0;
	CanvasRect.size.width = Size.x;
	CanvasRect.size.height = Size.y;
	
	m_pView = [[EAGLView alloc] initWithFrame:CanvasRect]; // - kPaletteHeight
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
	[m_pView initOpenGLContext];	
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
	EAGLContext* context = [m_pView openGLContext];
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

