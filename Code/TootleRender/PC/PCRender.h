/*-----------------------------------------------------

	Core file for Platform specific Render lib - 
	essentially the opengl interface


-------------------------------------------------------*/
#pragma once


//	include core lib header
#include <TootleCore/TLCore.h>
#include <TootleCore/TPtr.h>

//	include opengl stuff
#pragma comment( lib, "Opengl32.lib" )
#pragma comment( lib, "glu32.lib" )

#include "glsdk/gl.h"
#include "glsdk/glext.h"
#include "glsdk/wglext.h"


//	forward declaration
class TColour;

namespace TLAsset
{
	class TShader;
	class TFixedVertex;
};



namespace TLRender
{
	namespace Platform
	{
		SyncBool		Init();			//	platform/opengl initialisation
		SyncBool		Update();		//	platform/opengl update
		SyncBool		Shutdown();		//	platform/opengl shutdown

		extern SyncBool	g_OpenglInitialised;

		namespace Opengl
		{
			class TVertexBufferObject;
			
			SyncBool	Init();		//	init opengl
			SyncBool	Shutdown();	//	cleanup opengl

			Bool		Debug_CheckForError();		//	check for opengl error - returns TRUE on error

			Bool		BindFixedVertexes(const TArray<TLAsset::TFixedVertex>* pVertexes,TRefRef MeshRef);
			Bool		BindVertexes(const TArray<float3>* pVertexes,TRefRef MeshRef);
			Bool		BindColours(const TArray<TColour>* pColours,TRefRef MeshRef);
			void		Unbind(TRefRef MeshRef);

			void			DrawPrimitives(u32 GLPrimType,u32 IndexCount,const u16* pIndexData);	//	main renderer, just needs primitive type, and the data
			template<class TYPE>
			inline void		DrawPrimitives(u32 GLPrimType,const TArray<TYPE>* pPrimitivesArray);
			inline void		DrawPrimitives(u32 GLPrimType,const TArray<TArray<u16> >* pPrimitivesArray);
			inline void		DrawPrimitives(u32 GLPrimType,const TArray<TArray<u16> >* pPrimitivesArray,u32 Debug_LimitPrimSize);

			void			ResetPolyCount();	//	reset counter of polygons rendered back to zero
			u32				GetPolyCount();		//	get number of polygons rendered

		}
	}
}




class TLRender::Platform::Opengl::TVertexBufferObject
{
public:
	TVertexBufferObject() : m_VertexVBO(0), m_ColourVBO(0)	{}

	Bool		BindVertexVBO()											{	return BindBuffer( m_VertexVBO );	}
	Bool		BindColourVBO()											{	return BindBuffer( m_ColourVBO );	}
	Bool		UploadVertexBuffer(const void* pData,u32 DataSize)		{	return UploadBuffer( pData, DataSize, m_VertexVBO );	}
	Bool		UploadColourBuffer(const void* pData,u32 DataSize)		{	return UploadBuffer( pData, DataSize, m_ColourVBO );	}
	void		Delete();												//	delete VBO data
	static void	BindNone()												{	BindBuffer( 0 );	}

private:
	static Bool	BindBuffer(u32 BufferObject);									//	bind VBO - returns FALSE if no VBO
	Bool		UploadBuffer(const void* pData,u32 DataSize,u32& BufferObject);	//	bind data to VBO - returns FALSE if failed, and/or buffer no longer exists

public:
	u32		m_VertexVBO;
	u32		m_ColourVBO;
};


template<class TYPE>
inline void TLRender::Platform::Opengl::DrawPrimitives(u32 GLPrimType,const TArray<TYPE>* pPrimitivesArray)		
{
	if ( !pPrimitivesArray )
		return;
	
	if ( !pPrimitivesArray->GetSize() )
		return;

	const TYPE& FirstPrim = pPrimitivesArray->ElementAtConst(0);
	DrawPrimitives( GLPrimType, pPrimitivesArray->GetSize() * TYPE::GetSize(), FirstPrim.GetData() );	
}


inline void TLRender::Platform::Opengl::DrawPrimitives(u32 GLPrimType,const TArray<TArray<u16> >* pPrimitivesArray)
{
	if ( !pPrimitivesArray )
		return;

	for ( u32 i=0;	i<pPrimitivesArray->GetSize();	i++ )
	{
		const TArray<u16>& PrimArray = pPrimitivesArray->ElementAtConst(i);
		DrawPrimitives( GLPrimType, PrimArray.GetSize(), PrimArray.GetData() );
	}
}


inline void TLRender::Platform::Opengl::DrawPrimitives(u32 GLPrimType,const TArray<TArray<u16> >* pPrimitivesArray,u32 Debug_LimitPrimSize)
{
	if ( !pPrimitivesArray )
		return;

	for ( u32 i=0;	i<pPrimitivesArray->GetSize();	i++ )
	{
		const TArray<u16>& PrimArray = pPrimitivesArray->ElementAtConst(i);
		u32 PrimSize = Debug_LimitPrimSize < PrimArray.GetSize() ? Debug_LimitPrimSize : PrimArray.GetSize();
		DrawPrimitives( GLPrimType, PrimSize, PrimArray.GetData() );
	}
}

