#include "PCRenderTarget.h"
#include "PCRender.h"
#include "PCOpenglExt.h"
#include "../TRenderNode.h"
#include <TootleAsset/TLAsset.h>
#include <TootleAsset/TMesh.h>
#include <TootleAsset/TShader.h>




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
	glEnable( GL_SCISSOR_TEST );
	
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

	//	enable/disable antialiasing
	if ( GetFlag( Flag_AntiAlias ) )
		glEnable( GL_MULTISAMPLE_ARB );
	else
		glDisable( GL_MULTISAMPLE_ARB );

	//	clear render target (viewport has been set)
	GLbitfield ClearMask = 0x0;
	if ( GetFlag( Flag_ClearColour ) )
	{
		//	if the clear colour has an alpha, we dont use the opengl clear as it doesnt support alpha
		if ( !m_ClearColour.IsTransparent() )
			ClearMask |= GL_COLOR_BUFFER_BIT;
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
	glClearDepth( ClearDepth );
	glDepthFunc( GL_LEQUAL );

	//	clear
	glClear( ClearMask );

	Opengl::Debug_CheckForError();		

	return TRUE;	
}


void gluPerspective(GLdouble fovy, GLdouble aspect, GLdouble zNear, GLdouble zFar)
{
	GLdouble xmin, xmax, ymin, ymax;

	ymax = zNear * tan(fovy * PI / 360.0);
	ymin = -ymax;
	xmin = ymin * aspect;
	xmax = ymax * aspect;

	glFrustum(xmin, xmax, ymin, ymax, zNear, zFar);
}


//-------------------------------------------------------
//	setup projection mode
//-------------------------------------------------------
Bool TLRender::Platform::RenderTarget::BeginProjectDraw(const Type4<s32>& ViewportSize)
{
	//	init projection matrix
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	
	//	get the camera
	TLRender::TProjectCamera* pCamera = GetCamera().GetObject<TLRender::TProjectCamera>();
	if ( !pCamera )
		return FALSE;
	
	//	work out a frustum matrix for the perspective from the camera
	float AspectRatio = (float)ViewportSize.Width() / (float)ViewportSize.Height();
	TLMaths::TMatrix FrustumMatrix;
	pCamera->GetFrustumMatrix( FrustumMatrix, AspectRatio );

	//	do translation with this matrix to make the frustum
	//	gr: this works... but further away than with gluperspective...
	//	Translate( FrustumMatrix );
	gluPerspective( pCamera->GetHorzFov().GetDegrees(), AspectRatio, pCamera->GetNearZ(), pCamera->GetFarZ() );

	//	setup camera
	glMatrixMode(GL_MODELVIEW);
	BeginSceneReset();

	return TRUE;
}


//-------------------------------------------------------
//	setup projection mode
//-------------------------------------------------------
void TLRender::Platform::RenderTarget::EndProjectDraw()
{
	//	restore projection
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();

	//	restore scene
	glMatrixMode(GL_MODELVIEW);
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
	glPushMatrix();
	glLoadIdentity();

	//	change orthographic projection
	Type4<float> OrthoSize;
	pCamera->GetOrthoSize( OrthoSize, ViewportSize );
	glOrtho( OrthoSize.Left(), OrthoSize.Right(), OrthoSize.Bottom(), OrthoSize.Top(), GetCamera()->GetNearZ(), GetCamera()->GetFarZ() );

	Opengl::Debug_CheckForError();		
	
	//	clear the screen manually if we need to apply alpha
	if ( m_ClearColour.GetAlpha() > 0.f && m_ClearColour.IsTransparent() )
	{
		if ( !m_pRenderNodeClear )
		{
			m_pRenderNodeClear = new TRenderNodeClear("Clear","Clear");
		}
		m_pRenderNodeClear->SetSize( OrthoSize, -1.f );
		m_pRenderNodeClear->SetColour( m_ClearColour );
	}
	else
	{
		//	remove the clear object
		m_pRenderNodeClear = NULL;
	}

	//	setup camera
	glMatrixMode(GL_MODELVIEW);
	BeginSceneReset();

	return TRUE;
}


//-------------------------------------------------------
//	setup projection mode
//-------------------------------------------------------
void TLRender::Platform::RenderTarget::EndOrthoDraw()
{
	//	restore projection
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	
	//	restore scene
	glMatrixMode(GL_MODELVIEW);
	EndScene();
}



//-----------------------------------------------------------
//	save off current scene 
//	gr: no more push attribs, it's not supported on opengl ES so handle it ourselves
//-----------------------------------------------------------
void TLRender::Platform::RenderTarget::BeginScene()
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
		TranslateCamera();
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


//-----------------------------------------------------------
//	save off the current scene's camera matrix
//-----------------------------------------------------------
Bool Platform::RenderTarget::GetSceneMatrix(TLMaths::TMatrix& Matrix)
{
	glGetFloatv( GL_MODELVIEW_MATRIX, Matrix );

	return TRUE;
}

//--------------------------------------------------------------
//	do simple transformation to the scene
//--------------------------------------------------------------
void Platform::RenderTarget::Translate(const TLMaths::TTransform& Transform)
{
	//	gr: do this in the same order as the Transform() functions?
	//		currently in old-render-code-order
	if ( Transform.HasTranslate() )
	{
		const float3& Trans = Transform.GetTranslate();
		glTranslatef( Trans.x, Trans.y, Trans.z );
	}

	if ( Transform.HasScale() )
	{
		const float3& Scale = Transform.GetScale();
		glScalef( Scale.x, Scale.y, Scale.z );
	}

	if ( Transform.HasMatrix() )
	{
		glMultMatrixf( Transform.GetMatrix().GetData() );
	}

	if ( Transform.HasRotation() )
	{
		TLMaths::TMatrix RotMatrix;
		TLMaths::QuaternionToMatrix( Transform.GetRotation(), RotMatrix );
		glMultMatrixf( RotMatrix.GetData() );
	}
}


//-------------------------------------------------------------
//	set current render colour (glColourf)
//-------------------------------------------------------------
void Platform::RenderTarget::SetSceneColour(const TColour& SceneColour)
{
	//	no changes
	if ( m_SceneColour == SceneColour )
		return;

	//	update scene colour
	m_SceneColour = SceneColour;

	//	set scene colour
	glColor4fv( m_SceneColour.GetData() );

	//	setup alpha blending
	if ( m_SceneColour.IsTransparent() )
	{
		glEnable( GL_BLEND );
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}
	else
	{
		glDisable( GL_BLEND );
	}
}

	

//-------------------------------------------------------------
//	render mesh asset
//-------------------------------------------------------------
TLRender::DrawResult TLRender::Platform::RenderTarget::DrawMesh(TLAsset::TMesh& Mesh,const TLRender::TRenderNode* pRenderNode,const TFlags<TLRender::TRenderNode::RenderFlags::Flags>* pForceFlags)
{
	//	get pointers to mesh data
	const TArray<float3>* pVertexes = &Mesh.GetVertexes();
	
	//	check mesh has some required data
	if ( !pVertexes->GetSize() )
		return TLRender::Draw_Empty;
	
	const TArray<TLAsset::TFixedVertex>* pFixedVertexes	= &Mesh.GetFixedVertexes();
	const TArray<TColour>* pColours						= &Mesh.GetColours();
	const TArray<TLAsset::TMesh::Triangle>* pTriangles	= &Mesh.GetTriangles();
	const TArray<TLAsset::TMesh::Tristrip>* pTristrips	= &Mesh.GetTristrips();
	const TArray<TLAsset::TMesh::Trifan>* pTrifans		= &Mesh.GetTrifans();
	const TArray<TLAsset::TMesh::Line>* pLines			= &Mesh.GetLines();
	
	//	list of what parts we're going to render
	TFlags<TRenderNode::RenderFlags::Flags> RenderFlags = pForceFlags ? *pForceFlags : pRenderNode->GetRenderFlags();

	//	manipulate colour usage
	if ( !pColours )
		RenderFlags.Clear( TRenderNode::RenderFlags::UseVertexColours );
	if ( !RenderFlags( TRenderNode::RenderFlags::UseVertexColours ) )
	{
		pColours = NULL;
		pFixedVertexes = NULL;
	}

	TRef VBOMeshRef = RenderFlags(TRenderNode::RenderFlags::EnableVBO) ? Mesh.GetAssetRef() : TRef();

	//	gr: interleaving vertex data results in glColorXX having no effect, so if the scene colour is not white, force using normal buffers and not interleaved
	if ( m_SceneColour != TColour( 1.f, 1.f, 1.f, 1.f ) )
		pFixedVertexes = NULL;
	


		 
	//	bind vertex data
	if ( RenderFlags(TRenderNode::RenderFlags::EnableFixedVerts) && pFixedVertexes && pFixedVertexes->GetSize() )
	{
		Opengl::BindFixedVertexes( pFixedVertexes, VBOMeshRef );
	}
	else
	{
		Opengl::BindVertexes( pVertexes, VBOMeshRef );
		Opengl::BindColours( pColours, VBOMeshRef );
	}
	

	//	wireframe render
	if ( RenderFlags( TRenderNode::RenderFlags::Debug_Wireframe ) )
	{
		//	force white colour by unbinding colours and setting scene colour
		SetSceneColour( TColour(1.f,1.f,1.f,1.f) );
		Opengl::BindColours( NULL, VBOMeshRef );
		glLineWidth(1.f);

		//	set up rendering params
		glDisable( GL_DEPTH_TEST );
		glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );

		//	render polygons
		Opengl::DrawPrimitives( GL_TRIANGLES,		pTriangles );
		Opengl::DrawPrimitives( GL_TRIANGLE_STRIP,	pTristrips );
		Opengl::DrawPrimitives( GL_TRIANGLE_FAN,	pTrifans );

		//	dont do another wireframe pass
		RenderFlags.Clear( TRenderNode::RenderFlags::Debug_Wireframe );

		//	rebind colours and re-set scene setup
		Opengl::BindColours( pColours, VBOMeshRef );
		glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
	}



	//	normal render 

	//	enable/disable depth test
	if ( RenderFlags( TRenderNode::RenderFlags::DepthRead ) )
	{
		glEnable( GL_DEPTH_TEST );
	}
	else
	{
		glDisable( GL_DEPTH_TEST );
	}

	//	enable/disable depth writing
	if ( RenderFlags( TRenderNode::RenderFlags::DepthWrite ) )
	{
		glDepthMask( GL_TRUE );
	}
	else
	{
		glDepthMask( GL_FALSE );
	}

	
	//	setup line width if required
	if ( pLines && pLines->GetSize() > 0 ) 
	{
		float LineWidth = pRenderNode->GetLineWidth();
		if ( LineWidth < 1.f )
		{
			//	gr: need some kinda world->screen space conversion.. drawing is in pixels, widths are in world space
			float MeshLineWidth = Mesh.GetLineWidth() * 2.f;
			MeshLineWidth *= 320.f / 100.f;	//	ortho scale
			LineWidth = MeshLineWidth;

			//	min width
			if ( LineWidth < 1.f )
				LineWidth = 1.f;
		}
			
		glLineWidth( LineWidth );
	}
		
	//	draw primitives
	Opengl::DrawPrimitives( GL_TRIANGLES,		pTriangles );
	Opengl::DrawPrimitives( GL_TRIANGLE_STRIP,	pTristrips );
	Opengl::DrawPrimitives( GL_TRIANGLE_FAN,	pTrifans );
	Opengl::DrawPrimitives( GL_LINE_STRIP,		pLines );
	
	//	draw points like a primitive
	if ( RenderFlags( TRenderNode::RenderFlags::Debug_Points ) )
	{
		glPointSize(4.f);
		Opengl::DrawPrimitivePoints( GL_POINTS, pVertexes );
	}
	

	
	//	drawn okay
	return TLRender::Draw_Okay;
}


