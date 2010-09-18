/*-----------------------------------------------------

	Core file for Platform specific Render lib - 
	essentially the opengl interface


-------------------------------------------------------*/
#pragma once


#include <TootleCore/TLTypes.h>
#include <TootleCore/TLCore.h>
#include <TootleCore/TPtr.h>
#include <TootleCore/TColour.h>
#include <TootleAsset/TLAsset.h>
#include <TootleAsset/TShader.h>

#import <OpenGLES/ES1/gl.h>


//	forward declaration
class TColour;



namespace TLRender
{
	namespace Platform
	{
		SyncBool		Init();			//	platform/opengl initialisation
		SyncBool		Shutdown();		//	platform/opengl shutdown
		
		class TShaderAssetFactory;		//	asset factory for fixed-function shaders
		extern SyncBool	g_OpenglInitialised;
	}

	namespace Opengl
	{
		namespace Platform
		{
			SyncBool				Init();		//	init opengl
			SyncBool				Shutdown();	//	cleanup opengl

			FORCEINLINE Bool		Debug_CheckForError()		{	return FALSE;	}

			FORCEINLINE u16			GetPrimTypeTriangle()		{	return GL_TRIANGLES;	}
			FORCEINLINE u16			GetPrimTypeTristrip()		{	return GL_TRIANGLE_STRIP;	}
			FORCEINLINE u16			GetPrimTypeTrifan()			{	return GL_TRIANGLE_FAN;	}
			FORCEINLINE u16			GetPrimTypeLinestrip()		{	return GL_LINE_STRIP;	}
			FORCEINLINE u16			GetPrimTypeLine()			{	return GL_LINES;	}
			FORCEINLINE u16			GetPrimTypePoint()			{	return GL_POINTS;	}

			FORCEINLINE void		EnableWireframe(Bool Enable)			{	}	//	gr: line polygon modes are not supported in opengl ES. Wireframe does nothing!
			FORCEINLINE void		EnableAlpha(Bool Enable)				{	if ( Enable )	glEnable( GL_BLEND );		else	glDisable( GL_BLEND );	}
			FORCEINLINE void		EnableAddBlending(Bool Enable)			{	if ( Enable )	glBlendFunc(GL_SRC_ALPHA, GL_ONE);	else	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);	}
			FORCEINLINE void		EnableDepthRead(Bool Enable)			{	if ( Enable )	glEnable( GL_DEPTH_TEST );	else	glDisable( GL_DEPTH_TEST );	}
			FORCEINLINE void		EnableDepthWrite(Bool Enable)			{	glDepthMask( Enable ? GL_TRUE : GL_FALSE );	}
			FORCEINLINE void		EnableScissor(Bool Enable)				{	if ( Enable )	glEnable( GL_SCISSOR_TEST );	else	glDisable( GL_SCISSOR_TEST );	}
			FORCEINLINE void		SetScissor(u32 x, u32 y, u32 width, u32 height)	{ 	glScissor( x, y, width, height ); }
			FORCEINLINE void		SetSceneColour(const TColour& Colour)	{	glColor4f( Colour.GetRedf(), Colour.GetGreenf(), Colour.GetBluef(), Colour.GetAlphaf() );	}
			FORCEINLINE void		SetLineWidth(float Width)				{	glLineWidth( Width );	}
			FORCEINLINE void		SetPointSize(float Size)				{	glPointSize( Size );	}
			FORCEINLINE void		EnablePointSprites(Bool Enable)			{	if ( Enable )	glEnable( GL_POINT_SPRITE_OES );		else	glDisable( GL_POINT_SPRITE_OES );	}
			FORCEINLINE void		EnablePointSizeUVMapping(Bool Enable)	{	GLint GLEnable = (Enable ? GL_TRUE : GL_FALSE);	glTexEnvi(GL_POINT_SPRITE_OES, GL_COORD_REPLACE_OES, GLEnable);	}


		}
	}

}



//	fixed functions opengl shaders
namespace TLAsset
{
	class TShader_TextureMatrix;
}


//------------------------------------------------
//	asset factory for fixed-function shaders
//------------------------------------------------
class TLRender::Platform::TShaderAssetFactory : public TLAsset::TAssetFactory
{
protected:
	virtual TLAsset::TAsset*	CreateObject(TRefRef InstanceRef,TRefRef TypeRef);	
};

//------------------------------------------------
//	Fixed function shader which modifies the texture matrix. This allows us to modify 
//	ALL the uv's on a mesh in one go. Useful for sprites for animated textures.
//	Possible that a software shader which modifies the UV's might be faster (no state change).
//	Certainly a vertex shader would be faster.
//	
//	shader data expects to find a TTransform
//------------------------------------------------
class TLAsset::TShader_TextureMatrix : public TLAsset::TShader
{
public:
	TShader_TextureMatrix(TRefRef AssetRef);
	
	static TRef			GetShaderInstanceRef()		{	return TRef_Static(F,F,T,x,M);	}	//	FixedFunctionTeXtureMatrix
	
	virtual Bool		PreRender(TBinaryTree& ShaderData);
	virtual void		PostRender();
};
