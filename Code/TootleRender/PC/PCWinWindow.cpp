#include "PCWinWindow.h"
#include "PCScreen.h"
#include "PCOpenglExt.h"


namespace Win32
{
	u32	g_MultisamplePixelFormat = 0;
}


//---------------------------------------------------------
//	create window class and create win32 control
//---------------------------------------------------------
Bool Win32::GWindow::Init(TPtr<GWinControl>& pOwner, u32 Flags)
{
	//	window needs to create class first
	if ( ! CreateClass() )
	{
		return NULL;
	}

	return GWinControl::Init( pOwner, Flags );
}




Win32::GOpenglWindow::GOpenglWindow(TRefRef InstanceRef) :
	GWindow				( InstanceRef ),
	m_HDC				( NULL ),
	m_HGLRC				( NULL ),
	m_HasArbMultiSample	( FALSE )
{
}


//----------------------------------------------------------
//	gr: todo: move all this opengl stuff into the window OnCreate()
//	and fail in there so all the smart pointers are cleaned up
//----------------------------------------------------------
Bool Win32::GOpenglWindow::Init(TPtr<GWinControl>& pOwner, u32 Flags)
{
	//	create window
	if ( !GWindow::Init( pOwner, Flags ) )
		return FALSE;

	//	create rendering stuff
	if ( !InitDisplay() )
		return FALSE;
/*
	//	if we dont know if we have multisample support, re-init the display to detect it
	SyncBool MultisampleSupport = TLRender::Platform::OpenglExtensions::IsHardwareSupported(TLRender::Platform::OpenglExtensions::GHardware_ARBMultiSample);
	if ( MultisampleSupport == SyncWait )
	{
		ShutdownDisplay();
		if ( !InitDisplay() )
			return FALSE;
	}
*/
	//	setup opengl stuff
	glDisable( GL_LIGHTING );

	glDisable( GL_CULL_FACE );
	glEnable( GL_DEPTH_TEST );
	glDepthFunc(GL_LEQUAL);								// The Type Of Depth Testing To Do

	//	antialiasing for polys
	glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);

	//	fastest for debug line/points
	glHint(GL_POINT_SMOOTH_HINT, GL_FASTEST);
	glHint(GL_LINE_SMOOTH_HINT, GL_FASTEST);

	//	make perspective correction good (mostly for textures)
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	glEnable(GL_POLYGON_SMOOTH);
	//glShadeModel(GL_SMOOTH);							// Enable Smooth Shading

	//	setup the texture alpha blending mode
	glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
	//glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL );
	//glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_BLEND );

	//	enable sphere map generation
	glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
	glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);


	TLDebug_Print( TString("Device %s", glGetString( GL_RENDERER ) ) );
	TLDebug_Print( TString("Driver %s %s", glGetString( GL_VENDOR ), glGetString( GL_VERSION ) ) );
//	TLDebug_Print( "-------------" );

	//	print off extensions first	
//	TLDebug_Print( "Extensions...");
//	TLDebug_Print( (char*)glGetString( GL_EXTENSIONS ) );
//	TLDebug_Print( "-------------");

	return TRUE;
}




Bool Win32::GOpenglWindow::InitDisplay()
{
	using namespace TLRender::Platform;
	
	//	make the pixel format descriptor
	PIXELFORMATDESCRIPTOR pfd=				// pfd Tells Windows How We Want Things To Be
	{
		sizeof(PIXELFORMATDESCRIPTOR),		// Size Of This Pixel Format Descriptor
		1,									// Version Number
		PFD_DRAW_TO_WINDOW |				// Format Must Support Window
		PFD_SUPPORT_OPENGL |				// Format Must Support OpenGL
		PFD_DOUBLEBUFFER,					// Must Support Double Buffering
		PFD_TYPE_RGBA,						// Request An RGBA Format
	//	16,									// Select Our Color Depth
		24,									// Select Our Color Depth
		0, 0, 0, 0, 0, 0,					// Color Bits Ignored
		0,									// No Alpha Buffer
		0,									// Shift Bit Ignored
		0,									// No Accumulation Buffer
		0, 0, 0, 0,							// Accumulation Bits Ignored
		16,									// 16Bit Z-Buffer (Depth Buffer)  
		1,									//	use stencil buffer
		0,									// No Auxiliary Buffer
		PFD_MAIN_PLANE,						// Main Drawing Layer
		0,									// Reserved
		0, 0, 0								// Layer Masks Ignored
	};
			
	u32 PixelFormat=0;

	//	get the hdc
	if ( ! ( m_HDC = GetDC( m_Hwnd ) ) )
	{
		TLDebug_Break("Failed to get HDC\n");
		return FALSE;
	}


	//	if multisample is supported, use a multisample pixelformat
	SyncBool MultisampleSupport = OpenglExtensions::IsHardwareSupported(OpenglExtensions::GHardware_ARBMultiSample);

	if ( MultisampleSupport == SyncTrue )
	{
		PixelFormat = OpenglExtensions::GetArbMultisamplePixelFormat();
		m_HasArbMultiSample = TRUE;
	}
	else
	{
		//	check we can use this pfd
		if ( ! ( PixelFormat = ChoosePixelFormat( m_HDC, &pfd ) ) )
		{	
			TLDebug_Break( TString("Failed to choose pixel format %d\n",PixelFormat) );
			return FALSE;
		}
		m_HasArbMultiSample = FALSE;
	}

	//	set it to the pfd
	if ( ! SetPixelFormat( m_HDC, PixelFormat, &pfd ) )
	{
		TLDebug_Break("Failed to set pixel format");
		return FALSE;
	}

	//	make and get the windows gl context for the hdc
	if ( ! ( m_HGLRC = wglCreateContext( m_HDC ) ) )
	{
		TLDebug_Break("Failed to create context");
		return FALSE;
	}

	//	set the active HGLRC for this HDC
	if ( ! wglMakeCurrent( m_HDC, m_HGLRC ) )
	{
		TLDebug_Break("Failed wglMakeCurrent");
		return FALSE;
	}

	//	mark opengl as initialised once we've created a GL wglCreateContext
	TLRender::Platform::g_OpenglInitialised = SyncTrue;

	return TRUE;
}


void Win32::GOpenglWindow::ShutdownDisplay()
{
	if ( m_HDC )											// Does The Window Have A Device Context?
	{
		wglMakeCurrent( m_HDC, 0 );							// Set The Current Active Rendering Context To Zero
		if ( m_HGLRC != NULL )										// Does The Window Have A Rendering Context?
		{
			wglDeleteContext( m_HGLRC );							// Release The Rendering Context
			m_HGLRC = NULL;										// Zero The Rendering Context
		}

		ReleaseDC( m_Hwnd, m_HDC );						// Release The Device Context
		m_HDC = NULL;											// Zero The Device Context
	}
	
}

