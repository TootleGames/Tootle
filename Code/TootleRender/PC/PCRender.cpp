#include "../TLRender.h"
#include "PCRender.h"
#include "PCWinControl.h"
#include <TootleCore/TSyncQueue.h>
#include "PCOpenglExt.h"
#include "PCWinControl.h"
#include "PCWinWindow.h"
#include <TootleAsset/TMesh.h>
#include <TootleCore/TColour.h>
#include <TootleCore/TKeyArray.h>


//---------------------------------------------------
//	globals
//---------------------------------------------------
namespace TLRender
{
	namespace Platform
	{
		SyncBool					g_OpenglInitialised = SyncWait;
		TPtr<Win32::GOpenglWindow>	g_pSpareWindow;
	}

}


//---------------------------------------------------
//	platform/opengl initialisation
//---------------------------------------------------
SyncBool TLRender::Platform::Init()
{
	//	setup default states
	glEnable( GL_SCISSOR_TEST );
	glDisable( GL_TEXTURE_2D );
	glDisable( GL_CULL_FACE );

	TSyncQueue Jobs;
	Jobs.Add( Win32::Init, "Win32::Init", FALSE );
	Jobs.Add( Opengl::Platform::Init, "Opengl::Platform::Init", FALSE );
	Jobs.Add( OpenglExtensions::Init, "OpenglExtensions::Init", FALSE );

	return Jobs.Update();
}


//---------------------------------------------------
//	platform/opengl shutdown
//---------------------------------------------------
SyncBool TLRender::Platform::Shutdown()	
{
	TSyncQueue Jobs;
	Jobs.Add( Opengl::Platform::Shutdown, "Opengl::Shutdown", FALSE );
	Jobs.Add( OpenglExtensions::Shutdown, "OpenglExtensions::Shutdown", FALSE );
	Jobs.Add( Win32::Shutdown, "Win32::Shutdown", FALSE );

	return Jobs.Update();
}


//---------------------------------------------------
//	opengl init
//---------------------------------------------------
SyncBool TLRender::Opengl::Platform::Init()
{
	//	need to create a window to initialise the opengl system
	if ( g_pSpareWindow )
		return SyncTrue;

	g_pSpareWindow = Win32::g_pFactory->GetInstance("Spare",TRUE,"OpenglWindow");
	if ( !g_pSpareWindow )
		return SyncFalse;

	TPtr<Win32::GWinControl> pNullParent;
	if ( !g_pSpareWindow->Init( pNullParent, g_pSpareWindow->DefaultFlags() ) )
	{
		Win32::g_pFactory->RemoveInstance( g_pSpareWindow->GetRef() );
		g_pSpareWindow = NULL;
		return SyncFalse;
	}
	
	//	init opengl state
	glDisable( GL_CULL_FACE );

	Debug_CheckForError();		

	//	all succeeded
	return SyncTrue;
}


//---------------------------------------------------
//	opengl shutdown
//---------------------------------------------------
SyncBool TLRender::Opengl::Platform::Shutdown()
{
	//	make sure the initial window is deleted if it hasn't been used
	g_pSpareWindow = NULL;

	return SyncTrue;
}


//---------------------------------------------------
//	check for opengl error - returns TRUE on error
//---------------------------------------------------
Bool TLRender::Opengl::Platform::Debug_CheckForError()
{
	//	can't detect errors until opengl initialised
	if ( g_OpenglInitialised != SyncTrue )
		return FALSE;

	//	won't see result anyway
	if ( !TLDebug::IsEnabled() )
		return FALSE;
	
	//	check for a GLError
	u16 Error = glGetError();
	
	//	no error
	if ( Error == GL_NO_ERROR )
		return FALSE;
	
	//	get error index
	TString ErrorString("Opengl error 0x%x: ");
	
	const char* pErrorString = NULL;
	switch ( Error )
	{
	case GL_NO_ERROR:			pErrorString = "GL_NO_ERROR: No error has been recorded.";	break;
	case GL_INVALID_ENUM:		pErrorString = "GL_INVALID_ENUM: An unacceptable value is specified for an enumerated argument.";	break;
	case GL_INVALID_VALUE:		pErrorString = "GL_INVALID_VALUE: A numeric argument is out of range.";	break;
	case GL_INVALID_OPERATION:	pErrorString = "GL_INVALID_OPERATION: The specified operation is not allowed in the current state.";	break;
	case GL_STACK_OVERFLOW:		pErrorString = "GL_STACK_OVERFLOW: This command would cause a stack overflow.";	break;
	case GL_STACK_UNDERFLOW:	pErrorString = "GL_STACK_UNDERFLOW: This command would cause a stack underflow.";	break;
	case GL_OUT_OF_MEMORY:		pErrorString = "GL_OUT_OF_MEMORY: There is not enough memory left to execute the command.";	break;
	case GL_TABLE_TOO_LARGE:	pErrorString = "GL_TABLE_TOO_LARGE: The specified table exceeds the implementation's maximum supported table size.";	break;
	default:					pErrorString = "GL_???: Unknown error.";	break;
	}
	
	ErrorString.Append( pErrorString );
	
	TLDebug_Break( ErrorString );
	
	return FALSE;
}

