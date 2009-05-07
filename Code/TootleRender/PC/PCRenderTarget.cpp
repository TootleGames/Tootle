#include "PCRenderTarget.h"
#include "PCRender.h"
#include "PCOpenglExt.h"
#include "../TRenderNode.h"
#include "../TScreen.h"
#include <TootleAsset/TLAsset.h>
#include <TootleAsset/TMesh.h>
#include <TootleAsset/TShader.h>



//#define FORCE_RENDERNODE_CLEAR		//	even if our clear colour is opaque, clear with a render node regardless


TLRender::Platform::RenderTarget::RenderTarget(const TRef& Ref) :
	TRenderTarget	( Ref )
{
}


//-------------------------------------------------------
//
//-------------------------------------------------------
Bool TLRender::Platform::RenderTarget::BeginDraw(const Type4<s32>& RenderTargetMaxSize,const Type4<s32>& ViewportMaxSize,const TScreen& Screen)			
{
	//	do base stuff
	if ( !TRenderTarget::BeginDraw( RenderTargetMaxSize, ViewportMaxSize, Screen) )
		return FALSE;

	//	platform specific stuff...

	//	enable/disable antialiasing
	if ( GetFlag( Flag_AntiAlias ) )
		glEnable( GL_MULTISAMPLE_ARB );
	else
		glDisable( GL_MULTISAMPLE_ARB );

	//	clear render target (viewport has been set)
	GLbitfield ClearMask = 0x0;
	if ( GetFlag( Flag_ClearColour ) )
	{
#if !defined(TLRENDER_DISABLE_CLEAR) && !defined(FORCE_RENDERNODE_CLEAR)
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
		glClearColor( m_ClearColour.GetRedf(),  m_ClearColour.GetGreenf(),  m_ClearColour.GetBluef(),  m_ClearColour.GetAlphaf() );

	//	set the clear depth
	float ClearDepth = GetCamera()->GetFarZ();
	TLMaths::Limit( ClearDepth, 0.f, 1.f );
	glClearDepth( ClearDepth );
	glDepthFunc( GL_LEQUAL );

	//	clear
	glClear( ClearMask );

	Opengl::Debug_CheckForError();

	return TRUE;	
}



//-------------------------------------------------------
//	setup projection mode
//-------------------------------------------------------
Bool TLRender::Platform::RenderTarget::BeginProjectDraw(TLRender::TProjectCamera* pCamera,TLRender::TScreenShape ScreenShape)
{
	//	get the camera
	//	init projection matrix
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	//	get view box
	const TLMaths::TBox2D& ScreenViewBox = pCamera->GetScreenViewBox();
	
	//	set projection matrix - 
	//	gr: note, Bottom and Top are the WRONG way around to invert opengl's upside coordinate system and makes things simpiler in our own code
	glFrustum( ScreenViewBox.GetLeft(), ScreenViewBox.GetRight(), ScreenViewBox.GetTop(), ScreenViewBox.GetBottom(), pCamera->GetNearZ(), pCamera->GetFarZ() );

	//	rotate the view matrix so that UP is properly relative to the new screen
	//	gr: another "thing what is backwards" - as is the -/+ of the shape rotation....
	float ProjectionRotationDeg = -pCamera->GetCameraRoll().GetDegrees();

	if ( ScreenShape == TLRender::ScreenShape_WideRight )
		ProjectionRotationDeg -= 90.f;
	else if ( ScreenShape == TLRender::ScreenShape_WideLeft )
		ProjectionRotationDeg += 90.f;

	//	roll around z
	Opengl::SceneRotate( TLMaths::TAngle(ProjectionRotationDeg), float3( 0.f, 0.f, 1.f ) );

	//	setup camera
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	//	setup camera
	m_CameraTransform.SetInvalid();
	m_CameraTransform.SetTranslate( pCamera->GetPosition() * -1.f );

	//	apply look at matrix (rotate)
	const TLMaths::TMatrix& LookAtMatrix = pCamera->GetCameraLookAtMatrix();
	m_pCameraMatrix = &LookAtMatrix;

	Opengl::SceneTransform( m_CameraTransform, m_pCameraMatrix);

	return TRUE;
}


//-------------------------------------------------------
//	clean up projection scene
//-------------------------------------------------------
void TLRender::Platform::RenderTarget::EndProjectDraw()
{
}


//-------------------------------------------------------
//	setup projection mode
//-------------------------------------------------------
Bool TLRender::Platform::RenderTarget::BeginOrthoDraw(TLRender::TOrthoCamera* pCamera,TLRender::TScreenShape ScreenShape)
{
	//	setup ortho projection
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	//	get ortho dimensions box
	const TLMaths::TBox2D& OrthoBox = pCamera->GetOrthoRenderTargetBox();
	
	//	rotate the view matrix so that UP is properly relative to the new screen
	//	gr: another "thing what is backwards" - as is the -/+ of the shape rotation....
	float ProjectionRotationDeg = -pCamera->GetCameraRoll().GetDegrees();

	if ( ScreenShape == TLRender::ScreenShape_WideRight )
		ProjectionRotationDeg -= 90.f;
	else if ( ScreenShape == TLRender::ScreenShape_WideLeft )
		ProjectionRotationDeg += 90.f;

	//	roll around z
	Opengl::SceneRotate( TLMaths::TAngle(ProjectionRotationDeg), float3( 0.f, 0.f, 1.f ) );

	//	set the world coordinates
	glOrtho( OrthoBox.GetLeft(), OrthoBox.GetRight(), OrthoBox.GetBottom(), OrthoBox.GetTop(), pCamera->GetNearZ(), pCamera->GetFarZ() );

	Opengl::Debug_CheckForError();		

	Bool UseClearRenderNode = (m_ClearColour.IsVisible() && m_ClearColour.IsTransparent());
	
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
		m_pRenderNodeClear->SetSize( OrthoBox, -1.f/*pCamera->GetNearZ()*/ );
		m_pRenderNodeClear->SetColour( m_ClearColour );
	}
	else
	{
		//	remove the clear object
		m_pRenderNodeClear = NULL;
	}

	//	setup camera
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	//	translate
	m_CameraTransform.SetInvalid();

	m_CameraTransform.SetTranslate( pCamera->GetPosition() );

	//	apply look at matrix (rotate)
//	const TLMaths::TMatrix& LookAtMatrix = pCamera->GetCameraLookAtMatrix();
//	m_CameraTransform.SetMatrix( LookAtMatrix );
	Opengl::SceneTransform( m_CameraTransform );

	return TRUE;
}


//-------------------------------------------------------
//	clean up ortho scene
//-------------------------------------------------------
void TLRender::Platform::RenderTarget::EndOrthoDraw()
{
}



//-----------------------------------------------------------
//	save off current scene 
//	gr: no more push attribs, it's not supported on opengl ES so handle it ourselves
//-----------------------------------------------------------
void TLRender::Platform::RenderTarget::BeginScene()
{
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
	{
		Opengl::SceneTransform( m_CameraTransform, m_pCameraMatrix );
	}
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

