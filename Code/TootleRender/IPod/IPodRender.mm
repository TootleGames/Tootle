#include "IPodRender.h"
#include "../TLRender.h"

#import <OpenGLES/EAGL.h>
#import <OpenGLES/ES1/glext.h>
#import <OpenGLES/EAGLDrawable.h>

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
		namespace Platform
		{
			const TArray<float3>*	g_pBoundVertexes = NULL;
		}
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


	return SyncTrue;
}



//---------------------------------------------------
//	platform/opengl shutdown
//---------------------------------------------------
SyncBool TLRender::Platform::Shutdown()	
{
	return SyncTrue;
}

/*
//---------------------------------------------------
//	bind verts
//---------------------------------------------------
Bool TLRender::Platform::Opengl::BindFixedVertexes(const TArray<TLAsset::TFixedVertex>* pVertexes)
{
	//	remove pointer if empty
	if ( pVertexes && pVertexes->GetSize() == 0 )
		pVertexes = NULL;

	//	already bound to this
//	if ( pVertexes == g_pBoundFixedVertexes )
//		return TRUE;
	
	g_pBoundVertexes = NULL;
	g_pBoundFixedVertexes = pVertexes;

	
	//	unbind
	if ( !pVertexes )
	{
		glDisableClientState( GL_VERTEX_ARRAY );
		glDisableClientState( GL_COLOR_ARRAY );
		Debug_CheckForError();		
		return TRUE;
	}
	
	const TLAsset::TFixedVertex* pFirstVertex = &pVertexes->ElementAtConst(0);
	
	//	enable arrays
	glEnableClientState( GL_VERTEX_ARRAY );
	//glVertexPointer( 3, GL_FLOAT, sizeof(TLAsset::TFixedVertex), &pFirstVertex->m_Position );
	glVertexPointer( 3, GL_FIXED, sizeof(TLAsset::TFixedVertex), &pFirstVertex->m_PositionFixed );
	Debug_CheckForError();

	glEnableClientState( GL_COLOR_ARRAY );
	glColorPointer( 4, GL_UNSIGNED_BYTE, sizeof(TLAsset::TFixedVertex), &pFirstVertex->m_Colour );
	Debug_CheckForError();
	
	return TRUE;
	
}
*/

//---------------------------------------------------
//	bind verts
//---------------------------------------------------
Bool TLRender::Opengl::Platform::BindVertexes(const TArray<float3>* pVertexes)
{
	//	remove pointer if empty
	if ( pVertexes && pVertexes->GetSize() == 0 )
		pVertexes = NULL;

	//	already bound to this
//	if ( pVertexes == g_pBoundVertexes )
//		return TRUE;
	
//	g_pBoundFixedVertexes = NULL;
	g_pBoundVertexes = pVertexes;
	
	//	unbind
	if ( !pVertexes )
	{
		glDisableClientState( GL_VERTEX_ARRAY );
		Debug_CheckForError();		
		return TRUE;
	}
	
	//	enable texcoord array
	glEnableClientState( GL_VERTEX_ARRAY );

	//	bind
	glVertexPointer( 3, GL_FLOAT, 0, pVertexes->GetData() );
	Debug_CheckForError();		
	
	return TRUE;
}


//---------------------------------------------------
//	bind colours
//---------------------------------------------------
Bool TLRender::Opengl::Platform::BindColours(const TArray<TColour>* pColours)
{
	//	remove pointer if empty
	if ( pColours && pColours->GetSize() == 0 )
		pColours = NULL;
	
	//	already bound to this
//	if ( pColours == g_pBoundColours )
//		return TRUE;

	//	unbind
	if ( !pColours )
	{
		glDisableClientState( GL_COLOR_ARRAY );
		Opengl::Platform::Debug_CheckForError();
		return TRUE;
	}
	
	//	enable texcoord array
	glEnableClientState( GL_COLOR_ARRAY );
	
	//	bind
	glColorPointer( 4, GL_FLOAT, 0, pColours->GetData() );
	
	Debug_CheckForError();
	
	return TRUE;
}



//-----------------------------------------------------------
//	main renderer, just needs primitive type, and the data
//-----------------------------------------------------------
void TLRender::Opengl::Platform::DrawPrimitives(u16 GLPrimType,u32 IndexCount,const u16* pIndexData)
{
	//	no data to render
	if ( !IndexCount || !pIndexData )
		return;

	//	make sure we don't attempt to render vertexes that aren't bound
#ifdef _DEBUG
	if ( g_pBoundVertexes )
	{
		u32 MaxVertexIndex = g_pBoundVertexes->GetLastIndex();
		for ( u32 i=0;	i<IndexCount;	i++ )
		{
			if ( pIndexData[i] > MaxVertexIndex )
			{
				TLDebug_Break("Primitive contains vertex index out of the range of vertexes that were bound");
			}
		}
	}
#endif
	
	//	draw
	glDrawElements( GLPrimType, IndexCount, GL_UNSIGNED_SHORT, pIndexData );

	//	inc poly counter
	TLRender::g_VertexCount += IndexCount;
	TLRender::g_PolyCount += IndexCount / 3;

	Debug_CheckForError();
}

//---------------------------------------------------
//	bind UV's
//---------------------------------------------------
Bool TLRender::Opengl::Platform::BindUVs(const TArray<float2>* pUVs)
{
	//	remove pointer if empty
	if ( pUVs && pUVs->GetSize() == 0 )
		pUVs = NULL;
	
	//	already bound to this
//	if ( pUVs == g_pBoundUVs )
//		return TRUE;

	//	unbind
	if ( !pUVs )
	{
		glDisableClientState( GL_TEXTURE_COORD_ARRAY );
		Opengl::Platform::Debug_CheckForError();
		return TRUE;
	}

	//	enable texcoord array
	glEnableClientState( GL_TEXTURE_COORD_ARRAY );
	
	//	bind
	glTexCoordPointer( 2, GL_FLOAT, 0, pUVs->GetData() );
	
	Debug_CheckForError();
	
	return TRUE;
}


//---------------------------------------------------
//	check for opengl error - returns TRUE on error
//---------------------------------------------------
Bool TLRender::Opengl::Platform::Debug_CheckForError()
{
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
		default:					pErrorString = "GL_???: Unknown error.";	break;
	}
	
	ErrorString.Append( pErrorString );
	
	TLDebug_Break( ErrorString );
	
	return FALSE;
}
