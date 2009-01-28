/*-----------------------------------------------------

	Core file for Platform specific Render lib - 
	essentially the opengl interface


-------------------------------------------------------*/
#pragma once


#include <TootleCore/TLTypes.h>
#include <TootleCore/TLCore.h>
#include <TootleCore/TPtr.h>
#include <TootleCore/TColour.h>



//	forward declaration
class TColour;



namespace TLRender
{
	namespace Platform
	{
		SyncBool		Init();			//	platform/opengl initialisation
		SyncBool		Shutdown();		//	platform/opengl shutdown
		
		Bool			BeginDraw();	//	
		void			EndDraw();		//	
	}

	namespace Opengl
	{
		namespace Platform
		{
			class TVertexBufferObject;
			
			SyncBool				Init();		//	init opengl
			SyncBool				Shutdown();	//	cleanup opengl

			Bool					Debug_CheckForError()	{	return FALSE;	}	//	gr: very slow on ipod, so not used

			//Bool					BindFixedVertexes(const TArray<TLAsset::TFixedVertex>* pVertexes,TRefRef MeshRef);
			Bool					BindVertexes(const TArray<float3>* pVertexes,TRefRef MeshRef);
			Bool					BindColours(const TArray<TColour>* pColours,TRefRef MeshRef);
			void					DrawPrimitives(u32 GLPrimType,u32 IndexCount,const u16* pIndexData);	//	main renderer, just needs primitive type, and the data
			
			FORCEINLINE u16			GetPrimTypeTriangle()		{	return GL_TRIANGLES;	}
			FORCEINLINE u16			GetPrimTypeTristrip()		{	return GL_TRIANGLE_STRIP;	}
			FORCEINLINE u16			GetPrimTypeTrifan()			{	return GL_TRIANGLE_FAN;	}
			FORCEINLINE u16			GetPrimTypeLineStrip()		{	return GL_LINE_STRIP;	}
			FORCEINLINE u16			GetPrimTypePoint()			{	return GL_POINTS;	}

			FORCEINLINE void		EnableWireframe(Bool Enable)			{	glPolygonMode( GL_FRONT_AND_BACK, Enable ? GL_LINE : GL_FILL );	}
			FORCEINLINE void		EnableAlpha(Bool Enable)				{	if ( Enable )	glEnable( GL_BLEND );		else	glDisable( GL_BLEND );	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);	}
			FORCEINLINE void		EnableDepthRead(Bool Enable)			{	if ( Enable )	glEnable( GL_DEPTH_TEST );	else	glDisable( GL_DEPTH_TEST );	}
			FORCEINLINE void		EnableDepthWrite(Bool Enable)			{	glDepthMask( Enable ? GL_TRUE : GL_FALSE );	}
			FORCEINLINE void		SetSceneColour(const TColour& Colour)	{	glColor4f( Colour.x, Colour.y, Colour.z, Colour.w );	}
			FORCEINLINE void		SetLineWidth(float Width)				{	glLineWidth( Width );	}
			FORCEINLINE void		SetPointSize(float Size)				{	glPointSize( Size );	}
		}
	}

}




class TLRender::Opengl::Platform::TVertexBufferObject
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

