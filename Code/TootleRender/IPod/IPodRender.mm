#include "IPodRender.h"
#include "../TLRender.h"

#import <OpenGLES/EAGL.h>
#import <OpenGLES/ES1/gl.h>
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

			TKeyArray<TRef,TVertexBufferObject>		g_VertexBufferObjectCache;	//	for every mesh asset we have a VBO
			const TArray<TLAsset::TFixedVertex>*	g_pBoundFixedVertexes = NULL;
			const TArray<float3>*					g_pBoundVertexes = NULL;
		}
	}
}




//---------------------------------------------------
//	select VBO
//---------------------------------------------------
Bool TLRender::Opengl::Platform::TVertexBufferObject::BindBuffer(u32 BufferObject)
{
	//	selecting zero (no buffer) will select no buffer
	glBindBuffer( GL_ARRAY_BUFFER, BufferObject );
	return (BufferObject != 0);
}


//---------------------------------------------------
//	bind data to VBO - returns FALSE if failed, and/or buffer no longer exists
//---------------------------------------------------
Bool TLRender::Opengl::Platform::TVertexBufferObject::UploadBuffer(const void* pData,u32 DataSize,u32& BufferObject)
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
void TLRender::Opengl::Platform::TVertexBufferObject::Delete()
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
Bool TLRender::Platform::Opengl::BindFixedVertexes(const TArray<TLAsset::TFixedVertex>* pVertexes,TRefRef MeshRef)
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
		//	unbind any VBO's
		TVertexBufferObject::BindNone();

		glDisableClientState( GL_VERTEX_ARRAY );
		glDisableClientState( GL_COLOR_ARRAY );
		Debug_CheckForError();		
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
Bool TLRender::Opengl::Platform::BindVertexes(const TArray<float3>* pVertexes,TRefRef MeshRef)
{
	//	remove pointer if empty
	if ( pVertexes && pVertexes->GetSize() == 0 )
		pVertexes = NULL;

	//	already bound to this
	if ( pVertexes == g_pBoundVertexes )
		return TRUE;
	
//	g_pBoundFixedVertexes = NULL;
	g_pBoundVertexes = pVertexes;
	
	//	unbind
	if ( !pVertexes )
	{
		//	unbind any VBO's
		TVertexBufferObject::BindNone();

		glDisableClientState( GL_VERTEX_ARRAY );
		Debug_CheckForError();		
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
	Debug_CheckForError();		
	
	return TRUE;
}


//---------------------------------------------------
//	bind colours
//---------------------------------------------------
Bool TLRender::Opengl::Platform::BindColours(const TArray<TColour>* pColours,TRefRef MeshRef)
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
		//	unbind any VBO's
		TVertexBufferObject::BindNone();

		glDisableClientState( GL_COLOR_ARRAY );
		Opengl::Platform::Debug_CheckForError();
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

