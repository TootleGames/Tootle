/*-----------------------------------------------------

	Interface to the OpenGL extensions


-------------------------------------------------------*/
#pragma once

#include <TootleCore/TLTypes.h>
#include "PCRender.h"


namespace Win32
{
	class GOpenglWindow;
}


namespace TLRender
{
	namespace Platform
	{
		namespace OpenglExtensions
		{
			SyncBool		Init();		//	initialise extensions
			SyncBool		Shutdown();	//	shutdown extensions

			//	opengl extensions		
			enum
			{
				GHardware_MultiTexturing=0,
				GHardware_VertexBufferObjects,
				GHardware_DrawRangeElements,
				GHardware_SwapInterval,
				GHardware_NVVertexProgram,
				GHardware_ARBVertexProgram,
				GHardware_ARBFragmentProgram,
				GHardware_ARBMultiSample,

				GHardware_Max,
			};

			SyncBool		IsHardwareSupported(u32 HardwareIndex);		//	is this extension supported
			Bool			IsHardwareEnabled(u32 HardwareIndex);		//	is this extension enabled? 
			void			InitHardwareSupport(u32 HardwareIndex);		//	externally init hardware support

			u32				GetArbMultisamplePixelFormat();

			void*			GetExtensionFunctionAddress(u32 HardwareIndex,u32 FunctionIndex);

			#define DECLARE_EXT(TYPE, FUNC, HARDWARE, INDEX )	inline TYPE FUNC() {	return (TYPE)GetExtensionFunctionAddress( HARDWARE, INDEX );	}

						
			//	multitexture
			typedef void (APIENTRY * PFNGLACTIVETEXTUREARBPROC) (GLenum texture);
			typedef void (APIENTRY * PFNGLCLIENTACTIVETEXTUREARBPROC) (GLenum texture);
			typedef void (APIENTRY * PFNGLMULTITEXCOORD1DARBPROC) (GLenum target, GLdouble s);
			typedef void (APIENTRY * PFNGLMULTITEXCOORD1DVARBPROC) (GLenum target, const GLdouble *v);
			typedef void (APIENTRY * PFNGLMULTITEXCOORD1FARBPROC) (GLenum target, GLfloat s);
			typedef void (APIENTRY * PFNGLMULTITEXCOORD1FVARBPROC) (GLenum target, const GLfloat *v);
			typedef void (APIENTRY * PFNGLMULTITEXCOORD1IARBPROC) (GLenum target, GLint s);
			typedef void (APIENTRY * PFNGLMULTITEXCOORD1IVARBPROC) (GLenum target, const GLint *v);
			typedef void (APIENTRY * PFNGLMULTITEXCOORD1SARBPROC) (GLenum target, GLshort s);
			typedef void (APIENTRY * PFNGLMULTITEXCOORD1SVARBPROC) (GLenum target, const GLshort *v);
			typedef void (APIENTRY * PFNGLMULTITEXCOORD2DARBPROC) (GLenum target, GLdouble s, GLdouble t);
			typedef void (APIENTRY * PFNGLMULTITEXCOORD2DVARBPROC) (GLenum target, const GLdouble *v);
			typedef void (APIENTRY * PFNGLMULTITEXCOORD2FARBPROC) (GLenum target, GLfloat s, GLfloat t);
			typedef void (APIENTRY * PFNGLMULTITEXCOORD2FVARBPROC) (GLenum target, const GLfloat *v);
			typedef void (APIENTRY * PFNGLMULTITEXCOORD2IARBPROC) (GLenum target, GLint s, GLint t);
			typedef void (APIENTRY * PFNGLMULTITEXCOORD2IVARBPROC) (GLenum target, const GLint *v);
			typedef void (APIENTRY * PFNGLMULTITEXCOORD2SARBPROC) (GLenum target, GLshort s, GLshort t);
			typedef void (APIENTRY * PFNGLMULTITEXCOORD2SVARBPROC) (GLenum target, const GLshort *v);
			typedef void (APIENTRY * PFNGLMULTITEXCOORD3DARBPROC) (GLenum target, GLdouble s, GLdouble t, GLdouble r);
			typedef void (APIENTRY * PFNGLMULTITEXCOORD3DVARBPROC) (GLenum target, const GLdouble *v);
			typedef void (APIENTRY * PFNGLMULTITEXCOORD3FARBPROC) (GLenum target, GLfloat s, GLfloat t, GLfloat r);
			typedef void (APIENTRY * PFNGLMULTITEXCOORD3FVARBPROC) (GLenum target, const GLfloat *v);
			typedef void (APIENTRY * PFNGLMULTITEXCOORD3IARBPROC) (GLenum target, GLint s, GLint t, GLint r);
			typedef void (APIENTRY * PFNGLMULTITEXCOORD3IVARBPROC) (GLenum target, const GLint *v);
			typedef void (APIENTRY * PFNGLMULTITEXCOORD3SARBPROC) (GLenum target, GLshort s, GLshort t, GLshort r);
			typedef void (APIENTRY * PFNGLMULTITEXCOORD3SVARBPROC) (GLenum target, const GLshort *v);
			typedef void (APIENTRY * PFNGLMULTITEXCOORD4DARBPROC) (GLenum target, GLdouble s, GLdouble t, GLdouble r, GLdouble q);
			typedef void (APIENTRY * PFNGLMULTITEXCOORD4DVARBPROC) (GLenum target, const GLdouble *v);
			typedef void (APIENTRY * PFNGLMULTITEXCOORD4FARBPROC) (GLenum target, GLfloat s, GLfloat t, GLfloat r, GLfloat q);
			typedef void (APIENTRY * PFNGLMULTITEXCOORD4FVARBPROC) (GLenum target, const GLfloat *v);
			typedef void (APIENTRY * PFNGLMULTITEXCOORD4IARBPROC) (GLenum target, GLint s, GLint t, GLint r, GLint q);
			typedef void (APIENTRY * PFNGLMULTITEXCOORD4IVARBPROC) (GLenum target, const GLint *v);
			typedef void (APIENTRY * PFNGLMULTITEXCOORD4SARBPROC) (GLenum target, GLshort s, GLshort t, GLshort r, GLshort q);
			typedef void (APIENTRY * PFNGLMULTITEXCOORD4SVARBPROC) (GLenum target, const GLshort *v);


			//	array buffer arb
			#define GL_ARRAY_BUFFER_ARB 0x8892
			#define  GL_STREAM_DRAW_ARB   0x88E0 
			#define  GL_STREAM_READ_ARB   0x88E1 
			#define  GL_STREAM_COPY_ARB   0x88E2 
			#define  GL_STATIC_DRAW_ARB   0x88E4 
			#define  GL_STATIC_READ_ARB   0x88E5 
			#define  GL_STATIC_COPY_ARB   0x88E6 
			#define  GL_DYNAMIC_DRAW_ARB   0x88E8 
			#define  GL_DYNAMIC_READ_ARB   0x88E9 
			#define  GL_DYNAMIC_COPY_ARB   0x88EA 
			#define  GL_READ_ONLY_ARB   0x88B8 
			#define  GL_WRITE_ONLY_ARB   0x88B9 
			#define  GL_READ_WRITE_ARB   0x88BA 
			typedef void (APIENTRY * PFNGLBINDBUFFERARBPROC) (GLenum target, GLuint buffer);
			typedef void (APIENTRY * PFNGLDELETEBUFFERSARBPROC) (GLsizei n, const GLuint *buffers);
			typedef void (APIENTRY * PFNGLGENBUFFERSARBPROC) (GLsizei n, GLuint *buffers);
			typedef void (APIENTRY * PFNGLBUFFERDATAARBPROC) (GLenum target, int size, const GLvoid *data, GLenum usage);
			typedef void (APIENTRY * PFNGLDRAWRANGEELEMENTSPROC) (GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const GLvoid *indices);

			//	nv vertex program function
			typedef GLboolean (APIENTRY * PFNGLAREPROGRAMSRESIDENTNVPROC) (GLsizei n, const GLuint *programs, GLboolean *residences);
			typedef void (APIENTRY * PFNGLBINDPROGRAMNVPROC) (GLenum target, GLuint id);
			typedef void (APIENTRY * PFNGLDELETEPROGRAMSNVPROC) (GLsizei n, const GLuint *programs);
			typedef void (APIENTRY * PFNGLEXECUTEPROGRAMNVPROC) (GLenum target, GLuint id, const GLfloat *params);
			typedef void (APIENTRY * PFNGLGENPROGRAMSNVPROC) (GLsizei n, GLuint *programs);
			typedef void (APIENTRY * PFNGLGETPROGRAMPARAMETERDVNVPROC) (GLenum target, GLuint index, GLenum pname, GLdouble *params);
			typedef void (APIENTRY * PFNGLGETPROGRAMPARAMETERFVNVPROC) (GLenum target, GLuint index, GLenum pname, GLfloat *params);
			typedef void (APIENTRY * PFNGLGETPROGRAMIVNVPROC) (GLuint id, GLenum pname, GLint *params);
			typedef void (APIENTRY * PFNGLGETPROGRAMSTRINGNVPROC) (GLuint id, GLenum pname, GLubyte *program);
			typedef void (APIENTRY * PFNGLGETTRACKMATRIXIVNVPROC) (GLenum target, GLuint address, GLenum pname, GLint *params);
			typedef void (APIENTRY * PFNGLGETVERTEXATTRIBDVNVPROC) (GLuint index, GLenum pname, GLdouble *params);
			typedef void (APIENTRY * PFNGLGETVERTEXATTRIBFVNVPROC) (GLuint index, GLenum pname, GLfloat *params);
			typedef void (APIENTRY * PFNGLGETVERTEXATTRIBIVNVPROC) (GLuint index, GLenum pname, GLint *params);
			typedef void (APIENTRY * PFNGLGETVERTEXATTRIBPOINTERVNVPROC) (GLuint index, GLenum pname, GLvoid* *pointer);
			typedef GLboolean (APIENTRY * PFNGLISPROGRAMNVPROC) (GLuint id);
			typedef void (APIENTRY * PFNGLLOADPROGRAMNVPROC) (GLenum target, GLuint id, GLsizei len, const GLubyte *program);
			typedef void (APIENTRY * PFNGLPROGRAMPARAMETER4DNVPROC) (GLenum target, GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w);
			typedef void (APIENTRY * PFNGLPROGRAMPARAMETER4DVNVPROC) (GLenum target, GLuint index, const GLdouble *v);
			typedef void (APIENTRY * PFNGLPROGRAMPARAMETER4FNVPROC) (GLenum target, GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
			typedef void (APIENTRY * PFNGLPROGRAMPARAMETER4FVNVPROC) (GLenum target, GLuint index, const GLfloat *v);
			//typedef void (APIENTRY * PFNGLPROGRAMPARAMETERS4DVNVPROC) (GLenum target, GLuint index, GLsizei count, const GLdouble *v);
			//typedef void (APIENTRY * PFNGLPROGRAMPARAMETERS4FVNVPROC) (GLenum target, GLuint index, GLsizei count, const GLfloat *v);
			typedef void (APIENTRY * PFNGLREQUESTRESIDENTPROGRAMSNVPROC) (GLsizei n, const GLuint *programs);
			typedef void (APIENTRY * PFNGLTRACKMATRIXNVPROC) (GLenum target, GLuint address, GLenum matrix, GLenum transform);
			typedef void (APIENTRY * PFNGLVERTEXATTRIBPOINTERNVPROC) (GLuint index, GLint fsize, GLenum type, GLsizei stride, const GLvoid *pointer);
			typedef void (APIENTRY * PFNGLVERTEXATTRIB1DNVPROC) (GLuint index, GLdouble x);
			typedef void (APIENTRY * PFNGLVERTEXATTRIB1DVNVPROC) (GLuint index, const GLdouble *v);
			typedef void (APIENTRY * PFNGLVERTEXATTRIB1FNVPROC) (GLuint index, GLfloat x);
			typedef void (APIENTRY * PFNGLVERTEXATTRIB1FVNVPROC) (GLuint index, const GLfloat *v);
			typedef void (APIENTRY * PFNGLVERTEXATTRIB1SNVPROC) (GLuint index, GLshort x);
			typedef void (APIENTRY * PFNGLVERTEXATTRIB1SVNVPROC) (GLuint index, const GLshort *v);
			typedef void (APIENTRY * PFNGLVERTEXATTRIB2DNVPROC) (GLuint index, GLdouble x, GLdouble y);
			typedef void (APIENTRY * PFNGLVERTEXATTRIB2DVNVPROC) (GLuint index, const GLdouble *v);
			typedef void (APIENTRY * PFNGLVERTEXATTRIB2FNVPROC) (GLuint index, GLfloat x, GLfloat y);
			typedef void (APIENTRY * PFNGLVERTEXATTRIB2FVNVPROC) (GLuint index, const GLfloat *v);
			typedef void (APIENTRY * PFNGLVERTEXATTRIB2SNVPROC) (GLuint index, GLshort x, GLshort y);
			typedef void (APIENTRY * PFNGLVERTEXATTRIB2SVNVPROC) (GLuint index, const GLshort *v);
			typedef void (APIENTRY * PFNGLVERTEXATTRIB3DNVPROC) (GLuint index, GLdouble x, GLdouble y, GLdouble z);
			typedef void (APIENTRY * PFNGLVERTEXATTRIB3DVNVPROC) (GLuint index, const GLdouble *v);
			typedef void (APIENTRY * PFNGLVERTEXATTRIB3FNVPROC) (GLuint index, GLfloat x, GLfloat y, GLfloat z);
			typedef void (APIENTRY * PFNGLVERTEXATTRIB3FVNVPROC) (GLuint index, const GLfloat *v);
			typedef void (APIENTRY * PFNGLVERTEXATTRIB3SNVPROC) (GLuint index, GLshort x, GLshort y, GLshort z);
			typedef void (APIENTRY * PFNGLVERTEXATTRIB3SVNVPROC) (GLuint index, const GLshort *v);
			typedef void (APIENTRY * PFNGLVERTEXATTRIB4DNVPROC) (GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w);
			typedef void (APIENTRY * PFNGLVERTEXATTRIB4DVNVPROC) (GLuint index, const GLdouble *v);
			typedef void (APIENTRY * PFNGLVERTEXATTRIB4FNVPROC) (GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
			typedef void (APIENTRY * PFNGLVERTEXATTRIB4FVNVPROC) (GLuint index, const GLfloat *v);
			typedef void (APIENTRY * PFNGLVERTEXATTRIB4SNVPROC) (GLuint index, GLshort x, GLshort y, GLshort z, GLshort w);
			typedef void (APIENTRY * PFNGLVERTEXATTRIB4SVNVPROC) (GLuint index, const GLshort *v);
			typedef void (APIENTRY * PFNGLVERTEXATTRIB4UBVNVPROC) (GLuint index, const GLubyte *v);
			typedef void (APIENTRY * PFNGLVERTEXATTRIBS1DVNVPROC) (GLuint index, GLsizei count, const GLdouble *v);
			typedef void (APIENTRY * PFNGLVERTEXATTRIBS1FVNVPROC) (GLuint index, GLsizei count, const GLfloat *v);
			typedef void (APIENTRY * PFNGLVERTEXATTRIBS1SVNVPROC) (GLuint index, GLsizei count, const GLshort *v);
			typedef void (APIENTRY * PFNGLVERTEXATTRIBS2DVNVPROC) (GLuint index, GLsizei count, const GLdouble *v);
			typedef void (APIENTRY * PFNGLVERTEXATTRIBS2FVNVPROC) (GLuint index, GLsizei count, const GLfloat *v);
			typedef void (APIENTRY * PFNGLVERTEXATTRIBS2SVNVPROC) (GLuint index, GLsizei count, const GLshort *v);
			typedef void (APIENTRY * PFNGLVERTEXATTRIBS3DVNVPROC) (GLuint index, GLsizei count, const GLdouble *v);
			typedef void (APIENTRY * PFNGLVERTEXATTRIBS3FVNVPROC) (GLuint index, GLsizei count, const GLfloat *v);
			typedef void (APIENTRY * PFNGLVERTEXATTRIBS3SVNVPROC) (GLuint index, GLsizei count, const GLshort *v);
			typedef void (APIENTRY * PFNGLVERTEXATTRIBS4DVNVPROC) (GLuint index, GLsizei count, const GLdouble *v);
			typedef void (APIENTRY * PFNGLVERTEXATTRIBS4FVNVPROC) (GLuint index, GLsizei count, const GLfloat *v);
			typedef void (APIENTRY * PFNGLVERTEXATTRIBS4SVNVPROC) (GLuint index, GLsizei count, const GLshort *v);
			typedef void (APIENTRY * PFNGLVERTEXATTRIBS4UBVNVPROC) (GLuint index, GLsizei count, const GLubyte *v);

			// arb vertex program
			typedef void (APIENTRY *PFNGLVERTEXATTRIB1SARB)(GLuint index, GLshort x);
			typedef void (APIENTRY *PFNGLVERTEXATTRIB1FARB)(GLuint index, GLfloat x);
			typedef void (APIENTRY *PFNGLVERTEXATTRIB1DARB)(GLuint index, GLdouble x);
			typedef void (APIENTRY *PFNGLVERTEXATTRIB2SARB)(GLuint index, GLshort x, GLshort y);
			typedef void (APIENTRY *PFNGLVERTEXATTRIB2FARB)(GLuint index, GLfloat x, GLfloat y);
			typedef void (APIENTRY *PFNGLVERTEXATTRIB2DARB)(GLuint index, GLdouble x, GLdouble y);
			typedef void (APIENTRY *PFNGLVERTEXATTRIB3SARB)(GLuint index, GLshort x, GLshort y, GLshort z);
			typedef void (APIENTRY *PFNGLVERTEXATTRIB3FARB)(GLuint index, GLfloat x, GLfloat y, GLfloat z);
			typedef void (APIENTRY *PFNGLVERTEXATTRIB3DARB)(GLuint index, GLdouble x, GLdouble y, GLdouble z);
			typedef void (APIENTRY *PFNGLVERTEXATTRIB4SARB)(GLuint index, GLshort x, GLshort y, GLshort z, GLshort w);
			typedef void (APIENTRY *PFNGLVERTEXATTRIB4FARB)(GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
			typedef void (APIENTRY *PFNGLVERTEXATTRIB4DARB)(GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w);
			typedef void (APIENTRY *PFNGLVERTEXATTRIB4NUBARB)(GLuint index, GLubyte x, GLubyte y, GLubyte z, GLubyte w);
			typedef void (APIENTRY *PFNGLVERTEXATTRIB1SVARB)(GLuint index, const GLshort *v);
			typedef void (APIENTRY *PFNGLVERTEXATTRIB1FVARB)(GLuint index, const GLfloat *v);
			typedef void (APIENTRY *PFNGLVERTEXATTRIB1DVARB)(GLuint index, const GLdouble *v);
			typedef void (APIENTRY *PFNGLVERTEXATTRIB2SVARB)(GLuint index, const GLshort *v);
			typedef void (APIENTRY *PFNGLVERTEXATTRIB2FVARB)(GLuint index, const GLfloat *v);
			typedef void (APIENTRY *PFNGLVERTEXATTRIB2DVARB)(GLuint index, const GLdouble *v);
			typedef void (APIENTRY *PFNGLVERTEXATTRIB3SVARB)(GLuint index, const GLshort *v);
			typedef void (APIENTRY *PFNGLVERTEXATTRIB3FVARB)(GLuint index, const GLfloat *v);
			typedef void (APIENTRY *PFNGLVERTEXATTRIB3DVARB)(GLuint index, const GLdouble *v);
			typedef void (APIENTRY *PFNGLVERTEXATTRIB4BVARB)(GLuint index, const GLbyte *v);
			typedef void (APIENTRY *PFNGLVERTEXATTRIB4SVARB)(GLuint index, const GLshort *v);
			typedef void (APIENTRY *PFNGLVERTEXATTRIB4IVARB)(GLuint index, const GLint *v);
			typedef void (APIENTRY *PFNGLVERTEXATTRIB4UBVARB)(GLuint index, const GLubyte *v);
			typedef void (APIENTRY *PFNGLVERTEXATTRIB4USVARB)(GLuint index, const GLushort *v);
			typedef void (APIENTRY *PFNGLVERTEXATTRIB4UIVARB)(GLuint index, const GLuint *v);
			typedef void (APIENTRY *PFNGLVERTEXATTRIB4FVARB)(GLuint index, const GLfloat *v);
			typedef void (APIENTRY *PFNGLVERTEXATTRIB4DVARB)(GLuint index, const GLdouble *v);
			typedef void (APIENTRY *PFNGLVERTEXATTRIB4NBVARB)(GLuint index, const GLbyte *v);
			typedef void (APIENTRY *PFNGLVERTEXATTRIB4NSVARB)(GLuint index, const GLshort *v);
			typedef void (APIENTRY *PFNGLVERTEXATTRIB4NIVARB)(GLuint index, const GLint *v);
			typedef void (APIENTRY *PFNGLVERTEXATTRIB4NUBVARB)(GLuint index, const GLubyte *v);
			typedef void (APIENTRY *PFNGLVERTEXATTRIB4NUSVARB)(GLuint index, const GLushort *v);
			typedef void (APIENTRY *PFNGLVERTEXATTRIB4NUIVARB)(GLuint index, const GLuint *v);
			typedef void (APIENTRY *PFNGLVERTEXATTRIBPOINTERARB)(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride,  const void *pointer);
			typedef void (APIENTRY *PFNGLENABLEVERTEXATTRIBARRAYARB)(GLuint index);
			typedef void (APIENTRY *PFNGLDISABLEVERTEXATTRIBARRAYARB)(GLuint index);
			typedef void (APIENTRY *PFNGLPROGRAMSTRINGARB)(GLenum target, GLenum format, GLsizei len, const void *string); 
			typedef void (APIENTRY *PFNGLBINDPROGRAMARB)(GLenum target, GLuint program);
			typedef void (APIENTRY *PFNGLDELETEPROGRAMSARB)(GLsizei n, const GLuint *programs);
			typedef void (APIENTRY *PFNGLGENPROGRAMSARB)(GLsizei n, GLuint *programs);
			typedef void (APIENTRY *PFNGLPROGRAMENVPARAMETER4DARB)(GLenum target, GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w);
			typedef void (APIENTRY *PFNGLPROGRAMENVPARAMETER4DVARB)(GLenum target, GLuint index, const GLdouble *params);
			typedef void (APIENTRY *PFNGLPROGRAMENVPARAMETER4FARB)(GLenum target, GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
			typedef void (APIENTRY *PFNGLPROGRAMENVPARAMETER4FVARB)(GLenum target, GLuint index, const GLfloat *params);
			typedef void (APIENTRY *PFNGLPROGRAMLOCALPARAMETER4DARB)(GLenum target, GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w);
			typedef void (APIENTRY *PFNGLPROGRAMLOCALPARAMETER4DVARB)(GLenum target, GLuint index, const GLdouble *params);
			typedef void (APIENTRY *PFNGLPROGRAMLOCALPARAMETER4FARB)(GLenum target, GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
			typedef void (APIENTRY *PFNGLPROGRAMLOCALPARAMETER4FVARB)(GLenum target, GLuint index, const GLfloat *params);
			typedef void (APIENTRY *PFNGLGETPROGRAMENVPARAMETERDVARB)(GLenum target, GLuint index, GLdouble *params);
			typedef void (APIENTRY *PFNGLGETPROGRAMENVPARAMETERFVARB)(GLenum target, GLuint index, GLfloat *params);
			typedef void (APIENTRY *PFNGLGETPROGRAMLOCALPARAMETERDVARB)(GLenum target, GLuint index, GLdouble *params);
			typedef void (APIENTRY *PFNGLGETPROGRAMLOCALPARAMETERFVARB)(GLenum target, GLuint index, GLfloat *params);
			typedef void (APIENTRY *PFNGLGETPROGRAMIVARB)(GLenum target, GLenum pname, GLint *params);
			typedef void (APIENTRY *PFNGLGETPROGRAMSTRINGARB)(GLenum target, GLenum pname, void *string);
			typedef void (APIENTRY *PFNGLGETVERTEXATTRIBDVARB)(GLuint index, GLenum pname, GLdouble *params);
			typedef void (APIENTRY *PFNGLGETVERTEXATTRIBFVARB)(GLuint index, GLenum pname, GLfloat *params);
			typedef void (APIENTRY *PFNGLGETVERTEXATTRIBIVARB)(GLuint index, GLenum pname, GLint *params);
			typedef void (APIENTRY *PFNGLGETVERTEXATTRIBPOINTERVARB)(GLuint index, GLenum pname, void **pointer);
			typedef GLboolean (APIENTRY *PFNGLISPROGRAMARB)(GLuint program);



			//	multitexturing
			DECLARE_EXT( PFNGLACTIVETEXTUREARBPROC,				glActiveTextureARB,				GHardware_MultiTexturing,	0	);
			DECLARE_EXT( PFNGLMULTITEXCOORD2FARBPROC,			glMultiTexCoord2fARB,			GHardware_MultiTexturing,	1	);
			DECLARE_EXT( PFNGLCLIENTACTIVETEXTUREARBPROC,		glClientActiveTextureARB,		GHardware_MultiTexturing,	2	);

			//	vertex buffer object
			DECLARE_EXT( PFNGLGENBUFFERSARBPROC,				glGenBuffersARB,				GHardware_VertexBufferObjects,	0	);
			DECLARE_EXT( PFNGLBINDBUFFERARBPROC,				glBindBufferARB,				GHardware_VertexBufferObjects,	1	);
			DECLARE_EXT( PFNGLBUFFERDATAARBPROC,				glBufferDataARB,				GHardware_VertexBufferObjects,	2	);
			DECLARE_EXT( PFNGLDELETEBUFFERSARBPROC,				glDeleteBuffersARB,				GHardware_VertexBufferObjects,	3	);

			//	draw elements
			DECLARE_EXT( PFNGLDRAWRANGEELEMENTSPROC,			glDrawRangeElementsARB,			GHardware_DrawRangeElements,	0	);
			
			//	swap interval
			DECLARE_EXT( PFNWGLSWAPINTERVALEXTPROC,				glSwapIntervalEXT,				GHardware_SwapInterval,	0	);

			//	NV vertex program
			DECLARE_EXT( PFNGLAREPROGRAMSRESIDENTNVPROC,		glAreProgramsResidentNV,		GHardware_NVVertexProgram,	0	);
			DECLARE_EXT( PFNGLBINDPROGRAMNVPROC,				glBindProgramNV,				GHardware_NVVertexProgram,	1	);
			DECLARE_EXT( PFNGLDELETEPROGRAMSNVPROC,				glDeleteProgramsNV,				GHardware_NVVertexProgram,	2	);
			DECLARE_EXT( PFNGLEXECUTEPROGRAMNVPROC,				glExecuteProgramNV,				GHardware_NVVertexProgram,	3	);
			DECLARE_EXT( PFNGLGENPROGRAMSNVPROC,				glGenProgramsNV,				GHardware_NVVertexProgram,	4	);
			DECLARE_EXT( PFNGLGETPROGRAMPARAMETERDVNVPROC,		glGetProgramParameterdvNV,		GHardware_NVVertexProgram,	5	);
			DECLARE_EXT( PFNGLGETPROGRAMPARAMETERFVNVPROC,		glGetProgramParameterfvNV,		GHardware_NVVertexProgram,	6	);
			DECLARE_EXT( PFNGLGETPROGRAMIVNVPROC,				glGetProgramivNV,				GHardware_NVVertexProgram,	7	);
			DECLARE_EXT( PFNGLGETPROGRAMSTRINGNVPROC,			glGetProgramStringNV,			GHardware_NVVertexProgram,	8	);
			DECLARE_EXT( PFNGLGETTRACKMATRIXIVNVPROC,			glGetTrackMatrixivNV,			GHardware_NVVertexProgram,	9	);
			DECLARE_EXT( PFNGLGETVERTEXATTRIBDVNVPROC,			glGetVertexAttribdvNV,			GHardware_NVVertexProgram,	10	);
			DECLARE_EXT( PFNGLGETVERTEXATTRIBFVNVPROC,			glGetVertexAttribfvNV,			GHardware_NVVertexProgram,	11	);
			DECLARE_EXT( PFNGLGETVERTEXATTRIBIVNVPROC,			glGetVertexAttribivNV,			GHardware_NVVertexProgram,	12	);
			DECLARE_EXT( PFNGLGETVERTEXATTRIBPOINTERVNVPROC,	glGetVertexAttribPointervNV,	GHardware_NVVertexProgram,	13	);
			DECLARE_EXT( PFNGLISPROGRAMNVPROC,					glIsProgramNV,					GHardware_NVVertexProgram,	14	);
			DECLARE_EXT( PFNGLLOADPROGRAMNVPROC,				glLoadProgramNV,				GHardware_NVVertexProgram,	15	);
			DECLARE_EXT( PFNGLPROGRAMPARAMETER4DNVPROC,			glProgramParameter4dNV,			GHardware_NVVertexProgram,	16	);
			DECLARE_EXT( PFNGLPROGRAMPARAMETER4DVNVPROC,		glProgramParameters4dvNV,		GHardware_NVVertexProgram,	17	);
			DECLARE_EXT( PFNGLPROGRAMPARAMETER4FNVPROC,			glProgramParameter4fNV,			GHardware_NVVertexProgram,	18	);
			DECLARE_EXT( PFNGLPROGRAMPARAMETER4FVNVPROC,		glProgramParameter4fvNV,		GHardware_NVVertexProgram,	19	);
			DECLARE_EXT( PFNGLREQUESTRESIDENTPROGRAMSNVPROC,	glRequestResidentProgramsNV,	GHardware_NVVertexProgram,	20	);
			DECLARE_EXT( PFNGLTRACKMATRIXNVPROC,				glTrackMatrixNV,				GHardware_NVVertexProgram,	21	);
			DECLARE_EXT( PFNGLVERTEXATTRIBPOINTERNVPROC,		glVertexAttribPointerNV,		GHardware_NVVertexProgram,	22	);
			DECLARE_EXT( PFNGLVERTEXATTRIB1DNVPROC,				glVertexAttrib1dNV,				GHardware_NVVertexProgram,	23	);
			DECLARE_EXT( PFNGLVERTEXATTRIB1DVNVPROC,			glVertexAttrib1dvNV,			GHardware_NVVertexProgram,	24	);
			DECLARE_EXT( PFNGLVERTEXATTRIB1FNVPROC,				glVertexAttrib1fNV,				GHardware_NVVertexProgram,	25	);
			DECLARE_EXT( PFNGLVERTEXATTRIB1FVNVPROC,			glVertexAttrib1fvNV,			GHardware_NVVertexProgram,	26	);
			DECLARE_EXT( PFNGLVERTEXATTRIB1SNVPROC,				glVertexAttrib1sNV,				GHardware_NVVertexProgram,	27	);
			DECLARE_EXT( PFNGLVERTEXATTRIB1SVNVPROC,			glVertexAttrib1svNV,			GHardware_NVVertexProgram,	28	);
			DECLARE_EXT( PFNGLVERTEXATTRIB2DNVPROC,				glVertexAttrib2dNV,				GHardware_NVVertexProgram,	29	);
			DECLARE_EXT( PFNGLVERTEXATTRIB2DVNVPROC,			glVertexAttrib2dvNV,			GHardware_NVVertexProgram,	30	);
			DECLARE_EXT( PFNGLVERTEXATTRIB2FNVPROC,				glVertexAttrib2fNV,				GHardware_NVVertexProgram,	31	);
			DECLARE_EXT( PFNGLVERTEXATTRIB2FVNVPROC,			glVertexAttrib2fvNV,			GHardware_NVVertexProgram,	32	);
			DECLARE_EXT( PFNGLVERTEXATTRIB2SNVPROC,				glVertexAttrib2sNV,				GHardware_NVVertexProgram,	33	);
			DECLARE_EXT( PFNGLVERTEXATTRIB2SVNVPROC,			glVertexAttrib2svNV,			GHardware_NVVertexProgram,	34	);
			DECLARE_EXT( PFNGLVERTEXATTRIB3DNVPROC,				glVertexAttrib3dNV,				GHardware_NVVertexProgram,	35	);
			DECLARE_EXT( PFNGLVERTEXATTRIB3DVNVPROC,			glVertexAttrib3dvNV,			GHardware_NVVertexProgram,	36	);
			DECLARE_EXT( PFNGLVERTEXATTRIB3FNVPROC,				glVertexAttrib3fNV,				GHardware_NVVertexProgram,	37	);
			DECLARE_EXT( PFNGLVERTEXATTRIB3FVNVPROC,			glVertexAttrib3fvNV,			GHardware_NVVertexProgram,	38	);
			DECLARE_EXT( PFNGLVERTEXATTRIB3SNVPROC,				glVertexAttrib3sNV,				GHardware_NVVertexProgram,	39	);
			DECLARE_EXT( PFNGLVERTEXATTRIB3SVNVPROC,			glVertexAttrib3svNV,			GHardware_NVVertexProgram,	40	);
			DECLARE_EXT( PFNGLVERTEXATTRIB4DNVPROC,				glVertexAttrib4dNV,				GHardware_NVVertexProgram,	41	);
			DECLARE_EXT( PFNGLVERTEXATTRIB4DVNVPROC,			glVertexAttrib4dvNV,			GHardware_NVVertexProgram,	42	);
			DECLARE_EXT( PFNGLVERTEXATTRIB4FNVPROC,				glVertexAttrib4fNV,				GHardware_NVVertexProgram,	43	);
			DECLARE_EXT( PFNGLVERTEXATTRIB4FVNVPROC,			glVertexAttrib4fvNV,			GHardware_NVVertexProgram,	44	);
			DECLARE_EXT( PFNGLVERTEXATTRIB4SNVPROC,				glVertexAttrib4sNV,				GHardware_NVVertexProgram,	45	);
			DECLARE_EXT( PFNGLVERTEXATTRIB4SVNVPROC,			glVertexAttrib4svNV,			GHardware_NVVertexProgram,	46	);
			DECLARE_EXT( PFNGLVERTEXATTRIB4UBVNVPROC,			glVertexAttrib4ubvNV,			GHardware_NVVertexProgram,	47	);
			DECLARE_EXT( PFNGLVERTEXATTRIBS1DVNVPROC,			glVertexAttribs1dvNV,			GHardware_NVVertexProgram,	48	);
			DECLARE_EXT( PFNGLVERTEXATTRIBS1FVNVPROC,			glVertexAttribs1fvNV,			GHardware_NVVertexProgram,	49	);
			DECLARE_EXT( PFNGLVERTEXATTRIBS1SVNVPROC,			glVertexAttribs1svNV,			GHardware_NVVertexProgram,	50	);
			DECLARE_EXT( PFNGLVERTEXATTRIBS2DVNVPROC,			glVertexAttribs2dvNV,			GHardware_NVVertexProgram,	51	);
			DECLARE_EXT( PFNGLVERTEXATTRIBS2FVNVPROC,			glVertexAttribs2fvNV,			GHardware_NVVertexProgram,	52	);
			DECLARE_EXT( PFNGLVERTEXATTRIBS2SVNVPROC,			glVertexAttribs2svNV,			GHardware_NVVertexProgram,	53	);
			DECLARE_EXT( PFNGLVERTEXATTRIBS3DVNVPROC,			glVertexAttribs3dvNV,			GHardware_NVVertexProgram,	54	);
			DECLARE_EXT( PFNGLVERTEXATTRIBS3FVNVPROC,			glVertexAttribs3fvNV,			GHardware_NVVertexProgram,	55	);
			DECLARE_EXT( PFNGLVERTEXATTRIBS3SVNVPROC,			glVertexAttribs3svNV,			GHardware_NVVertexProgram,	56	);
			DECLARE_EXT( PFNGLVERTEXATTRIBS4DVNVPROC,			glVertexAttribs4dvNV,			GHardware_NVVertexProgram,	57	);
			DECLARE_EXT( PFNGLVERTEXATTRIBS4FVNVPROC,			glVertexAttribs4fvNV,			GHardware_NVVertexProgram,	58	);
			DECLARE_EXT( PFNGLVERTEXATTRIBS4SVNVPROC,			glVertexAttribs4svNV,			GHardware_NVVertexProgram,	59	);
			DECLARE_EXT( PFNGLVERTEXATTRIBS4UBVNVPROC,			glVertexAttribs4ubvNV,			GHardware_NVVertexProgram,	60	);

			//	ARB vertex program
			DECLARE_EXT( PFNGLVERTEXATTRIB1SARB,				glVertexAttrib1sARB,			GHardware_ARBVertexProgram,	0	);
			DECLARE_EXT( PFNGLVERTEXATTRIB1FARB,				glVertexAttrib1fARB,			GHardware_ARBVertexProgram,	1	);
			DECLARE_EXT( PFNGLVERTEXATTRIB1DARB,				glVertexAttrib1dARB,			GHardware_ARBVertexProgram,	2	);
			DECLARE_EXT( PFNGLVERTEXATTRIB2SARB,				glVertexAttrib2sARB,			GHardware_ARBVertexProgram,	3	);
			DECLARE_EXT( PFNGLVERTEXATTRIB2FARB,				glVertexAttrib2fARB,			GHardware_ARBVertexProgram,	4	);
			DECLARE_EXT( PFNGLVERTEXATTRIB2DARB,				glVertexAttrib2dARB,			GHardware_ARBVertexProgram,	5	);
			DECLARE_EXT( PFNGLVERTEXATTRIB3SARB,				glVertexAttrib3sARB,			GHardware_ARBVertexProgram,	6	);
			DECLARE_EXT( PFNGLVERTEXATTRIB3FARB,				glVertexAttrib3fARB,			GHardware_ARBVertexProgram,	7	);
			DECLARE_EXT( PFNGLVERTEXATTRIB3DARB,				glVertexAttrib3dARB,			GHardware_ARBVertexProgram,	8	);
			DECLARE_EXT( PFNGLVERTEXATTRIB4SARB,				glVertexAttrib4sARB,			GHardware_ARBVertexProgram,	9	);
			DECLARE_EXT( PFNGLVERTEXATTRIB4FARB,				glVertexAttrib4fARB,			GHardware_ARBVertexProgram,	10	);
			DECLARE_EXT( PFNGLVERTEXATTRIB4DARB,				glVertexAttrib4dARB,			GHardware_ARBVertexProgram,	11	);
			DECLARE_EXT( PFNGLVERTEXATTRIB4NUBARB,				glVertexAttrib4NubARB,			GHardware_ARBVertexProgram,	12	);
			DECLARE_EXT( PFNGLVERTEXATTRIB1SVARB,				glVertexAttrib1svARB,			GHardware_ARBVertexProgram,	13	);
			DECLARE_EXT( PFNGLVERTEXATTRIB1FVARB,				glVertexAttrib1fvARB,			GHardware_ARBVertexProgram,	14	);
			DECLARE_EXT( PFNGLVERTEXATTRIB1DVARB,				glVertexAttrib1dvARB,			GHardware_ARBVertexProgram,	15	);
			DECLARE_EXT( PFNGLVERTEXATTRIB2SVARB,				glVertexAttrib2svARB,			GHardware_ARBVertexProgram,	16	);
			DECLARE_EXT( PFNGLVERTEXATTRIB2FVARB,				glVertexAttrib2fvARB,			GHardware_ARBVertexProgram,	17	);
			DECLARE_EXT( PFNGLVERTEXATTRIB2DVARB,				glVertexAttrib2dvARB,			GHardware_ARBVertexProgram,	18	);
			DECLARE_EXT( PFNGLVERTEXATTRIB3SVARB,				glVertexAttrib3svARB,			GHardware_ARBVertexProgram,	19	);
			DECLARE_EXT( PFNGLVERTEXATTRIB3FVARB,				glVertexAttrib3fvARB,			GHardware_ARBVertexProgram,	20	);
			DECLARE_EXT( PFNGLVERTEXATTRIB3DVARB,				glVertexAttrib3dvARB,			GHardware_ARBVertexProgram,	21	);
			DECLARE_EXT( PFNGLVERTEXATTRIB4BVARB,				glVertexAttrib4bvARB,			GHardware_ARBVertexProgram,	22	);
			DECLARE_EXT( PFNGLVERTEXATTRIB4SVARB,				glVertexAttrib4svARB,			GHardware_ARBVertexProgram,	23	);
			DECLARE_EXT( PFNGLVERTEXATTRIB4IVARB,				glVertexAttrib4ivARB,			GHardware_ARBVertexProgram,	24	);
			DECLARE_EXT( PFNGLVERTEXATTRIB4UBVARB,				glVertexAttrib4ubvARB,			GHardware_ARBVertexProgram,	25	);
			DECLARE_EXT( PFNGLVERTEXATTRIB4USVARB,				glVertexAttrib4usvARB,			GHardware_ARBVertexProgram,	26	);
			DECLARE_EXT( PFNGLVERTEXATTRIB4UIVARB,				glVertexAttrib4uivARB,			GHardware_ARBVertexProgram,	27	);
			DECLARE_EXT( PFNGLVERTEXATTRIB4FVARB,				glVertexAttrib4fvARB,			GHardware_ARBVertexProgram,	28	);
			DECLARE_EXT( PFNGLVERTEXATTRIB4DVARB,				glVertexAttrib4dvARB,			GHardware_ARBVertexProgram,	29	);
			DECLARE_EXT( PFNGLVERTEXATTRIB4NBVARB,				glVertexAttrib4NbvARB,			GHardware_ARBVertexProgram,	30	);
			DECLARE_EXT( PFNGLVERTEXATTRIB4NSVARB,				glVertexAttrib4NsvARB,			GHardware_ARBVertexProgram,	31	);
			DECLARE_EXT( PFNGLVERTEXATTRIB4NIVARB,				glVertexAttrib4NivARB,			GHardware_ARBVertexProgram,	32	);
			DECLARE_EXT( PFNGLVERTEXATTRIB4NUBVARB,				glVertexAttrib4NubvARB,			GHardware_ARBVertexProgram,	33	);
			DECLARE_EXT( PFNGLVERTEXATTRIB4NUSVARB,				glVertexAttrib4NusvARB,			GHardware_ARBVertexProgram,	34	);
			DECLARE_EXT( PFNGLVERTEXATTRIB4NUIVARB,				glVertexAttrib4NuivARB,			GHardware_ARBVertexProgram,	35	);
			DECLARE_EXT( PFNGLVERTEXATTRIBPOINTERARB,			glVertexAttribPointerARB,		GHardware_ARBVertexProgram,	36	);
			DECLARE_EXT( PFNGLENABLEVERTEXATTRIBARRAYARB,		glEnableVertexAttribArrayARB,	GHardware_ARBVertexProgram,	37	);
			DECLARE_EXT( PFNGLDISABLEVERTEXATTRIBARRAYARB,		glDisableVertexAttribArrayARB,	GHardware_ARBVertexProgram,	38	);
			DECLARE_EXT( PFNGLPROGRAMSTRINGARB,					glProgramStringARB,				GHardware_ARBVertexProgram,	39	);
			DECLARE_EXT( PFNGLBINDPROGRAMARB,					glBindProgramARB,				GHardware_ARBVertexProgram,	40	);
			DECLARE_EXT( PFNGLDELETEPROGRAMSARB,				glDeleteProgramsARB,			GHardware_ARBVertexProgram,	41	);
			DECLARE_EXT( PFNGLGENPROGRAMSARB,					glGenProgramsARB,				GHardware_ARBVertexProgram,	42	);
			DECLARE_EXT( PFNGLPROGRAMENVPARAMETER4DARB,			glProgramEnvParameter4dARB,		GHardware_ARBVertexProgram,	43	);
			DECLARE_EXT( PFNGLPROGRAMENVPARAMETER4DVARB,		glProgramEnvParameter4dvARB,	GHardware_ARBVertexProgram,	44	);
			DECLARE_EXT( PFNGLPROGRAMENVPARAMETER4FARB,			glProgramEnvParameter4fARB,		GHardware_ARBVertexProgram,	45	);
			DECLARE_EXT( PFNGLPROGRAMENVPARAMETER4FVARB,		glProgramEnvParameter4fvARB,	GHardware_ARBVertexProgram,	46	);
			DECLARE_EXT( PFNGLPROGRAMLOCALPARAMETER4DARB,		glProgramLocalParameter4dARB,	GHardware_ARBVertexProgram,	47	);
			DECLARE_EXT( PFNGLPROGRAMLOCALPARAMETER4DVARB,		glProgramLocalParameter4dvARB,	GHardware_ARBVertexProgram,	48	);
			DECLARE_EXT( PFNGLPROGRAMLOCALPARAMETER4FARB,		glProgramLocalParameter4fARB,	GHardware_ARBVertexProgram,	49	);
			DECLARE_EXT( PFNGLPROGRAMLOCALPARAMETER4FVARB,		glProgramLocalParameter4fvARB,	GHardware_ARBVertexProgram,	50	);
			DECLARE_EXT( PFNGLGETPROGRAMENVPARAMETERDVARB,		glGetProgramEnvParameterdvARB,	GHardware_ARBVertexProgram,	51	);
			DECLARE_EXT( PFNGLGETPROGRAMENVPARAMETERFVARB,		glGetProgramEnvParameterfvARB,		GHardware_ARBVertexProgram,	52	);
			DECLARE_EXT( PFNGLGETPROGRAMLOCALPARAMETERDVARB,	glGetProgramLocalParameterdvARB,	GHardware_ARBVertexProgram,	53	);
			DECLARE_EXT( PFNGLGETPROGRAMLOCALPARAMETERFVARB,	glGetProgramLocalParameterfvARB,	GHardware_ARBVertexProgram,	54	);
			DECLARE_EXT( PFNGLGETPROGRAMIVARB,					glGetProgramivARB,				GHardware_ARBVertexProgram,	55	);
			DECLARE_EXT( PFNGLGETPROGRAMSTRINGARB,				glGetProgramStringARB,			GHardware_ARBVertexProgram,	56	);
			DECLARE_EXT( PFNGLGETVERTEXATTRIBDVARB,				glGetVertexAttribdvARB,			GHardware_ARBVertexProgram,	57	);
			DECLARE_EXT( PFNGLGETVERTEXATTRIBFVARB,				glGetVertexAttribfvARB,			GHardware_ARBVertexProgram,	58	);
			DECLARE_EXT( PFNGLGETVERTEXATTRIBIVARB,				glGetVertexAttribivARB,			GHardware_ARBVertexProgram,	59	);
			DECLARE_EXT( PFNGLGETVERTEXATTRIBPOINTERVARB,		glGetVertexAttribPointervARB,	GHardware_ARBVertexProgram,	60	);
			DECLARE_EXT( PFNGLISPROGRAMARB,						glIsProgramARB,					GHardware_ARBVertexProgram,	61	);
		};

		namespace TLShader
		{
			//	gr: shader support - currently wrapped here. will be moved when shader system is implemented
			inline Bool			InitHardware(u32 HardwareIndex)		{	return FALSE;	}
		};

	}
}

