#include "PCOpenglExt.h"
#include <TootleCore/TArray.h>
#include <TootleCore/TString.h>
#include "PCWinWindow.h"


//----------------------------------------------------------
//	globals
//----------------------------------------------------------
namespace TLRender
{
	namespace Platform
	{
		extern TPtr<Win32::GOpenglWindow>	g_pSpareWindow;

		namespace OpenglExtensions
		{
			u32					g_DisabledHardwareFlags		= 0x0;		//	manually disabled hardware features
			u32					g_HardwareFlags				= 0x0;		//	flags for what hardware features are supported
			TArray<void*>		g_ExtensionAddresses[GHardware_Max];	//	list of function pointers for each hardware extension
			SyncBool			g_ArbMultiSampleSupported	= SyncWait;	//	
			u32					g_ArbMultiSamplePixelFormat	= 0;

			Bool				IsExtensionNameSupported(const TString& ExtensionName,Bool CheckWGLExtensions);
			Bool				IsExtensionNameInString(const TString& ExtensionName,const char* pExtensionListString);
			void*				GetExtensionFunctionAddress(u32 HardwareIndex,u32 FunctionIndex);	//	fetch the address for a func out of the array
	
			Bool				GetExtensionName(u32 HardwareIndex,TString& String);				//	get the string identifier for an extension and put it into a string. the string is the extension's name in the OGL list
			void				SetHardwareSupported(u32 HardwareIndex,SyncBool Supported);			//	enable/disable support for a hardware extension
			void				InitHardwareSupport(u32 HardwareIndex);
			Bool				AddExtensionFunction(u32 HardwareIndex,const char* pFunctionName=NULL);	//	add this function to this hardware extension. if no function provided the extension name IS the function so fetch it
			void				Debug_IsHardwareSupported(u32 HardwareIndex);						//	print out if extension is supported or not
			u32					GetArbMultisamplePixelFormat()										{	return g_ArbMultiSamplePixelFormat;	}
			Bool				CreateMultiSamplePixelFormat();
		}
	}
}




//------------------------------------------------------------
//	get the string identifier for an extension and put it into a string. 
//	the string is the extension's name in the OGL list OR name of
//	a single function if that's all the extension is for
//------------------------------------------------------------
Bool TLRender::Platform::OpenglExtensions::GetExtensionName(u32 HardwareIndex,TString& String)
{
	switch ( HardwareIndex )
	{
		//	extensions
	case GHardware_MultiTexturing:		String.Append("GL_ARB_multitexture");			return TRUE;	break;
	case GHardware_VertexBufferObjects:	String.Append("GL_ARB_vertex_buffer_object");	return TRUE;	break;
	case GHardware_NVVertexProgram:		String.Append("GL_NV_vertex_program");			return TRUE;	break;
	case GHardware_ARBVertexProgram:	String.Append("GL_ARB_vertex_program");			return TRUE;	break;
	case GHardware_ARBFragmentProgram:	String.Append("GL_ARB_fragment_program");		return TRUE;	break;

		//	functions
	case GHardware_DrawRangeElements:	String.Append("glDrawRangeElements");			return TRUE;	break;
	case GHardware_SwapInterval:		String.Append("wglSwapIntervalEXT");			return TRUE;	break;
	case GHardware_ARBMultiSample:		String.Append("WGL_ARB_multisample");			return TRUE;	break;
	}

	//	invalid index
	TLDebug_Break( TString("Requesting invalid hardware extension index %d", HardwareIndex) );
	String.Append("???");
	return FALSE;
}


			
//----------------------------------------------------------
//	declarations of all the opengl stuff... 
//	actual engine implementations are at the bottom
//----------------------------------------------------------


//	multitexture

//	nv vertex program function

// arb vertex program
#define GL_VERTEX_PROGRAM_ARB                              0x8620
#define GL_PROGRAM_FORMAT_ASCII_ARB                        0x8875
#define GL_PROGRAM_ERROR_POSITION_ARB                      0x864B
#define GL_CURRENT_MATRIX_ARB                              0x8641
#define GL_TRANSPOSE_CURRENT_MATRIX_ARB                    0x88B7
#define GL_CURRENT_MATRIX_STACK_DEPTH_ARB                  0x8640
#define GL_MAX_VERTEX_ATTRIBS_ARB                          0x8869
#define GL_MAX_PROGRAM_MATRICES_ARB                        0x862F
#define GL_MAX_PROGRAM_MATRIX_STACK_DEPTH_ARB              0x862E
#define GL_PROGRAM_ERROR_STRING_ARB                        0x8874

#ifndef GL_ARB_fragment_program
#define GL_FRAGMENT_PROGRAM_ARB           0x8804
#define GL_PROGRAM_ALU_INSTRUCTIONS_ARB   0x8805
#define GL_PROGRAM_TEX_INSTRUCTIONS_ARB   0x8806
#define GL_PROGRAM_TEX_INDIRECTIONS_ARB   0x8807
#define GL_PROGRAM_NATIVE_ALU_INSTRUCTIONS_ARB 0x8808
#define GL_PROGRAM_NATIVE_TEX_INSTRUCTIONS_ARB 0x8809
#define GL_PROGRAM_NATIVE_TEX_INDIRECTIONS_ARB 0x880A
#define GL_MAX_PROGRAM_ALU_INSTRUCTIONS_ARB 0x880B
#define GL_MAX_PROGRAM_TEX_INSTRUCTIONS_ARB 0x880C
#define GL_MAX_PROGRAM_TEX_INDIRECTIONS_ARB 0x880D
#define GL_MAX_PROGRAM_NATIVE_ALU_INSTRUCTIONS_ARB 0x880E
#define GL_MAX_PROGRAM_NATIVE_TEX_INSTRUCTIONS_ARB 0x880F
#define GL_MAX_PROGRAM_NATIVE_TEX_INDIRECTIONS_ARB 0x8810
#define GL_MAX_TEXTURE_COORDS_ARB         0x8871
#define GL_MAX_TEXTURE_IMAGE_UNITS_ARB    0x8872
#endif


//----------------------------------------------------------
//	print out if extension is supported or not
//----------------------------------------------------------
void TLRender::Platform::OpenglExtensions::Debug_IsHardwareSupported(u32 HardwareIndex)
{
	//	invalid hardware index - abort
	TTempString ExtensionString;
	if ( !GetExtensionName( HardwareIndex, ExtensionString ) )
	{
		ExtensionString = "Invalid hardware index";
	}

	SyncBool HardwareSupport = IsHardwareSupported( HardwareIndex );

	if ( HardwareSupport == SyncTrue )
		ExtensionString.Append(" is supported.");
	else if ( HardwareSupport == SyncWait )
		ExtensionString.Append(" is MAYBE supported.");
	else
		ExtensionString.Append(" is NOT supported.");

	//	print
	TLDebug_Print( ExtensionString );
}


//----------------------------------------------------------
//	is this extension supported
//----------------------------------------------------------
SyncBool TLRender::Platform::OpenglExtensions::IsHardwareSupported(u32 HardwareIndex)
{
	u32 HardwareFlag = (1<<HardwareIndex);
	u32 SupportedFlags = g_HardwareFlags;

	if ( ( g_HardwareFlags & HardwareFlag ) == HardwareFlag )
		return SyncTrue;

	//	not supported, some cases have a maybe-supported state
	if ( HardwareIndex == GHardware_ARBMultiSample )
		return g_ArbMultiSampleSupported;

	return SyncFalse;
}


//----------------------------------------------------------
//	mark hardware as unsupported
//----------------------------------------------------------
void TLRender::Platform::OpenglExtensions::SetHardwareSupported(u32 HardwareIndex,SyncBool Supported)
{
	u32 HardwareFlag = (1<<HardwareIndex);
	SyncBool WasSupported = IsHardwareSupported(HardwareIndex);
	
	if ( Supported == SyncTrue )
	{
		//	add supported flag
		g_HardwareFlags |= HardwareFlag;
	}
	else
	{
		//	remove flag
		g_HardwareFlags &= ~HardwareFlag;

		//	remove extension addresses
		g_ExtensionAddresses[HardwareIndex].Empty();
	}

	//	special case
	if ( HardwareIndex == GHardware_ARBMultiSample )
		g_ArbMultiSampleSupported = Supported;

	//	debug that we do/dont support this hardware
	//	gr: onyl if state changes
	if ( WasSupported != Supported )
	{
		Debug_IsHardwareSupported( HardwareIndex );
	}
}

//----------------------------------------------------------
//	is this extension enabled? 
//----------------------------------------------------------
Bool TLRender::Platform::OpenglExtensions::IsHardwareEnabled(u32 HardwareIndex)
{
	//	not supported
	if ( IsHardwareSupported( HardwareIndex ) == SyncFalse )
		return FALSE;

	//	check is enabled
	u32 EnabledFlags = ~g_DisabledHardwareFlags;
	u32 HardwareFlag = (1<<HardwareIndex);
	
	return ( EnabledFlags & HardwareFlag ) == HardwareFlag;
}


//----------------------------------------------------------
//	initialise extensions
//----------------------------------------------------------
SyncBool TLRender::Platform::OpenglExtensions::Init()
{
	//	wait for opengl to initialise
	if ( TLRender::Platform::g_OpenglInitialised != SyncTrue )
		return TLRender::Platform::g_OpenglInitialised;

	//	probe to see what hardware is supported
	for ( u32 i=0;	i<GHardware_Max;	i++ )
	{
		//	init extension support
		InitHardwareSupport( i );

		if ( IsHardwareSupported(i) != SyncTrue )
		{
			//	report that it's not supported
			Debug_IsHardwareSupported( i );
		}
	}

	return SyncTrue;
}



//----------------------------------------------------------
//	shutdown extensions
//----------------------------------------------------------
SyncBool TLRender::Platform::OpenglExtensions::Shutdown()
{
	//	gr: this is corrupting the stack...
	SetHardwareSupported( GHardware_MultiTexturing,			SyncWait );
	SetHardwareSupported( GHardware_VertexBufferObjects,	SyncWait );
	SetHardwareSupported( GHardware_DrawRangeElements,		SyncWait );
	SetHardwareSupported( GHardware_SwapInterval,			SyncWait );
	SetHardwareSupported( GHardware_NVVertexProgram,		SyncWait );
	SetHardwareSupported( GHardware_ARBVertexProgram,		SyncWait );
	SetHardwareSupported( GHardware_ARBFragmentProgram,		SyncWait );

	return SyncTrue;
}



//-----------------------------------------------------------
//	fetch the address for a func out of the array
//-----------------------------------------------------------
void* TLRender::Platform::OpenglExtensions::GetExtensionFunctionAddress(u32 HardwareIndex,u32 FunctionIndex)
{
	return g_ExtensionAddresses[HardwareIndex][FunctionIndex];
}



//-----------------------------------------------------------
//	try and add an extension to our roster
//-----------------------------------------------------------
Bool TLRender::Platform::OpenglExtensions::AddExtensionFunction(u32 HardwareIndex,const char* pFunctionName)
{
	//	check hardware is supported
	if ( !IsHardwareSupported( HardwareIndex ) )
		return FALSE;

	TString ExtensionName;
	if ( !GetExtensionName( HardwareIndex, ExtensionName ) )
		return FALSE;

	//	if no function name is provided then the function name IS the extension name
	if ( !pFunctionName )
		pFunctionName = ExtensionName.GetData();

	void* pAddr = wglGetProcAddress(pFunctionName);

	//	no such address - remove support
	if ( !pAddr )
	{
		TLDebug_Break( TString("Address %s missing on extension %s, extension disabled", pFunctionName, ExtensionName.GetData() ) );
		SetHardwareSupported( HardwareIndex, SyncFalse );
		return FALSE;
	}

	//	add this function to this hardware extension's address
	g_ExtensionAddresses[HardwareIndex].Add( pAddr );

	return TRUE;
}


//-----------------------------------------------------------
//	is this extension name supported in opengl?
//-----------------------------------------------------------
Bool TLRender::Platform::OpenglExtensions::IsExtensionNameSupported(const TString& ExtensionName,Bool CheckWGLExtensions)
{
	const char *pExtensions = NULL;

	if ( CheckWGLExtensions )
	{
		// Try To Use wglGetExtensionStringARB On Current DC, If Possible
		PROC wglGetExtString = wglGetProcAddress("wglGetExtensionsStringARB");
		if (wglGetExtString)
			pExtensions = ((char*(__stdcall*)(HDC))wglGetExtString)(wglGetCurrentDC());
	}
	else
	{
		//	Get standard Extensions String
		pExtensions = (const char*)glGetString( GL_EXTENSIONS );
	}

	if ( !pExtensions )
		return FALSE;

	return IsExtensionNameInString( ExtensionName, pExtensions );
}


//-----------------------------------------------------------
//	is this extension name supported in opengl?
//-----------------------------------------------------------
Bool TLRender::Platform::OpenglExtensions::IsExtensionNameInString(const TString& ExtensionName,const char* pExtensionListString)
{
	//	check for an invalid extension name...
	//	no spaces!
	if ( ExtensionName.GetCharExists(' ') )
	{
		TLDebug_Break( TString("Invalid extension name: %s", ExtensionName.GetData() ) );
		return FALSE;
	}

	if ( !pExtensionListString )
		return FALSE;

	//	loop through and match our extension to the list of extensions
	while ( (*pExtensionListString) != 0x0 )
	{
		u32 Length = 0;
		Bool FoundTerminator = FALSE;

		//	find next space/terminator
		//	gr: count up to 100 for safety - no extension is going to be that long
		for ( Length=0;	Length<100;	Length++ )
		{
			const char& Char = pExtensionListString[Length];
			FoundTerminator = ( Char == 0x0 );
			if ( Char == ' ' || Char == 0x0 )
				break;
		}

		//	compare chunk of extension string to our extension name
		if ( ExtensionName.IsEqual( pExtensionListString, Length, FALSE ) )
		{
			//	match! is supported!
			return TRUE;
		}

		//	found terminator, no more to check
		if ( FoundTerminator )
			break;

		//	not supported, skip onto next string
		pExtensionListString = &pExtensionListString[Length+1];
	}

	//	not matched... not supported
	return FALSE;
}


void TLRender::Platform::OpenglExtensions::InitHardwareSupport(u32 HardwareIndex)
{
	//	already supported
	if ( IsHardwareSupported(HardwareIndex) == SyncTrue )
		return;

	//	get the extension name
	TString ExtensionName;
	if ( !GetExtensionName( HardwareIndex, ExtensionName ) )
	{
		SetHardwareSupported( HardwareIndex, SyncFalse );
		return;
	}

	//	check extension supported in GL_EXTENSIONS
	if ( wglGetProcAddress( ExtensionName.GetData() ) )
	{
		//	see if function with that name is supported
		SetHardwareSupported( HardwareIndex, SyncTrue );
	}
	else if ( IsExtensionNameSupported(ExtensionName,TRUE) )
	{
		SetHardwareSupported( HardwareIndex, SyncTrue );
	}
	else if ( IsExtensionNameSupported(ExtensionName,FALSE) )
	{
		SetHardwareSupported( HardwareIndex, SyncTrue );
	}
	else
	{
		//	not supported
		SetHardwareSupported( HardwareIndex, SyncFalse );
		return;
	}

	//	do specific initialisation
	switch ( HardwareIndex )
	{
	case GHardware_MultiTexturing:
		//	multitexturing
		AddExtensionFunction( GHardware_MultiTexturing,	"glActiveTextureARB" );
		AddExtensionFunction( GHardware_MultiTexturing,	"glMultiTexCoord2fARB" );
		AddExtensionFunction( GHardware_MultiTexturing,	"glClientActiveTextureARB" );
		break;

	case GHardware_VertexBufferObjects:
		//	vertex buffer objects
		AddExtensionFunction( GHardware_VertexBufferObjects,	"glGenBuffersARB" );
		AddExtensionFunction( GHardware_VertexBufferObjects,	"glBindBufferARB" );
		AddExtensionFunction( GHardware_VertexBufferObjects,	"glBufferDataARB" );
		AddExtensionFunction( GHardware_VertexBufferObjects,	"glDeleteBuffersARB" );
		break;

	case GHardware_DrawRangeElements:
		//	draw range elements
		AddExtensionFunction( GHardware_DrawRangeElements );
		break;
	
	case GHardware_SwapInterval:
		//	swap interval
		AddExtensionFunction( GHardware_SwapInterval );	
		break;

	case GHardware_NVVertexProgram:
		//	nv vertex program
		if ( !TLRender::Platform::TLShader::InitHardware( GHardware_NVVertexProgram ) )
		{
			//	failed to init, remove support
			SetHardwareSupported( GHardware_NVVertexProgram, SyncFalse );
		}
		break;

	case GHardware_ARBVertexProgram:
		//	arb vertex program
		if ( !TLRender::Platform::TLShader::InitHardware( GHardware_ARBVertexProgram ) )
		{
			//	failed to init, remove support
			SetHardwareSupported( GHardware_ARBVertexProgram, SyncFalse );
		}
		break;

	case GHardware_ARBMultiSample:
		if ( !CreateMultiSamplePixelFormat() )
		{
			SetHardwareSupported( GHardware_ARBMultiSample, SyncFalse );
		}
		break;
	}
}


Bool TLRender::Platform::OpenglExtensions::CreateMultiSamplePixelFormat()
{
	//	fetch the window to use... use the spare window which is the one we initialised the system with
	TPtr<Win32::GOpenglWindow>& pOpenglWindow = TLRender::Platform::g_pSpareWindow;

	//	invalid param
	if ( !pOpenglWindow )
	{
		TLDebug_Break("Window expected");
		return FALSE;
	}

	//	already got a format
	if ( g_ArbMultiSamplePixelFormat != 0 )
		return TRUE;

	// Get Our Pixel Format
	PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB = (PFNWGLCHOOSEPIXELFORMATARBPROC)wglGetProcAddress("wglChoosePixelFormatARB");	
	if ( !wglChoosePixelFormatARB ) 
		return FALSE;

	// Get Our Current Device Context
	HDC hDC = GetDC( pOpenglWindow->Hwnd() );

	int		pixelFormat;
	int		valid;
	UINT	numFormats;
	float	fAttributes[] = {0,0};

	#define WGL_SAMPLE_BUFFERS_ARB		 0x2041
	#define WGL_SAMPLES_ARB			     0x2042

	// These Attributes Are The Bits We Want To Test For In Our Sample
	// Everything Is Pretty Standard, The Only One We Want To 
	// Really Focus On Is The SAMPLE BUFFERS ARB And WGL SAMPLES
	// These Two Are Going To Do The Main Testing For Whether Or Not
	// We Support Multisampling On This Hardware.
	int iAttributes[] =
	{
		WGL_DRAW_TO_WINDOW_ARB,GL_TRUE,
		WGL_SUPPORT_OPENGL_ARB,GL_TRUE,
		WGL_ACCELERATION_ARB,WGL_FULL_ACCELERATION_ARB,
		WGL_COLOR_BITS_ARB,24,
		WGL_ALPHA_BITS_ARB,8,
		WGL_DEPTH_BITS_ARB,16,
		WGL_STENCIL_BITS_ARB,0,
		WGL_DOUBLE_BUFFER_ARB,GL_TRUE,
		WGL_SAMPLE_BUFFERS_ARB,GL_TRUE,
		WGL_SAMPLES_ARB,4,
		0,0
	};

	// First We Check To See If We Can Get A Pixel Format For 4 Samples
	valid = wglChoosePixelFormatARB(hDC,iAttributes,fAttributes,1,&pixelFormat,&numFormats);
 
	// If We Returned True, And Our Format Count Is Greater Than 1
	if (valid && numFormats >= 1)
	{
		g_ArbMultiSamplePixelFormat = pixelFormat;	
		return TRUE;
	}

	// Our Pixel Format With 4 Samples Failed, Test For 2 Samples
	iAttributes[19] = 2;	//	WGL_SAMPLES_ARB = 2
	valid = wglChoosePixelFormatARB(hDC,iAttributes,fAttributes,1,&pixelFormat,&numFormats);
	if (valid && numFormats >= 1)
	{
		g_ArbMultiSamplePixelFormat = pixelFormat;	 
		return TRUE;
	}

	return FALSE;
}