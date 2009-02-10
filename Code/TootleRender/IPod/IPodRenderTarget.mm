#include "IPodRenderTarget.h"
#include "IPodRender.h"

#import <OpenGLES/EAGL.h>
#import <OpenGLES/ES1/gl.h>
#import <OpenGLES/ES1/glext.h>
#import <OpenGLES/EAGLDrawable.h>


//#define FORCE_RENDERNODE_CLEAR		//	even if our clear colour is opaque, clear with a render node regardless


TLRender::Platform::RenderTarget::RenderTarget(const TRef& Ref) :
	TRenderTarget	( Ref )
{
}


//-------------------------------------------------------
//
//-------------------------------------------------------
Bool TLRender::Platform::RenderTarget::BeginDraw(const Type4<s32>& MaxSize)			
{
	//	do base checks
	if ( !TRenderTarget::BeginDraw(MaxSize) )
		return FALSE;
	
	Type4<s32> ViewportSize;
	if ( !GetViewportSize( ViewportSize, MaxSize ) )
		return FALSE;
	
	//	setup viewport
	glViewport( ViewportSize.Left(), ViewportSize.Top(), ViewportSize.Width(), ViewportSize.Height() );
	
	//	scissor everything outside the viewport
	glScissor( ViewportSize.Left(), ViewportSize.Top(), ViewportSize.Width(), ViewportSize.Height() );
	
	Opengl::Debug_CheckForError();		
	
	//	do projection vs orthographic setup
	if ( GetCamera()->IsOrtho() )
	{
		if ( !BeginOrthoDraw( ViewportSize ) )
			return FALSE;
	}
	else
	{
		if ( !BeginProjectDraw( ViewportSize ) )
			return FALSE;
	}
	
	//	clear render target (viewport has been set)
	GLbitfield ClearMask = 0x0;
	if ( GetFlag( Flag_ClearColour ) )
	{
#ifndef FORCE_RENDERNODE_CLEAR
		//	if the clear colour has an alpha, we dont use the opengl clear as it doesnt support alpha
		if ( !m_ClearColour.IsTransparent() )
			ClearMask |= GL_COLOR_BUFFER_BIT;
#endif
	}

	if ( GetFlag( Flag_ClearDepth ) )	
	{
		ClearMask |= GL_DEPTH_BUFFER_BIT;
	}

	if ( GetFlag( Flag_ClearStencil ) )	
	{
		ClearMask |= GL_STENCIL_BUFFER_BIT;
	}

	//	set the clear colour
	if ( ( ClearMask & GL_COLOR_BUFFER_BIT ) != 0x0 )
		glClearColor( m_ClearColour.GetRed(),  m_ClearColour.GetGreen(),  m_ClearColour.GetBlue(),  m_ClearColour.GetAlpha() );

	//	set the clear depth
	float ClearDepth = GetCamera()->GetFarZ();
	TLMaths::Limit( ClearDepth, 0.f, 1.f );
	glClearDepthf( ClearDepth );
	glDepthFunc( GL_LEQUAL );

	//	clear
	glClear( ClearMask );

	Opengl::Debug_CheckForError();		

	return TRUE;	
}


Bool gluPerspective(float fovy, float aspect, float zNear, float zFar)
{
	float xmin, xmax, ymin, ymax;
	
	ymax = zNear * tan(fovy * PI / 360.0);
	ymin = -ymax;
	xmin = ymin * aspect;
	xmax = ymax * aspect;
	
	glFrustumf(xmin, xmax, ymin, ymax, zNear, zFar);
	return TRUE;
}


//-------------------------------------------------------
//	setup projection mode
//-------------------------------------------------------
Bool TLRender::Platform::RenderTarget::BeginProjectDraw(const Type4<s32>& ViewportSize)
{
	//	get the camera
	TLRender::TProjectCamera* pCamera = GetCamera().GetObject<TLRender::TProjectCamera>();
	if ( !pCamera )
		return FALSE;
	
	//	init projection matrix
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	
	//	work out a frustum matrix for the perspective from the camera
//	float AspectRatio = (float)ViewportSize.Width() / (float)ViewportSize.Height();
	float AspectRatio = pCamera->GetAspectRatio();
//	TLMaths::TMatrix FrustumMatrix;
//	pCamera->GetFrustumMatrix( FrustumMatrix, AspectRatio );

	//	do translation with this matrix to make the frustum
	//	gr: this works... but further away than with gluperspective...
	//	Translate( FrustumMatrix );
	
	gluPerspective( pCamera->GetHorzFov().GetDegrees(), AspectRatio, pCamera->GetNearZ(), pCamera->GetFarZ() );

	//	update projection matrix
	TLMaths::TMatrix& ProjectionMatrix = pCamera->GetProjectionMatrix(TRUE);
	glGetFloatv( GL_PROJECTION_MATRIX, ProjectionMatrix );

	//	setup camera
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	//	translate
	m_CameraTransform.SetInvalid();

	m_CameraTransform.SetTranslate( pCamera->GetPosition() * -1.f );

	//	apply look at matrix (rotate)
	const TLMaths::TMatrix& LookAtMatrix = pCamera->GetCameraLookAtMatrix();
	m_CameraTransform.SetMatrix( LookAtMatrix );

	Opengl::SceneTransform( m_CameraTransform );

	//	update the modelview matrix on the camera
	TLMaths::TMatrix& ModelViewMatrix = pCamera->GetModelViewMatrix(TRUE);
	glGetFloatv( GL_MODELVIEW_MATRIX, ModelViewMatrix );

	//	gr: redundant now, but using temporarily for testing
	BeginSceneReset();

	return TRUE;
}


//-------------------------------------------------------
//	setup projection mode
//-------------------------------------------------------
void TLRender::Platform::RenderTarget::EndProjectDraw()
{
	EndScene();
}


//-------------------------------------------------------
//	setup projection mode
//-------------------------------------------------------
Bool TLRender::Platform::RenderTarget::BeginOrthoDraw(const Type4<s32>& ViewportSize)
{
	if ( !TRenderTarget::BeginOrthoDraw( ViewportSize ) )
		return FALSE;

	//	get the camera
	TLRender::TOrthoCamera* pCamera = GetCamera().GetObject<TLRender::TOrthoCamera>();
	if ( !pCamera )
		return FALSE;

	//	setup ortho projection
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	//	change orthographic projection
	Type4<float> OrthoSize;
	pCamera->GetOrthoSize( OrthoSize, ViewportSize );
	glOrthof( OrthoSize.Left(), OrthoSize.Right(), OrthoSize.Bottom(), OrthoSize.Top(), GetCamera()->GetNearZ(), GetCamera()->GetFarZ() );

	Opengl::Debug_CheckForError();		

	//	update projection matrix
	TLMaths::TMatrix& ProjectionMatrix = pCamera->GetProjectionMatrix(TRUE);
	glGetFloatv( GL_PROJECTION_MATRIX, ProjectionMatrix );
	
	Bool UseClearRenderNode = (m_ClearColour.GetAlpha() > 0.f && m_ClearColour.IsTransparent());
	
#ifdef FORCE_RENDERNODE_CLEAR
	UseClearRenderNode = TRUE;
#endif
	
	//	clear the screen manually if we need to apply alpha
	if ( UseClearRenderNode )
	{
		if ( !m_pRenderNodeClear )
		{
			m_pRenderNodeClear = new TRenderNodeClear("Clear","Clear");
		}
		m_pRenderNodeClear->SetSize( OrthoSize, -1.f, m_ClearColour );
	}
	else
	{
		//	remove the clear object
		m_pRenderNodeClear = NULL;
	}

	//	setup camera
	glMatrixMode(GL_MODELVIEW);
	//	gr: this was missing, but not techniccaly needed, I guess because we push and pop the matricies back to their original state anywa
	glLoadIdentity();

	//	translate
	m_CameraTransform.SetInvalid();

	m_CameraTransform.SetTranslate( pCamera->GetPosition() );

	//	apply look at matrix (rotate)
//	const TLMaths::TMatrix& LookAtMatrix = pCamera->GetCameraLookAtMatrix();
//	m_CameraTransform.SetMatrix( LookAtMatrix );
	Opengl::SceneTransform( m_CameraTransform );

	//	update the modelview matrix on the camera
	TLMaths::TMatrix& ModelViewMatrix = pCamera->GetModelViewMatrix(TRUE);
	glGetFloatv( GL_MODELVIEW_MATRIX, ModelViewMatrix );

	//	gr: redundant now, but using temporarily for testing
	BeginSceneReset();

	return TRUE;
}


//-------------------------------------------------------
//	setup projection mode
//-------------------------------------------------------
void TLRender::Platform::RenderTarget::EndOrthoDraw()
{
	EndScene();
}



//-----------------------------------------------------------
//	save off current scene 
//	gr: no more push attribs, it's not supported on opengl ES so handle it ourselves
//-----------------------------------------------------------
void TLRender::Platform::RenderTarget::BeginScene()
{
	//	very expensive on ipod! so omitted, can be checked with PC as it's all logical code
	#if !defined(TL_TARGET_IPOD)
	{
		//	ensure we're in the right mode... shouldn't be pushing PROJECTion matrixes
		if ( TLDebug::IsEnabled() )
		{
			u32 MatrixMode = GetCurrentMatrixMode();
			if ( MatrixMode != GL_MODELVIEW )
			{
				TLDebug_Break("BeginScene: In wrong GL_MATRIX_MODE");
				glMatrixMode(GL_MODELVIEW);
			}
		}
	}
	#endif
	
	//	save the scene
	glPushMatrix();
	Opengl::Debug_CheckForError();		

	//	keep track of how many scenes we've started to keep the pushing and popping in sync
	m_Debug_SceneCount++;

}


//-----------------------------------------------------------
//	save off current scene and reset camera 
//-----------------------------------------------------------
void TLRender::Platform::RenderTarget::BeginSceneReset(Bool ApplyCamera)
{
	BeginScene();
	
	//	reset opengl scene
	glLoadIdentity();
	
	//	and reset to camera pos
	if ( ApplyCamera )
		Opengl::SceneTransform( m_CameraTransform );
}


//-----------------------------------------------------------
//	restore previous scene
//-----------------------------------------------------------
void TLRender::Platform::RenderTarget::EndScene()
{
	if ( m_Debug_SceneCount <= 0 )
	{
		if ( TLDebug_Break("Ending more scenes than we've begun") )
			return;
		
		//	we have elected to carry on so reset the scenecount to 1 and it'll be set to 0 again
		m_Debug_SceneCount = 1;
	}
	
	if ( m_Debug_SceneCount > 0 )
	{
		glPopMatrix();
		Opengl::Debug_CheckForError();		
		
		m_Debug_SceneCount--;
	}
}


//-----------------------------------------------------------
//	fetch current matrix mode so we dont inadvertently switch modes
//	maybe make this a stack?
//-----------------------------------------------------------
u32 TLRender::Platform::RenderTarget::GetCurrentMatrixMode()
{
	GLint MatrixMode;
	glGetIntegerv( GL_MATRIX_MODE, &MatrixMode );
	
	return MatrixMode;
}




