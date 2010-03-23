#include "../TLRender.h"
#include "MacRender.h"
#include <TootleCore/TSyncQueue.h>
#include <TootleAsset/TMesh.h>
#include <TootleCore/TColour.h>
#include <TootleCore/TKeyArray.h>


//---------------------------------------------------
//	globals
//---------------------------------------------------
namespace TLRender
{
	namespace Opengl
	{
		SyncBool	g_Initialised = SyncWait;	//	has opengl been initialied? (ie first Init() after context is created)
	}
}


//---------------------------------------------------
//	opengl init
//---------------------------------------------------
SyncBool TLRender::Opengl::Platform::Init()
{
	//	already initalised
	if ( g_Initialised != SyncWait )
		return g_Initialised;

	//	gr: should check for an active context here (the caller should have created 
	//		a context and made it current. Otherwise the commands below will fail
	
	//	init opengl state
	glDisable( GL_CULL_FACE );
	glEnable( GL_SCISSOR_TEST );
	glDisable( GL_TEXTURE_2D );
	
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
	
	Debug_CheckForError();		

	//	all succeeded
	return SyncTrue;
}


//---------------------------------------------------
//	opengl shutdown
//---------------------------------------------------
SyncBool TLRender::Opengl::Platform::Shutdown()
{
	//	gr: any shutdown required here? just need the opengl canvas's to free their contexts....
	//	maybe we need a ref counter to make sure there are no more context's left
	
	//	back to uninitialised gl state
	g_Initialised = SyncWait;
	
	return SyncTrue;
}


//---------------------------------------------------
//	check for opengl error - returns TRUE on error
//---------------------------------------------------
Bool TLRender::Opengl::Platform::Debug_CheckForError()
{
	//	can't detect errors until opengl initialised
	if ( g_Initialised != SyncTrue )
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

