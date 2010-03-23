#include "MacOpenglCanvas.h"
#include "MacWindow.h"
#import "MacView.h"	//	cocoa opengl NSView implementation

#if defined(TL_ENABLE_WX)
//	wx gui implementation
#include "wxWidgets/Window.h"
#include "wxWidgets/OpenglCanvas.h"
#include "wxWidgets/Tree.h"
#else
//	mac cocoa includes, NSApp, NSArray etc
#include <TootleInput/Mac/MacInput.h>
#import <IOKit/IOKitLib.h>
#import <IOKit/hid/IOHIDManager.h>
#import <IOKit/hid/IOHIDKeys.h>
#import <Appkit/Appkit.h>
#include <TootleCore/TLMemory.h> // TEMP
#import "MacWindowDelegate.h"
#endif




//------------------------------------------------------
//	
//------------------------------------------------------
TPtr<TLGui::TOpenglCanvas> TLGui::CreateOpenglCanvas(TWindow& Parent,TRefRef Ref)
{
#if defined(TL_ENABLE_WX)
	TPtr<TLGui::TOpenglCanvas> pControl = new wx::OpenglCanvas( Parent, Ref );
#else
	TPtr<TLGui::TOpenglCanvas> pControl = new TLGui::Platform::OpenglCanvas( Parent, Ref );
#endif
	
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
	NSRect CanvasRect;
	CanvasRect.origin.x = 0;
	CanvasRect.origin.x = 0;
	CanvasRect.size.width = Size.x;
	CanvasRect.size.height = Size.y;
	
	m_pView = [[TootleOpenGLView alloc] initWithFrame:CanvasRect]; // - kPaletteHeight
	//[[NSApp mainWindow] addSubview:glView];
		
	if ( !m_pView )
	{
		TLDebug_Break("Failed to alloc TootleOpenglView");
		return;
	}
	
	
	//	get NSWindow 
	Platform::Window& PlatformWindow = static_cast<Platform::Window&>( Parent );
	NSWindow* pWindow = PlatformWindow.m_pWindow;

	//	set the view on the parent window
	//NSView* contentview = [[NSApp mainWindow] contentView];
	//[contentview addSubview:glView];
	[pWindow setContentView:m_pView];
	//	[glView setWindow:window];

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

	//	gr: need to set current context?
	
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

	//	get the context to flush (this flips)
	// Get the view in use by the window
//	NSOpenGLView* view = [[NSApp mainWindow] contentView];
	NSOpenGLContext* context = [m_pView openGLContext];
	[context flushBuffer];
	
	/*
	//	post-draw make sure the swap interval is limited/not limited
	if ( OpenglExtensions::IsHardwareEnabled( OpenglExtensions::GHardware_SwapInterval ) )
	{
		
		if ( GetFlag( TScreen::Flag_SyncFrameRate ) )
		{
			GLint swap = 1;
			//OpenglExtensions::glSwapIntervalEXT()( 1 );
			[context setValues:&swap forParameter:NSOpenGLCPSwapInterval];
		}
		else
		{
			GLint swap = 0;
			//OpenglExtensions::glSwapIntervalEXT()( 0 );
			[context setValues:&swap forParameter:NSOpenGLCPSwapInterval];
		}
	}
	*/
}

