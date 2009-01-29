/*------------------------------------------------------

	Render lib interface







-------------------------------------------------------*/
#pragma once

#include <TootleCore/TLTypes.h>
#include <TootleCore/TColour.h>

#include "TRendergraph.h"

namespace TLRender
{
	//	forward declaration of platform specific implementations
	namespace Platform
	{
		SyncBool	Init();
		SyncBool	Shutdown();
	}

	extern const TColour	g_DebugMissingColoursColour;
	extern u32				g_PolyCount;
	extern u32				g_VertexCount;

	namespace Opengl
	{
		FORCEINLINE Bool		Debug_CheckForError();

		void					Unbind();			//	unbind all verts/colours etc
		template<class TYPE>
		FORCEINLINE void		DrawPrimitives(u16 GLPrimType,const TArray<TYPE>* pPrimitivesArray);
		FORCEINLINE void		DrawPrimitives(u16 GLPrimType,const TArray<TArray<u16> >* pPrimitivesArray);
		FORCEINLINE void		DrawPrimitives(u16 GLPrimType,const TArray<TArray<u16> >* pPrimitivesArray,u32 Debug_LimitPrimSize);
		void					DrawPrimitivePoints(const TArray<float3>* pVertexes);

		void					SceneTransform(const TLMaths::TTransform& Transform);		//	transform scene

		//	scene settings - only calls platform implementations as required - inlined for speed
		FORCEINLINE void		EnableWireframe(Bool Enable);
		FORCEINLINE void		EnableAlpha(Bool Enable);
		FORCEINLINE void		EnableDepthRead(Bool Enable);
		FORCEINLINE void		EnableDepthWrite(Bool Enable);
		FORCEINLINE void		SetSceneAlpha(float Alpha);		//	if < 1 then it will enable alpha too
		FORCEINLINE void		SetLineWidth(float Width);
		FORCEINLINE void		SetPointSize(float Size);
		

		//	platform specific implementations - dumb - just do whatever is specified
		//	gr: if your code cannot resolve these functions, include the platform header 
		//		(eg "PC/PCRender.h") in your SOURCE FILE which uses it. This is to keep 
		//		it all inline and fast whilst still being able to "overload" it
		//	gr: now included below to get rid of the compile warnings on GCC
		namespace Platform
		{
			SyncBool				Init();						//	init opengl
			SyncBool				Shutdown();					//	cleanup opengl
			
			Bool					Debug_CheckForError();		//	check for opengl error - returns TRUE on error

			//Bool					BindFixedVertexes(const TArray<TLAsset::TFixedVertex>* pVertexes,TRefRef MeshRef);
			Bool					BindVertexes(const TArray<float3>* pVertexes,TRefRef MeshRef);
			Bool					BindColours(const TArray<TColour>* pColours,TRefRef MeshRef);
			void					DrawPrimitives(u16 GLPrimType,u32 IndexCount,const u16* pIndexData);	//	main renderer, just needs primitive type, and the data

			FORCEINLINE u16			GetPrimTypeTriangle();		//	GL_TRIANGLES
			FORCEINLINE u16			GetPrimTypeTristrip();		//	GL_TRIANGLE_STRIP
			FORCEINLINE u16			GetPrimTypeTrifan();		//	GL_TRIANGLE_FAN
			FORCEINLINE u16			GetPrimTypeLineStrip();		//	GL_LINE_STRIP
			FORCEINLINE u16			GetPrimTypePoint();			//	GL_POINTS

			FORCEINLINE void		EnableWireframe(Bool Enable);
			FORCEINLINE void		EnableAlpha(Bool Enable);
			FORCEINLINE void		EnableDepthRead(Bool Enable);
			FORCEINLINE void		EnableDepthWrite(Bool Enable);
			FORCEINLINE void		SetSceneColour(const TColour& Colour);
			FORCEINLINE void		SetLineWidth(float Width);
			FORCEINLINE void		SetPointSize(float Size);
		}
	};
}



//	gr: because I made the platform opengl calls inline (for super speed) we have to include the platform header
#if defined(TL_TARGET_PC)
	#include "PC/PCRender.h"
#elif defined(TL_TARGET_IPOD)
	#include "IPod/IPodRender.h"
#else
	#error unknown platform
#endif





FORCEINLINE Bool TLRender::Opengl::Debug_CheckForError()
{
#ifdef _DEBUG
	return Platform::Debug_CheckForError();
#else
	return FALSE;
#endif
}

FORCEINLINE void TLRender::Opengl::EnableWireframe(Bool Enable)
{
	static SyncBool g_SceneEnabled = SyncWait;
	SyncBool NewEnabled = Enable ? SyncTrue : SyncFalse;

	if ( NewEnabled != g_SceneEnabled )
	{
		Platform::EnableWireframe( Enable );
		g_SceneEnabled = NewEnabled;
	}
}


FORCEINLINE void TLRender::Opengl::EnableAlpha(Bool Enable)
{
	static SyncBool g_SceneEnabled = SyncWait;
	SyncBool NewEnabled = Enable ? SyncTrue : SyncFalse;

	if ( NewEnabled != g_SceneEnabled )
	{
		Platform::EnableAlpha( Enable );
		g_SceneEnabled = NewEnabled;
	}
}


FORCEINLINE void TLRender::Opengl::EnableDepthRead(Bool Enable)
{
	static SyncBool g_SceneEnabled = SyncWait;
	SyncBool NewEnabled = Enable ? SyncTrue : SyncFalse;

	if ( NewEnabled != g_SceneEnabled )
	{
		Platform::EnableDepthRead( Enable );
		g_SceneEnabled = NewEnabled;
	}
}


FORCEINLINE void TLRender::Opengl::EnableDepthWrite(Bool Enable)
{
	static SyncBool g_SceneEnabled = SyncWait;
	SyncBool NewEnabled = Enable ? SyncTrue : SyncFalse;

	if ( NewEnabled != g_SceneEnabled )
	{
		Platform::EnableDepthWrite( Enable );
		g_SceneEnabled = NewEnabled;
	}
}


//---------------------------------------------------
//	if < 1 then it will enable alpha too
//---------------------------------------------------
void TLRender::Opengl::SetSceneAlpha(float Alpha)
{
	static float g_SceneAlpha = 1.f;
	
	if ( g_SceneAlpha != Alpha )
	{
		Platform::SetSceneColour( TColour( 1.f, 1.f, 1.f, Alpha ) );
		EnableAlpha( Alpha < TLMaths::g_NearOne );
		g_SceneAlpha = Alpha;
	}
}


void TLRender::Opengl::SetLineWidth(float Width)
{
	static float g_SceneLineWidth = 1.f;
	
	if ( g_SceneLineWidth != Width )
	{
		Platform::SetLineWidth( Width );
		g_SceneLineWidth = Width;
	}
}


void TLRender::Opengl::SetPointSize(float Size)
{
	static float g_ScenePointSize = 1.f;
	
	if ( g_ScenePointSize != Size )
	{
		Platform::SetLineWidth( Size );
		g_ScenePointSize = Size;
	}
}




template<class TYPE>
FORCEINLINE void TLRender::Opengl::DrawPrimitives(u16 GLPrimType,const TArray<TYPE>* pPrimitivesArray)		
{
	if ( pPrimitivesArray && pPrimitivesArray->GetSize() )
	{
		const TYPE& FirstPrim = pPrimitivesArray->ElementAtConst(0);
		Platform::DrawPrimitives( GLPrimType, pPrimitivesArray->GetSize() * TYPE::GetSize(), FirstPrim.GetData() );	
	}
}


FORCEINLINE void TLRender::Opengl::DrawPrimitives(u16 GLPrimType,const TArray<TArray<u16> >* pPrimitivesArray)
{
	if ( pPrimitivesArray )
	{
		for ( u32 i=0;	i<pPrimitivesArray->GetSize();	i++ )
		{
			const TArray<u16>& PrimArray = pPrimitivesArray->ElementAtConst(i);
			Platform::DrawPrimitives( GLPrimType, PrimArray.GetSize(), PrimArray.GetData() );
		}
	}
}


FORCEINLINE void TLRender::Opengl::DrawPrimitives(u16 GLPrimType,const TArray<TArray<u16> >* pPrimitivesArray,u32 Debug_LimitPrimSize)
{
	if ( pPrimitivesArray )
	{
		for ( u32 i=0;	i<pPrimitivesArray->GetSize();	i++ )
		{
			const TArray<u16>& PrimArray = pPrimitivesArray->ElementAtConst(i);
			u32 PrimSize = Debug_LimitPrimSize < PrimArray.GetSize() ? Debug_LimitPrimSize : PrimArray.GetSize();
			Platform::DrawPrimitives( GLPrimType, PrimSize, PrimArray.GetData() );
		}
	}
}

