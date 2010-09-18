#include "PCOpengl.h"


//---------------------------------------------------
//	opengl init
//---------------------------------------------------
bool Opengl::Platform::Init()
{
	//	enable sphere map generation
	glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
	glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);

	return true;
}


//---------------------------------------------------
//	opengl shutdown
//---------------------------------------------------
void Opengl::Platform::Shutdown()
{
}


//---------------------------------------------------
//	check for opengl error - returns TRUE on error
//---------------------------------------------------
Bool Opengl::Platform::Debug_CheckForError()
{
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

