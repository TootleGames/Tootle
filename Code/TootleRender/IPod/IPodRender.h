/*-----------------------------------------------------

	Core file for Platform specific Render lib - 
	essentially the opengl interface


-------------------------------------------------------*/
#pragma once


#include <TootleCore/TLTypes.h>
#include <TootleCore/TLCore.h>
#include <TootleCore/TPtr.h>
#include <TootleCore/TColour.h>

#import <OpenGLES/ES1/gl.h>


//	forward declaration
class TColour;



namespace TLRender
{
	namespace Platform
	{
		SyncBool		Init();			//	platform/opengl initialisation
		SyncBool		Shutdown();		//	platform/opengl shutdown
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
			FORCEINLINE void		SetSceneColour(const TColour& Colour)	{	glColor4f( Colour.GetRedf(), Colour.GetGreenf(), Colour.GetBluef(), Colour.GetAlphaf() );	}
			FORCEINLINE void		SetLineWidth(float Width)				{	glLineWidth( Width );	}
			FORCEINLINE void		SetPointSize(float Size)				{	glPointSize( Size );	}
		}
	}

}

