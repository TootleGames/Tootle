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


//------------------------------------------------------
//	
//------------------------------------------------------
TPtr<TLGui::TWindow> TLGui::CreateGuiWindow(TRefRef Ref)
{
	TPtr<TLGui::TWindow> pWindow = new TLGui::Platform::Window( Ref );	
	return pWindow;
}



TPtr<TLGui::TTree> TLGui::CreateTree(TLGui::TWindow& Parent,TRefRef Ref,TPtr<TLGui::TTreeItem>& pRootItem,const TArray<TRef>& Columns)
{
	TPtr<TLGui::TTree> pControl;
	return pControl;
}



TLGui::Platform::Window::Window(TRefRef WindowRef) : 
	TLGui::TWindow		( WindowRef ),
	m_pWindow			( NULL ),
	m_pDelegate			( NULL )
{
	//	Create a full-screen window
	//	create window (hidden so size is irrelevent but we'll use the full size)
	Type4<s32> DesktopSize;
	TLGui::Platform::GetDesktopSize(DesktopSize);
	NSRect Frame = NSMakeRect( DesktopSize.x, DesktopSize.y, DesktopSize.GetWidth(), DesktopSize.GetHeight() );
	
	//	alloc
	m_pWindow = [UIWindow alloc];
	
	//	init (returns the window object again)
	if ( m_pWindow )
		m_pWindow = [m_pWindow initWithFrame:Frame];
	
	//	failed to alloc/init
	if ( !m_pWindow )
	{
		TLDebug_Break("Failed to allocate NSWindow");
		return;
	}
	
	// Use a red window for debug so we can see any glitchy views of it
	//	gr: always pink background
	[m_pWindow setBackgroundColor:[NSColor magentaColor]];

	// Setup the accelerometer
	// Configure and start the accelerometer
    [[UIAccelerometer sharedAccelerometer] setUpdateInterval:(1.0 / ACCELEROMETER_FREQUENCY)];
    [[UIAccelerometer sharedAccelerometer] setDelegate:self];
}


Bool TLGui::Platform::Window::IsVisible() const
{
	return true;
}

void TLGui::Platform::Window::Show()
{
	//	gr: not applicable on ipod?
	//	Show the window
	//	mac:
	//[m_pWindow display];
	//[m_pWindow makeKeyAndOrderFront:nil];
	
	//	ipod
	[m_pWindow makeKeyAndVisible];	
}


//---------------------------------------------------------
//	set the CLIENT SIZE ("content" in os x) of the window
//---------------------------------------------------------
void TLGui::Platform::Window::SetSize(const int2& WidthHeight)
{
	//	gr: fail on ipod?
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
	//	gr: fail on ipod?
	NSPoint TopLeft;
	TopLeft.x = xy.x;
	TopLeft.y = xy.y;
	[m_pWindow setFrameTopLeftPoint:TopLeft];
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
