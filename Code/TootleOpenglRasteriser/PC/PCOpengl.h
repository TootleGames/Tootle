/*

	PC specific opengl implementations

*/
#pragma once

//	include opengl stuff
#include <windows.h>		//	ergh! don't really want to do this!
#include "glsdk/gl.h"
#include "glsdk/glext.h"
#include "glsdk/wglext.h"

#pragma comment( lib, "Opengl32.lib" )
#pragma comment( lib, "glu32.lib" )

#include <TootleCore/TTransform.h>
#include <TootleCore/TColour.h>
#include <TootleMaths/TBox.h>


namespace Opengl
{
	namespace Platform
	{
		bool					Init();		//	init opengl
		void					Shutdown();	//	cleanup opengl

		void					TextureTransform(const TLMaths::TTransform& Transform);	//	transform the texture matrix

		Bool					Debug_CheckForError();		//	check for opengl error - returns TRUE on error
		void					SetFrustumProjection(const TLMaths::TBox2D& ScreenViewBox,float NearZ,float FarZ);
		void					SetFrustumOrthographic(const TLMaths::TBox2D& ScreenViewBox,float NearZ,float FarZ);

		FORCEINLINE u16			GetPrimTypeTriangle()		{	return GL_TRIANGLES;	}
		FORCEINLINE u16			GetPrimTypeTristrip()		{	return GL_TRIANGLE_STRIP;	}
		FORCEINLINE u16			GetPrimTypeTrifan()			{	return GL_TRIANGLE_FAN;	}
		FORCEINLINE u16			GetPrimTypeLinestrip()		{	return GL_LINE_STRIP;	}
		FORCEINLINE u16			GetPrimTypeLine()			{	return GL_LINES;	}
		FORCEINLINE u16			GetPrimTypePoint()			{	return GL_POINTS;	}

		FORCEINLINE void		EnableWireframe(Bool Enable)			{	glPolygonMode( GL_FRONT_AND_BACK, Enable ? GL_LINE : GL_FILL );	}
		FORCEINLINE void		EnableDepthRead(Bool Enable)			{	Enable ? glEnable( GL_DEPTH_TEST ) : glDisable( GL_DEPTH_TEST );	}
		FORCEINLINE void		EnableDepthWrite(Bool Enable)			{	glDepthMask( Enable ? GL_TRUE : GL_FALSE );	}
		FORCEINLINE void		EnableScissor(Bool Enable)				{	Enable ? glEnable( GL_SCISSOR_TEST ) : glDisable( GL_SCISSOR_TEST );	}
		FORCEINLINE void		SetScissor(u32 x, u32 y, u32 width, u32 height)	{ 	glScissor( x, y, width, height ); }
		FORCEINLINE void		SetSceneColour(const TColour& Colour)	{	glColor4fv( Colour.GetData() );	}
		FORCEINLINE void		SetClearColour(const TColour& Colour)	{	glClearColor( Colour.GetRedf(), Colour.GetGreenf(), Colour.GetBluef(), Colour.GetAlphaf() );	}
		FORCEINLINE void		SetLineWidth(float Width)				{	glLineWidth( Width );	}
		FORCEINLINE void		SetPointSize(float Size)				{	glPointSize( Size );	}
		FORCEINLINE void		EnablePointSprites(Bool Enable)			{	Enable ? glEnable( GL_POINT_SPRITE_ARB ) : glDisable( GL_POINT_SPRITE_ARB );	}
		FORCEINLINE void		EnablePointSizeUVMapping(Bool Enable)	{	GLint GLEnable = (Enable ? GL_TRUE : GL_FALSE);	glTexEnvi(GL_POINT_SPRITE_ARB, GL_COORD_REPLACE_ARB, GLEnable);	}
		FORCEINLINE void		EnableAntiAliasing(Bool Enable)			{	Enable ? glEnable( GL_MULTISAMPLE_ARB ) : glDisable( GL_MULTISAMPLE_ARB );	}
	}
}


FORCEINLINE void Opengl::Platform::SetFrustumProjection(const TLMaths::TBox2D& ScreenViewBox,float NearZ,float FarZ)
{
	//	set projection matrix - 
	//	gr: note, Bottom and Top are the WRONG way around to invert opengl's upside coordinate system and makes things simpiler in our own code
	glFrustum( ScreenViewBox.GetLeft(), ScreenViewBox.GetRight(), ScreenViewBox.GetTop(), ScreenViewBox.GetBottom(), NearZ, FarZ );
}

FORCEINLINE void Opengl::Platform::SetFrustumOrthographic(const TLMaths::TBox2D& ScreenViewBox,float NearZ,float FarZ)
{
	//	set the world coordinates
	glOrtho( ScreenViewBox.GetLeft(), ScreenViewBox.GetRight(), ScreenViewBox.GetBottom(), ScreenViewBox.GetTop(), NearZ, FarZ );
}
