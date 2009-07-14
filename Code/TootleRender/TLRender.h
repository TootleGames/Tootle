/*------------------------------------------------------

	Render lib interface







-------------------------------------------------------*/
#pragma once

#include <TootleCore/TLTypes.h>
#include <TootleCore/TColour.h>

#include "TRendergraph.h"

namespace TLRender
{
	enum TScreenShape
	{
		ScreenShape_Portrait = 0,	//	portrait
		ScreenShape_WideLeft,		//	widescreen, portrait but rendered sideways to the left
		ScreenShape_WideRight,		//	widescreen, portrait but rendered sideways to the right
		ScreenShape_Wide,			//	widescreen
	};
	
	FORCEINLINE const TLMaths::TAngle&	GetScreenAngle(TScreenShape ScreenShape);	//	get the angle that the screenshape is rotated to
	

	//	forward declaration of platform specific implementations
	namespace Platform
	{
		SyncBool	Init();
		SyncBool	Shutdown();
	}

	extern u32				g_PolyCount;
	extern u32				g_VertexCount;

	namespace Opengl
	{
		FORCEINLINE Bool		Debug_CheckForError();

		Bool					BindVertexes(const TArray<float3>* pVertexes);
		Bool					BindColours(const TArray<TColour>* pColours);
		Bool					BindColours(const TArray<TColour24>* pColours);
		Bool					BindColours(const TArray<TColour32>* pColours);
		Bool					BindColours(const TArray<TColour64>* pColours);
		FORCEINLINE Bool		BindColoursNull()										{	return BindColours( (const TArray<TColour>*)NULL );	}	//	wrapper for ambigious use of NULL
		Bool					BindUVs(const TArray<float2>* pUVs);
		Bool					BindTexture(const TLAsset::TTexture* pTexture);			//	bind texture - returns FALSE if no texture is bound (either fail or expected)
		void					Unbind();			//	unbind all verts/colours etc
		
		void					DrawPrimitives(u16 GLPrimType,u32 IndexCount,const u16* pIndexData);		//	main renderer, just needs primitive type, and the data
		template<class TYPE>
		FORCEINLINE void		DrawPrimitives(u16 GLPrimType,const TArray<TYPE>& PrimitivesArray);			//	wrapper to pull out the index data for different types
		FORCEINLINE void		DrawPrimitives(u16 GLPrimType,const TArray<TArray<u16> >& PrimitivesArray);	//	specialisation for arrays of u16s (ie. strips)
		void					DrawPrimitivePoints(const TArray<float3>* pVertexes);

		void					SceneTransform(const TLMaths::TTransform& Transform,const TLMaths::TMatrix* pMatrix=NULL);	//	transform scene
		void					SceneRotate(const TLMaths::TAngle& Rotation,const float3& Axis);	//	eular rotation on the scene - wrapper for glRotatef

		void					GetViewportSize(Type4<s32>& ViewportSize,const Type4<s32>& ViewportTargetMaxSize,const Type4<s32>& RenderTargetSize,const Type4<s32>& RenderTargetMaxSize,TScreenShape ScreenShape);	//	get render target's viewport size from the size and the screen size
	
		//	scene settings - only calls platform implementations as required - inlined for speed
		FORCEINLINE void		EnableWireframe(Bool Enable);
		FORCEINLINE void		EnableAlpha(Bool Enable,Bool AddBlending=FALSE);
		FORCEINLINE void		EnableAddBlending(Bool Enable);
		FORCEINLINE void		EnableDepthRead(Bool Enable);
		FORCEINLINE void		EnableDepthWrite(Bool Enable);
		FORCEINLINE void		EnableScissor(Bool Enable);
		FORCEINLINE void		SetScissor(u32 x, u32 y, u32 width, u32 height);
		FORCEINLINE void		SetSceneColour(const TColour& Colour,Bool ForceEnableAlpha=FALSE,Bool AddBlending=FALSE);		//	if alpha < 1 then it will enable alpha too
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

			FORCEINLINE u16			GetPrimTypeTriangle();		//	GL_TRIANGLES
			FORCEINLINE u16			GetPrimTypeTristrip();		//	GL_TRIANGLE_STRIP
			FORCEINLINE u16			GetPrimTypeTrifan();		//	GL_TRIANGLE_FAN
			FORCEINLINE u16			GetPrimTypeLinestrip();		//	GL_LINE_STRIP
			FORCEINLINE u16			GetPrimTypeLine();			//	GL_LINES
			FORCEINLINE u16			GetPrimTypePoint();			//	GL_POINTS

			FORCEINLINE void		EnableWireframe(Bool Enable);
			FORCEINLINE void		EnableAlpha(Bool Enable);
			FORCEINLINE void		EnableAddBlending(Bool Enable);
			FORCEINLINE void		EnableDepthRead(Bool Enable);
			FORCEINLINE void		EnableDepthWrite(Bool Enable);
			FORCEINLINE void		EnableScissor(Bool Enable);
			FORCEINLINE void		SetScissor(u32 x, u32 y, u32 width, u32 height);
			FORCEINLINE void		SetSceneColour(const TColour& Colour);
			FORCEINLINE void		SetLineWidth(float Width);
			FORCEINLINE void		SetPointSize(float Size);
		}
	};
}



//	gr: because I made the platform opengl calls inline (for super speed) we have to include the platform header
#if defined(TL_TARGET_PC) && defined(_MSC_EXTENSIONS)
	#include "PC/PCRender.h"
#elif defined(TL_TARGET_PC) 
	//	ansi build can't include any headers
#elif defined(TL_TARGET_IPOD)
	#include "IPod/IPodRender.h"
#elif defined(TL_TARGET_MAC)
	#include "Mac/MacRender.h"
#else
	#error unknown platform
#endif





FORCEINLINE Bool TLRender::Opengl::Debug_CheckForError()
{
#if defined(_DEBUG)&&!defined(TL_TARGET_IPOD)
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


FORCEINLINE void TLRender::Opengl::EnableAlpha(Bool Enable,Bool AddBlending)
{
	static SyncBool g_SceneEnabled = SyncWait;
	SyncBool NewEnabled = Enable ? SyncTrue : SyncFalse;

	if ( NewEnabled != g_SceneEnabled )
	{
		Platform::EnableAlpha( Enable );
		g_SceneEnabled = NewEnabled;
	}

	//	set correct blending mode
	if ( Enable )
		Platform::EnableAddBlending( AddBlending );
}

FORCEINLINE void TLRender::Opengl::EnableAddBlending(Bool Enable)
{
	static SyncBool g_SceneEnabled = SyncWait;
	SyncBool NewEnabled = Enable ? SyncTrue : SyncFalse;

	if ( NewEnabled != g_SceneEnabled )
	{
		Platform::EnableAddBlending( Enable );
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


FORCEINLINE void TLRender::Opengl::EnableScissor(Bool Enable)
{
	static SyncBool g_SceneEnabled = SyncWait;
	SyncBool NewEnabled = Enable ? SyncTrue : SyncFalse;

	if ( NewEnabled != g_SceneEnabled )
	{
		Platform::EnableScissor( Enable );
		g_SceneEnabled = NewEnabled;
	}
}


FORCEINLINE void TLRender::Opengl::SetScissor(u32 x, u32 y, u32 width, u32 height)
{
	static u32 g_SceneScissor[4] = {0,0,0,0};

	if( x != g_SceneScissor[0] ||
		y != g_SceneScissor[1] ||
		width != g_SceneScissor[2] ||
		height != g_SceneScissor[3])
	{
		Platform::SetScissor( x, y, width, height );

		g_SceneScissor[0] = x;
		g_SceneScissor[1] = y;
		g_SceneScissor[2] = width;
		g_SceneScissor[3] = height;
	}
}



//---------------------------------------------------
//	if < 1 then it will enable alpha too
//---------------------------------------------------
void TLRender::Opengl::SetSceneColour(const TColour& Colour,Bool ForceEnableAlpha,Bool AddBlending)
{
	static TColour g_SceneColour( 1.f, 1.f, 1.f, 1.f );

	if ( g_SceneColour != Colour )
	{
		Platform::SetSceneColour( Colour );
		EnableAlpha( ForceEnableAlpha || Colour.IsTransparent(), AddBlending );
		g_SceneColour = Colour;
	}
	else if ( ForceEnableAlpha )
	{
		//	no colour change, but alpha needs to be forced on so enable it
		EnableAlpha( ForceEnableAlpha, AddBlending );
	}
	else if ( !Colour.IsTransparent() )
	{
		//	if a parent forced alpha on, but it's off now, turn it off again
		EnableAlpha( FALSE );
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
FORCEINLINE void TLRender::Opengl::DrawPrimitives(u16 GLPrimType,const TArray<TYPE>& PrimitivesArray)		
{
	const TYPE& FirstPrim = PrimitivesArray.ElementAtConst(0);

	//	gr: Size * TypeSize is DIFFERENT to DataSize() - it's an element count (eg. array size * 3 for a triangle)
	DrawPrimitives( GLPrimType, PrimitivesArray.GetSize() * TYPE::GetSize(), FirstPrim.GetData() );	
}


FORCEINLINE void TLRender::Opengl::DrawPrimitives(u16 GLPrimType,const TArray<TArray<u16> >& PrimitivesArray)
{
	u32 Size = PrimitivesArray.GetSize();
	for ( u32 i=0;	i<Size;	i++ )
	{
		const TArray<u16>& PrimArray = PrimitivesArray.ElementAtConst(i);
		DrawPrimitives( GLPrimType, PrimArray.GetSize(), PrimArray.GetData() );
	}
}


//---------------------------------------------------
//	get the angle that the screenshape is rotated to
//---------------------------------------------------
FORCEINLINE const TLMaths::TAngle& TLRender::GetScreenAngle(TScreenShape ScreenShape)
{
	static TLMaths::TAngle g_Angles[3] = 
	{
		0.f,	//	Portrait
		-90.f,	//	Left
		90.f,	//	Right
	};
	
	return g_Angles[ ScreenShape ];
}


