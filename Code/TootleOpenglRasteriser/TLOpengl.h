/*

	Because of SDK differences (ES, PC, Mac etc) the real opengl functions are in platform specific
	files and can then be inlined
	
*/
#pragma once
#include <TootleCore/TLTypes.h>
#include <TootleCore/TArray.h>
#include <TootleCore/TColour.h>
#include <TootleRender/TRasteriser.h>


namespace TLAsset
{
	class TTexture;
}

namespace TLMaths
{
	class TTransform;
}

//----------------------------------------------------------------------------//
//	opengl functionality, abstracted from the rasteriser.
//	also implements client(engine) side state checks to avoid stalls when using opengl
//----------------------------------------------------------------------------//
namespace Opengl
{
	bool					Init();
	void					Shutdown();

	FORCEINLINE Bool		Debug_CheckForError();
	
	//	conversion from type to opengl type. This has been implemented as some platforms (GLES 2.0) don't support some times (32 bit int)
	template<typename TYPE>
	FORCEINLINE u16			GetDataType()			{	TLDebug_Break("Unsupported opengl data type");	return 0;	}
	
	Bool					BindVertexes(const TArray<float3>* pVertexes);
	Bool					BindColours(const TArray<TColour>* pColours);
	Bool					BindColours(const TArray<TColour24>* pColours);
	Bool					BindColours(const TArray<TColour32>* pColours);
	Bool					BindColours(const TArray<TColour64>* pColours);
	FORCEINLINE Bool		BindColoursNull()										{	return BindColours( (const TArray<TColour>*)NULL );	}	//	wrapper for ambigious use of NULL
	Bool					BindUVs(const TArray<float2>* pUVs);
	Bool					BindTexture(const TLAsset::TTexture* pTexture);			//	bind texture - returns FALSE if no texture is bound (either fail or expected)
	void					Unbind();												//	unbind all verts/colours etc
	void					Unbind(const TLRaster::TVertexElementType::Type& Element);		//	unbind this vertex element
	bool					BindVertexElement(const TLRaster::TVertexElement& Element);		//	bind this vertex element
	
	//	gr: don't expose this when moved into rasteriser as the rasteriser could call this directly
	Bool					DestroyTexture(TRefRef TextureRef);					//	destroy opengl texture by texture reference

	void					DrawPrimitives(u16 GLPrimType,u32 IndexCount,const u16* pIndexData);		//	main renderer, just needs primitive type, and the data
	template<class TYPE>
	FORCEINLINE void		DrawPrimitives(u16 GLPrimType,const TArray<TYPE>& PrimitivesArray);			//	wrapper to pull out the index data for different types
	FORCEINLINE void		DrawPrimitives(u16 GLPrimType,const TArray<THeapArray<u16> >& PrimitivesArray);	//	specialisation for arrays of u16s (ie. strips)
	void					DrawPrimitivePoints(const TArray<float3>* pVertexes);

	void					TextureTransform(const TLMaths::TTransform& Transform);				//	texture matrix transform
	void					SceneReset();														//	reset scene
	void					SceneTransform(const TLMaths::TTransform& Transform,const TLMaths::TMatrix* pMatrix=NULL);	//	transform scene
	void					SceneRotate(const TLMaths::TAngle& Rotation,const float3& Axis);	//	eular rotation on the scene - wrapper for glRotatef

	FORCEINLINE void		ClampPointSpriteSize(float& PointSize)		{	TLMaths::Limit( PointSize, 1.f, 64.f );	}	//	gr: 64 is the limit on the iphone. change this to use glGetFloatv(GL_ALIASED_POINT_SIZE_RANGE, sizes); to get accurate sizes. 
	void					Clear(const TColour& Colour,bool ClearColour,bool ClearDepth,bool ClearStencil);		//	clear current viewport

	
	//	scene settings - only calls platform implementations as required - inlined for speed
	FORCEINLINE void		EnableWireframe(Bool Enable);
	FORCEINLINE void		SetBlendMode(TLRaster::TBlendMode::Type Mode);
	FORCEINLINE void		EnableDepthRead(Bool Enable);
	FORCEINLINE void		EnableDepthWrite(Bool Enable);
	FORCEINLINE void		EnableScissor(Bool Enable);
	FORCEINLINE void		SetScissor(u32 x, u32 y, u32 width, u32 height);
	FORCEINLINE void		SetSceneColour(const TColour& Colour);
	FORCEINLINE void		SetClearColour(const TColour& Colour);
	FORCEINLINE void		SetLineWidth(float Width);
	FORCEINLINE void		SetPointSize(float Size);
	FORCEINLINE void		EnablePointSprites(Bool Enable);
	FORCEINLINE void		EnablePointSizeUVMapping(Bool Enable);
	FORCEINLINE void		EnableAntiAliasing(Bool Enable)			{}	//	todo!


	//	dumb platform specific implementations - just do whatever is specified
	namespace Platform
	{
		bool					Init();						//	init opengl
		void					Shutdown();					//	cleanup opengl
		
		Bool					Debug_CheckForError();		//	check for opengl error - returns TRUE on error
		void					SetFrustumProjection(const TLMaths::TBox2D& ScreenViewBox,float NearZ,float FarZ);
		void					SetFrustumOrthographic(const TLMaths::TBox2D& ScreenViewBox,float NearZ,float FarZ);

		FORCEINLINE u16			GetPrimTypeTriangle();		//	GL_TRIANGLES
		FORCEINLINE u16			GetPrimTypeTristrip();		//	GL_TRIANGLE_STRIP
		FORCEINLINE u16			GetPrimTypeTrifan();		//	GL_TRIANGLE_FAN
		FORCEINLINE u16			GetPrimTypeLinestrip();		//	GL_LINE_STRIP
		FORCEINLINE u16			GetPrimTypeLine();			//	GL_LINES
		FORCEINLINE u16			GetPrimTypePoint();			//	GL_POINTS

		FORCEINLINE void		EnableWireframe(Bool Enable);
		FORCEINLINE void		SetBlendMode(TLRaster::TBlendMode::Type Mode);
		FORCEINLINE void		EnableDepthRead(Bool Enable);
		FORCEINLINE void		EnableDepthWrite(Bool Enable);
		FORCEINLINE void		EnableScissor(Bool Enable);
		FORCEINLINE void		SetScissor(u32 x, u32 y, u32 width, u32 height);
		FORCEINLINE void		SetSceneColour(const TColour& Colour);
		FORCEINLINE void		SetClearColour(const TColour& Colour);
		FORCEINLINE void		SetLineWidth(float Width);
		FORCEINLINE void		SetPointSize(float Size);
		FORCEINLINE void		EnablePointSprites(Bool Enable);
		FORCEINLINE void		EnablePointSizeUVMapping(Bool Enable);
		FORCEINLINE void		EnableAntiAliasing(Bool Enable);
	}
};



//----------------------------------------------------------------------------//
//	include -inline- platform-specific function calls
//----------------------------------------------------------------------------//
#include PLATFORMHEADER(Opengl.h)


namespace Opengl
{
	template<> FORCEINLINE u16		GetDataType<float>()	{	return GL_FLOAT;	}
	template<> FORCEINLINE u16		GetDataType<u8>()		{	return GL_UNSIGNED_BYTE;	}
	template<> FORCEINLINE u16		GetDataType<s8>()		{	return GL_BYTE;	}
	template<> FORCEINLINE u16		GetDataType<u16>()		{	return GL_UNSIGNED_SHORT;	}
	template<> FORCEINLINE u16		GetDataType<s16>()		{	return GL_SHORT;	}
	template<> FORCEINLINE u16		GetDataType<u32>()		{	return GL_UNSIGNED_INT;	}
	template<> FORCEINLINE u16		GetDataType<s32>()		{	return GL_INT;	}
}



FORCEINLINE Bool Opengl::Debug_CheckForError()
{
#if defined(_DEBUG) && !defined(TL_TARGET_IPOD) && !defined(TL_TARGET_IPAD)
	return Platform::Debug_CheckForError();
#else
	return FALSE;
#endif
}

FORCEINLINE void Opengl::EnableWireframe(Bool Enable)
{
	static SyncBool g_SceneEnabled = SyncWait;
	SyncBool NewEnabled = Enable ? SyncTrue : SyncFalse;

	if ( NewEnabled != g_SceneEnabled )
	{
		Platform::EnableWireframe( Enable );
		g_SceneEnabled = NewEnabled;
	}
}


FORCEINLINE void Opengl::SetBlendMode(TLRaster::TBlendMode::Type Mode)
{
	static TLRaster::TBlendMode::Type g_BlendMode = TLRaster::TBlendMode::Opaque;

	if ( Mode != g_BlendMode )
	{
		Platform::SetBlendMode( Mode );
		g_BlendMode = Mode;
	}
}


FORCEINLINE void Opengl::EnableDepthRead(Bool Enable)
{
	static SyncBool g_SceneEnabled = SyncWait;
	SyncBool NewEnabled = Enable ? SyncTrue : SyncFalse;

	if ( NewEnabled != g_SceneEnabled )
	{
		Platform::EnableDepthRead( Enable );
		g_SceneEnabled = NewEnabled;
	}
}


FORCEINLINE void Opengl::EnableDepthWrite(Bool Enable)
{
	static SyncBool g_SceneEnabled = SyncWait;
	SyncBool NewEnabled = Enable ? SyncTrue : SyncFalse;

	if ( NewEnabled != g_SceneEnabled )
	{
		Platform::EnableDepthWrite( Enable );
		g_SceneEnabled = NewEnabled;
	}
}


FORCEINLINE void Opengl::EnableScissor(Bool Enable)
{
	static SyncBool g_SceneEnabled = SyncWait;
	SyncBool NewEnabled = Enable ? SyncTrue : SyncFalse;

	if ( NewEnabled != g_SceneEnabled )
	{
		Platform::EnableScissor( Enable );
		g_SceneEnabled = NewEnabled;
	}
}


FORCEINLINE void Opengl::SetScissor(u32 x, u32 y, u32 width, u32 height)
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


FORCEINLINE void Opengl::EnablePointSprites(Bool Enable)
{
	static SyncBool g_SceneEnabled = SyncWait;
	SyncBool NewEnabled = Enable ? SyncTrue : SyncFalse;

	if ( NewEnabled != g_SceneEnabled )
	{
		Platform::EnablePointSprites( Enable );
		g_SceneEnabled = NewEnabled;
	}
}


FORCEINLINE void Opengl::EnablePointSizeUVMapping(Bool Enable)
{
	static SyncBool g_PointSizeUVMappingEnabled = SyncWait;
	SyncBool NewEnabled = (Enable ? SyncTrue : SyncFalse);

	if ( NewEnabled != g_PointSizeUVMappingEnabled )
	{
		Platform::EnablePointSizeUVMapping( Enable );
		g_PointSizeUVMappingEnabled = NewEnabled;
	}
}


//---------------------------------------------------
//	if < 1 then it will enable alpha too
//---------------------------------------------------
void Opengl::SetSceneColour(const TColour& Colour)
{
	static TColour g_SceneColour( 1.f, 1.f, 1.f, 1.f );

	//	gr: probably need some floating point discrepency here
	if ( g_SceneColour != Colour )
	{
		Platform::SetSceneColour( Colour );
		g_SceneColour = Colour;
	}
}



void Opengl::SetClearColour(const TColour& Colour)
{
	static TColour g_ClearColour( 1.f, 1.f, 1.f, 1.f );
	
	if ( g_ClearColour != Colour )
	{
		Platform::SetClearColour( Colour );
		g_ClearColour = Colour;
	}
}

void Opengl::SetLineWidth(float Width)
{
	static float g_SceneLineWidth = 1.f;
	
	if ( g_SceneLineWidth != Width )
	{
		Platform::SetLineWidth( Width );
		g_SceneLineWidth = Width;
	}
}


void Opengl::SetPointSize(float Size)
{
	static float g_ScenePointSize = 1.f;
	
	if ( g_ScenePointSize != Size )
	{
		Platform::SetPointSize( Size );
		g_ScenePointSize = Size;
	}
}




template<class TYPE>
FORCEINLINE void Opengl::DrawPrimitives(u16 GLPrimType,const TArray<TYPE>& PrimitivesArray)		
{
	const TYPE& FirstPrim = PrimitivesArray.ElementAtConst(0);

	//	gr: Size * TypeSize is DIFFERENT to DataSize() - it's an element count (eg. array size * 3 for a triangle)
	DrawPrimitives( GLPrimType, PrimitivesArray.GetSize() * TYPE::GetSize(), FirstPrim.GetData() );	
}


FORCEINLINE void Opengl::DrawPrimitives(u16 GLPrimType,const TArray<THeapArray<u16> >& PrimitivesArray)
{
	u32 Size = PrimitivesArray.GetSize();
	for ( u32 i=0;	i<Size;	i++ )
	{
		const TArray<u16>& PrimArray = PrimitivesArray.ElementAtConst(i);
		DrawPrimitives( GLPrimType, PrimArray.GetSize(), PrimArray.GetData() );
	}
}

//-------------------------------------------------------
//	clear current viewport
//-------------------------------------------------------
FORCEINLINE void Opengl::Clear(const TColour& Colour,bool ClearColour,bool ClearDepth,bool ClearStencil)
{
	//	update clear colour
	if ( ClearColour )
		SetClearColour( Colour );
	
	//	this returns mask or 0
//#define TLBoolToBitMask(boolean,mask)	( ((boolean)<<31) & mask )
	#define TLBoolToBitMask(boolean,mask)	( boolean ? mask : 0 )

	//	get clear mask
	GLbitfield ClearMask = TLBoolToBitMask(ClearDepth,GL_DEPTH_BUFFER_BIT) | TLBoolToBitMask(ClearStencil,GL_STENCIL_BUFFER_BIT) | TLBoolToBitMask(ClearColour,GL_COLOR_BUFFER_BIT);

	//	clear
	glClear( ClearMask );
	
}	


FORCEINLINE void Opengl::Platform::SetBlendMode(TLRaster::TBlendMode::Type Mode)
{
	switch ( Mode )
	{
		default:
		case TLRaster::TBlendMode::Opaque:
			glDisable( GL_BLEND );
			break;

		case TLRaster::TBlendMode::Alpha:
			glEnable( GL_BLEND );
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			break;

		case TLRaster::TBlendMode::Add:
			glEnable( GL_BLEND );
			glBlendFunc(GL_SRC_ALPHA, GL_ONE);
			break;
	}
}

