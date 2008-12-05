#include "IPodRender.h"
#include "../TLRender.h"

#import <OpenGLES/EAGL.h>
#import <OpenGLES/ES1/gl.h>
#import <OpenGLES/ES1/glext.h>
#import <OpenGLES/EAGLDrawable.h>

#include <TootleAsset/TMesh.h>
#include <TootleCore/TKeyArray.h>


namespace TLRender
{
	namespace Platform
	{
		namespace Opengl
		{
			Bool		g_State_ScissorTest = TRUE;
			Bool		g_State_DepthRead = TRUE;
			Bool		g_State_DepthWrite = TRUE;
			
			TKeyArray<TRef,TVertexBufferObject>		g_VertexBufferObjectCache;	//	for every mesh asset we have a VBO
			const TArray<TLAsset::TFixedVertex>*	g_pBoundFixedVertexes = NULL;
			const TArray<float3>*					g_pBoundVertexes = NULL;
		}
	}
}


Bool TLRender::Platform::BeginDraw()
{
	TLRender::g_PolyCount = 0;
	TLRender::g_VertexCount = 0;
	
	return TRUE;
}

void TLRender::Platform::EndDraw()
{
//	NSLog(@"p: %d v: %d",g_PolyCount,g_VertexCount);
//	TLDebug_Print( TString("Drawn %d polys, %d verts", g_PolyCount, g_VertexCount ) );
}


//---------------------------------------------------
//	select VBO
//---------------------------------------------------
Bool TLRender::Platform::Opengl::TVertexBufferObject::BindBuffer(u32 BufferObject)
{
	//	selecting zero (no buffer) will select no buffer
	glBindBuffer( GL_ARRAY_BUFFER, BufferObject );
	return (BufferObject != 0);
}


//---------------------------------------------------
//	bind data to VBO - returns FALSE if failed, and/or buffer no longer exists
//---------------------------------------------------
Bool TLRender::Platform::Opengl::TVertexBufferObject::UploadBuffer(const void* pData,u32 DataSize,u32& BufferObject)
{
	if ( DataSize == 0 )
		pData = NULL;

	//	do we need to update the data?
	Bool UpdateData = FALSE;

	//	need to alloc VBO
	if ( BufferObject == 0 )
	{
		//	not allocated, and no data, do nothing
		if ( !pData )
			return FALSE;
		
		//	alloc buffer
		glGenBuffers( 1, &BufferObject );

		//	new buffer, so need to update data
		UpdateData = TRUE;
	}
	else
	{
		//	buffer already exists, and we're NULL'ing the data, so needs updating
		if ( !pData )
			UpdateData = TRUE;
	}

	//	bind VBO
	if ( !BindBuffer( BufferObject ) )
		return FALSE;

	//	upload data
	if ( UpdateData )
	{
		glBufferData( GL_ARRAY_BUFFER, DataSize, pData, GL_STATIC_DRAW );
	}
	
	//	data has been deleted, mark VBO as gone, and bind to no buffer
	if ( !pData )
	{
		BufferObject = 0;
		glBindBuffer( GL_ARRAY_BUFFER, BufferObject );
	}
	
	return (BufferObject != 0);
}


//---------------------------------------------------
//	delete VBO data
//---------------------------------------------------
void TLRender::Platform::Opengl::TVertexBufferObject::Delete()
{
	//	bind to NULL to delete buffers
	UploadVertexBuffer( NULL, 0 );
	UploadColourBuffer( NULL, 0 );
}



//---------------------------------------------------
//	platform/opengl initialisation
//---------------------------------------------------
SyncBool TLRender::Platform::Init()
{
	//	setup default states
	
	glEnable( GL_SCISSOR_TEST );
	TLRender::Platform::Opengl::g_State_ScissorTest = TRUE;
	
	glEnable( GL_DEPTH_TEST );
	TLRender::Platform::Opengl::g_State_DepthRead = TRUE;
	
	glDepthMask( GL_TRUE );
	TLRender::Platform::Opengl::g_State_DepthWrite = TRUE;

	glDisable( GL_TEXTURE_2D );
	glDisable( GL_CULL_FACE );

	return SyncTrue;
}


//---------------------------------------------------
//	platform/opengl update
//---------------------------------------------------
SyncBool TLRender::Platform::Update()
{
	return SyncTrue;	
}


//---------------------------------------------------
//	platform/opengl shutdown
//---------------------------------------------------
SyncBool TLRender::Platform::Shutdown()	
{
	return SyncTrue;
}


//---------------------------------------------------
//	bind verts
//---------------------------------------------------
Bool TLRender::Platform::Opengl::BindFixedVertexes(const TArray<TLAsset::TFixedVertex>* pVertexes,TRefRef MeshRef)
{
	//	remove pointer if empty
	if ( pVertexes && pVertexes->GetSize() == 0 )
		pVertexes = NULL;

	//	already bound to this
	if ( pVertexes == g_pBoundFixedVertexes )
		return TRUE;
	
	g_pBoundVertexes = NULL;
	g_pBoundFixedVertexes = pVertexes;

	
	//	unbind
	if ( !pVertexes )
	{
		//	unbind any VBO's
		TVertexBufferObject::BindNone();

		glDisableClientState( GL_VERTEX_ARRAY );
		glDisableClientState( GL_COLOR_ARRAY );
		Opengl::Debug_CheckForError();		
		return TRUE;
	}
	
	const TLAsset::TFixedVertex* pFirstVertex = &pVertexes->ElementAtConst(0);

	//	add VBO for this mesh, if it exists it will be returned
	TVertexBufferObject* pVBO = NULL;
	if ( MeshRef.IsValid() )
		pVBO = g_VertexBufferObjectCache.Add( MeshRef, TVertexBufferObject(), FALSE );

	if ( pVBO )
		if ( !pVBO->UploadVertexBuffer( pFirstVertex, pVertexes->GetDataSize() ) )
			pVBO = NULL;

	if ( pVBO )
	{
		pFirstVertex = NULL;
	}
	else
	{
		TVertexBufferObject::BindNone();
	}
	
	//	enable arrays
	glEnableClientState( GL_VERTEX_ARRAY );
	//glVertexPointer( 3, GL_FLOAT, sizeof(TLAsset::TFixedVertex), &pFirstVertex->m_Position );
	glVertexPointer( 3, GL_FIXED, sizeof(TLAsset::TFixedVertex), &pFirstVertex->m_PositionFixed );
	Opengl::Debug_CheckForError();

	glEnableClientState( GL_COLOR_ARRAY );
	glColorPointer( 4, GL_UNSIGNED_BYTE, sizeof(TLAsset::TFixedVertex), &pFirstVertex->m_Colour );
	Opengl::Debug_CheckForError();
	
	return TRUE;
	
}


//---------------------------------------------------
//	bind verts
//---------------------------------------------------
Bool TLRender::Platform::Opengl::BindVertexes(const TArray<float3>* pVertexes,TRefRef MeshRef)
{
	//	remove pointer if empty
	if ( pVertexes && pVertexes->GetSize() == 0 )
		pVertexes = NULL;

	//	already bound to this
	if ( pVertexes == g_pBoundVertexes )
		return TRUE;
	
	g_pBoundFixedVertexes = NULL;
	g_pBoundVertexes = pVertexes;
	
	//	unbind
	if ( !pVertexes )
	{
		//	unbind any VBO's
		TVertexBufferObject::BindNone();

		glDisableClientState( GL_VERTEX_ARRAY );
		Opengl::Debug_CheckForError();		
		return TRUE;
	}
	
	//	enable texcoord array
	glEnableClientState( GL_VERTEX_ARRAY );

	//	add VBO for this mesh, if it exists it will be returned
	TVertexBufferObject* pVBO = NULL;
	if ( MeshRef.IsValid() )
		pVBO = g_VertexBufferObjectCache.Add( MeshRef, TVertexBufferObject(), FALSE );

	if ( pVBO )
	{
		if ( !pVBO->UploadVertexBuffer( pVertexes->GetData(), pVertexes->GetDataSize() ) )
			pVBO = NULL;
	}
	
	if ( !pVBO )
	{
		TVertexBufferObject::BindNone();
	}

	//	bind
	glVertexPointer( 3, GL_FLOAT, 0, pVBO ? NULL : pVertexes->GetData() );
	Opengl::Debug_CheckForError();		
	
	return TRUE;
}


//---------------------------------------------------
//	bind colours
//---------------------------------------------------
Bool TLRender::Platform::Opengl::BindColours(const TArray<TColour>* pColours,TRefRef MeshRef)
{
	//	remove pointer if empty
	if ( pColours && pColours->GetSize() == 0 )
		pColours = NULL;
	
	//	unbind
	if ( !pColours )
	{
		//	unbind any VBO's
		TVertexBufferObject::BindNone();

		glDisableClientState( GL_COLOR_ARRAY );
		Opengl::Debug_CheckForError();
		return TRUE;
	}
	
	//	enable texcoord array
	glEnableClientState( GL_COLOR_ARRAY );

	//	add VBO for this mesh, if it exists it will be returned
	TVertexBufferObject* pVBO = NULL;
	if ( MeshRef.IsValid() )
		pVBO = g_VertexBufferObjectCache.Add( MeshRef, TVertexBufferObject(), FALSE );

	if ( pVBO )
	{
		if ( !pVBO->UploadColourBuffer( pColours->GetData(), pColours->GetDataSize() ) )
			pVBO = NULL;
	}

	if ( !pVBO )
	{
		TVertexBufferObject::BindNone();
	}
	
	//	bind
	glColorPointer( 4, GL_FLOAT, 0, pVBO ? NULL : pColours->GetData() );
	
	Opengl::Debug_CheckForError();
	
	return TRUE;
}


//---------------------------------------------------
//	unbind everything
//---------------------------------------------------
void TLRender::Platform::Opengl::Unbind(TRefRef MeshRef)
{
	//	unbind attribs
	BindFixedVertexes( NULL, MeshRef );
	BindVertexes( NULL, MeshRef );
	BindColours( NULL, MeshRef );

	//	unbind any VBO's
	TVertexBufferObject::BindNone();
	
	Opengl::Debug_CheckForError();
}


//---------------------------------------------------
//	check for opengl error - returns TRUE on error
//---------------------------------------------------
Bool TLRender::Platform::Opengl::Debug_CheckForError()
{
	//	too slow on ipod
	return FALSE;
/*
	//	glGetError very expensive on ipod, debug only
	#if !defined(_DEBUG)
	{
		return FALSE;
	}
	#endif
	
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
//		case GL_TABLE_TOO_LARGE:	pErrorString = "GL_TABLE_TOO_LARGE: The specified table exceeds the implementation's maximum supported table size.";	break;
		default:					pErrorString = "GL_???: Unknown error.";	break;
	}
	
	ErrorString.Append( pErrorString );
	
	TLDebug_Break( ErrorString );
	
	return FALSE;
 */
}




//-----------------------------------------------------------
//	main renderer, just needs primitive type, and the data
//-----------------------------------------------------------
void TLRender::Platform::Opengl::DrawPrimitives(u32 GLPrimType,u32 IndexCount,const u16* pIndexData)
{
	//	no data to render
	if ( !IndexCount || !pIndexData )
		return;

#ifdef _DEBUG
	u32 MaxVerts = 0;
	if ( g_pBoundFixedVertexes )
	{
		MaxVerts = g_pBoundFixedVertexes->GetSize();		
	}
	else if ( g_pBoundVertexes )
	{
		MaxVerts = g_pBoundVertexes->GetSize();		
	}
	else
	{
		//	no verts bound!
		TLDebug_Break("Vertexes not bound!");
		return;
	}

	//	check bounds of data
	for ( u32 i=0;	i<IndexCount;	i++ )
	{
		if ( pIndexData[i] >= MaxVerts )
		{
			TLDebug_Break("Primitive data exceeds vertex bounds");
			return;
		}
	}
#endif
	
	//	draw
	glDrawElements( GLPrimType, IndexCount, GL_UNSIGNED_SHORT, pIndexData );

	//	inc poly counter
	TLRender::g_VertexCount += IndexCount;
	TLRender::g_PolyCount += IndexCount / 3;
	
	Opengl::Debug_CheckForError();
}


	
void UpdateStateEnabled(Bool& StateEnabled,Bool Enable,u32 State)
{
	if ( StateEnabled != Enable )
	{
		StateEnabled = Enable;
			
		if ( StateEnabled )
			glEnable( State );
		else
			glDisable( State );
	}
}

	
void TLRender::Platform::Opengl::EnableScissorTest(Bool Enable)
{
	UpdateStateEnabled( TLRender::Platform::Opengl::g_State_ScissorTest, Enable, GL_SCISSOR_TEST );
}
	
	
void TLRender::Platform::Opengl::EnableDepthRead(Bool Enable)
{
	UpdateStateEnabled( TLRender::Platform::Opengl::g_State_DepthRead, Enable, GL_DEPTH_TEST );
}
	
	
void TLRender::Platform::Opengl::EnableDepthWrite(Bool Enable)
{
	if ( TLRender::Platform::Opengl::g_State_DepthWrite != Enable )
	{
		TLRender::Platform::Opengl::g_State_DepthWrite = Enable;
		glDepthMask( Enable ? GL_TRUE : GL_FALSE );
	}		
}


