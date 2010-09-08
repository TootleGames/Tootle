#include "MacWindow.h"

//	include NSApp, NSArray etc
#include <TootleInput/Mac/MacInput.h>
#import <IOKit/IOKitLib.h>
#import <IOKit/hid/IOHIDManager.h>
#import <IOKit/hid/IOHIDKeys.h>
#import <Appkit/Appkit.h>
#include <TootleCore/TLMemory.h> // TEMP
#import "MacWindowDelegate.h"




//	temp whilst the factory functions are in this file
#include "../TTree.h"

TLGui::Platform::Window::Window(TRefRef WindowRef) : 
	TLGui::TWindow		( WindowRef ),
	m_pWindow			( NULL ),
	m_pDelegate			( NULL )
{
	//	create window (hidden so size is irrelevent)
	NSRect Frame = NSMakeRect( 0, 0, 50, 50 );
	unsigned int styleMask = NSTitledWindowMask	| NSMiniaturizableWindowMask | NSClosableWindowMask /*| NSResizeableWindowMask*/;

	
	//	alloc
	m_pWindow = [NSWindow alloc];
	
	//	init (returns the window object again)
	if ( m_pWindow )
		m_pWindow = [m_pWindow initWithContentRect:Frame styleMask: styleMask backing: NSBackingStoreBuffered defer:YES];
	
	//	failed to alloc/init
	if ( !m_pWindow )
	{
		TLDebug_Break("Failed to allocate NSWindow");
		return;
	}
	
	// Use a red window for debug so we can see any glitchy views of it
	//	gr: always pink background
	[m_pWindow setBackgroundColor:[NSColor magentaColor]];
	
	//	allocate delegate
	m_pDelegate = [TootleWindowDelegate alloc];
	if ( !m_pDelegate )
	{
		TLDebug_Break("Failed to alloc window delegate");
		return;
	}

	//	set window's delegate
	[m_pWindow setDelegate:m_pDelegate];
	[m_pDelegate setWindow:m_pWindow];

}


Bool TLGui::Platform::Window::IsVisible() const
{
	return true;
}

void TLGui::Platform::Window::Show()
{
	//	Show the window
	[m_pWindow display];
	[m_pWindow makeKeyAndOrderFront:nil];	
}


//---------------------------------------------------------
//	set the CLIENT SIZE ("content" in os x) of the window
//---------------------------------------------------------
void TLGui::Platform::Window::SetSize(const int2& WidthHeight)
{
	NSSize Size;
	Size.width = WidthHeight.x;
	Size.height = WidthHeight.y;
	[m_pWindow setContentSize:Size];
}


//---------------------------------------------------------
//	set the CLIENT SIZE ("content" in os x) of the window
//---------------------------------------------------------	 
int2 TLGui::Platform::Window::GetSize()
{
	//	gr: i think this is the window size, not the content size
	NSRect Rect = m_pWindow.frame;
	
	//	gr: cast required as mac window dimensions are in floats...
	return int2( (int)Rect.size.width, (int)Rect.size.height );
}

//---------------------------------------------------
//	set the top-left position of the window frame
//---------------------------------------------------
void TLGui::Platform::Window::SetPosition(const int2& xy)
{
	NSPoint TopLeft;
	TopLeft.x = xy.x;
	TopLeft.y = xy.y;
	
	//	gr: set the origin position to position the window at the top left that we percieve.
	//		oddly, if you use SetFrameTopLeft it's in upside-down space (graph style) like the rest
	//		of the window coordinate system. So we use this instead of having to fetch the desktop size
	[m_pWindow setFrameOrigin:TopLeft];
}


/*
	
	
Canvas::Canvas()
{
 //Create the OpenGL drawing view and add it to the window programatically
 // NOTE: Only needed if the window is not setup in a project .nib file
		
	glView = [[TootleOpenGLView alloc] initWithFrame:rect]; // - kPaletteHeight
	//[[NSApp mainWindow] addSubview:glView];
	
	if(!glView)
	{
		return;
	}
	
	
	//NSView* contentview = [[NSApp mainWindow] contentView];
	//[contentview addSubview:glView];
	[window setContentView:glView];
	//	[glView setWindow:window];
	
	// Now we are part of the window we can setup the opengl context
	// NOTE: Has to be done AFTER the view is added to the window
	[glView initOpenGLContext];	
	
}
*/
