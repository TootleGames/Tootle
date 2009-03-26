/*-----------------------------------------------------

	Core file for Platform specific Render lib - 
	essentially the opengl interface


-------------------------------------------------------*/
#pragma once


#include <TootleCore/TLTypes.h>
#include <TootleCore/TLCore.h>
#include <TootleCore/TPtr.h>
#include <TootleCore/TColour.h>

//	include opengl stuff
#pragma comment( lib, "Opengl32.lib" )
#pragma comment( lib, "glu32.lib" )

#include "glsdk/gl.h"
#include "glsdk/glext.h"
#include "glsdk/wglext.h"


//	forward declaration
class TColour;



namespace TLRender
{
	namespace Platform
	{
		SyncBool		Init();			//	platform/opengl initialisation
		SyncBool		Shutdown();		//	platform/opengl shutdown

		extern SyncBool	g_OpenglInitialised;
	}

	namespace Opengl
	{
		namespace Platform
		{
			SyncBool				Init();		//	init opengl
			SyncBool				Shutdown();	//	cleanup opengl

			Bool					Debug_CheckForError();		//	check for opengl error - returns TRUE on error

			//Bool					BindFixedVertexes(const TArray<TLAsset::TFixedVertex>* pVertexes);
			Bool					BindVertexes(const TArray<float3>* pVertexes);
			Bool					BindColours(const TArray<TColour>* pColours);
			Bool					BindUVs(const TArray<float2>* pUVs);
			void					DrawPrimitives(u32 GLPrimType,u32 IndexCount,const u16* pIndexData);	//	main renderer, just needs primitive type, and the data
			
			FORCEINLINE u16			GetPrimTypeTriangle()		{	return GL_TRIANGLES;	}
			FORCEINLINE u16			GetPrimTypeTristrip()		{	return GL_TRIANGLE_STRIP;	}
			FORCEINLINE u16			GetPrimTypeTrifan()			{	return GL_TRIANGLE_FAN;	}
			FORCEINLINE u16			GetPrimTypeLinestrip()		{	return GL_LINE_STRIP;	}
			FORCEINLINE u16			GetPrimTypeLine()			{	return GL_LINES;	}
			FORCEINLINE u16			GetPrimTypePoint()			{	return GL_POINTS;	}

			FORCEINLINE void		EnableWireframe(Bool Enable)			{	glPolygonMode( GL_FRONT_AND_BACK, Enable ? GL_LINE : GL_FILL );	}
			FORCEINLINE void		EnableAlpha(Bool Enable)				{	if ( Enable )	glEnable( GL_BLEND );		else	glDisable( GL_BLEND );	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);	}
			FORCEINLINE void		EnableAddBlending(Bool Enable)				{	if ( Enable )	glBlendFunc(GL_SRC_ALPHA, GL_ONE);	else	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);	}
			FORCEINLINE void		EnableDepthRead(Bool Enable)			{	if ( Enable )	glEnable( GL_DEPTH_TEST );	else	glDisable( GL_DEPTH_TEST );	}
			FORCEINLINE void		EnableDepthWrite(Bool Enable)			{	glDepthMask( Enable ? GL_TRUE : GL_FALSE );	}
			FORCEINLINE void		EnableScissor(Bool Enable)				{	if ( Enable )	glEnable( GL_SCISSOR_TEST );	else	glDisable( GL_SCISSOR_TEST );	}
			FORCEINLINE void		SetSceneColour(const TColour& Colour)	{	glColor4fv( Colour.GetData() );	}
			FORCEINLINE void		SetLineWidth(float Width)				{	glLineWidth( Width );	}
			FORCEINLINE void		SetPointSize(float Size)				{	glPointSize( Size );	}
		}
	}

}


