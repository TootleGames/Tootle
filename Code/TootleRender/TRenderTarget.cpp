#include "TRenderTarget.h"
#include "TScreen.h"
#include <TootleCore/TPtr.h>
#include "TScreenManager.h"


#define DEBUG_RENDER_DATUMS_IN_WORLD	//	if not defined, renders datums in local space which is faster

#if defined(_DEBUG) && !defined(TL_TARGET_IPOD)
//#define DEBUG_DRAW_RENDERZONES
#define DEBUG_DRAW_FRUSTUM
//#define DEBUG_NODE_RENDERED_COUNT
#endif

#define VISIBILITY_FRUSTUM_TEST_FIRST

//#define PREDIVIDE_RENDER_ZONES

#define FORCE_COLOUR	Colour_TColour

//-----------------------------------------------------------
//	
//-----------------------------------------------------------
TLRender::TRenderTarget::TRenderTarget(const TRef& Ref) :
	m_Size				( g_MaxSize, g_MaxSize, g_MaxSize, g_MaxSize ),
	m_Ref				( Ref ),
	m_Debug_SceneCount	( 0 ),
	m_Debug_PolyCount	( 0 ),
	m_Debug_VertexCount	( 0 ),
	m_Debug_NodeCount	( 0 ),
	m_Debug_NodeCulledCount	( 0 ),
	m_pCameraMatrix		( NULL ),
	m_ScreenZ			( 0 )
{
	//	set default flags
	m_Flags.Set( Flag_Enabled );
	m_Flags.Set( Flag_ClearColour );
	m_Flags.Set( Flag_ClearDepth );

	//	gr: turned off default anti-aliasing as it doesn't work on the ipod
//	m_Flags.Set( Flag_AntiAlias );

	//	calc initial size
	OnSizeChanged();
}



//------------------------------------------------------------
//	dont allow render without a camera
//------------------------------------------------------------
Bool TLRender::TRenderTarget::BeginDraw(const Type4<s32>& RenderTargetMaxSize,const Type4<s32>& ViewportMaxSize,const TScreen& Screen)			
{
	m_Debug_NodeCount = 0;
	m_Debug_NodeCulledCount = 0;

	//	render target disabled
	if ( !GetFlags()(Flag_Enabled) )
		return FALSE;

	//	need a camera
	if ( !m_pCamera )
		return FALSE;
	TCamera& Camera = *m_pCamera;

	//	get the size... fails if it's too small to be of any use
	Type4<s32> RenderTargetSize;
	GetSize( RenderTargetSize, RenderTargetMaxSize );
	if ( RenderTargetSize.Width() <= 2 || RenderTargetSize.Height() <= 2 )
		return FALSE;

	//	if we have no root render node, and not going to be clearing the screen, skip render
	Bool WillClear = GetFlags()( Flag_ClearColour ) && (m_ClearColour.IsVisible());
	if ( !WillClear )
	{
		//	won't be clearing, so if we're missing our root node/have none, skip rendering
		if ( !m_RootRenderNodeRef.IsValid() )
			return FALSE;

		TPtr<TRenderNode>& pRootRenderNode = GetRootRenderNode();
		if ( !pRootRenderNode )
		{
			TTempString Debug_String("Warning: render target (");
			GetRef().GetString( Debug_String );
			Debug_String.Append(") missing root render target node (");
			m_RootRenderNodeRef.GetString( Debug_String );
			Debug_String.Append(")");
			TLDebug_Print( Debug_String );
			return FALSE;
		}
	}

	//	clean post render list
	m_TempPostRenderList.Empty();

	//	setup viewport and sissor outside the viewport
	const TLMaths::TBox2D& ViewportBox = Camera.GetViewportBox();
	if ( !ViewportBox.IsValid() )
	{
		TLDebug_Break("Viewport box should be valid");
		return FALSE;
	}
	//	gr: store a integer box for these?
	glViewport( (s32)ViewportBox.GetLeft(), (s32)ViewportBox.GetTop(), (s32)ViewportBox.GetWidth(), (s32)ViewportBox.GetHeight() );
	Opengl::Debug_CheckForError();

	//	gr: todo; cache current scissor box
	const TLMaths::TBox2D& ScissorBox = Camera.GetScissorBox();
	if ( !ScissorBox.IsValid() )
	{
		TLDebug_Break("Scissor box should be valid");
		return FALSE;
	}
	Opengl::EnableScissor( TRUE );
	Opengl::SetScissor( (u32)ScissorBox.GetLeft(), (u32)ScissorBox.GetTop(), (u32)ScissorBox.GetWidth(), (u32)ScissorBox.GetHeight() );
	Opengl::Debug_CheckForError();

	//	do projection vs orthographic setup
	if ( Camera.IsOrtho() )
	{
		if ( !BeginOrthoDraw( static_cast<TLRender::TOrthoCamera&>(Camera), Screen.GetScreenShape() ) )
			return FALSE;
	}
	else
	{
		if ( !BeginProjectDraw( static_cast<TLRender::TProjectCamera&>(Camera), Screen.GetScreenShape() ) )
			return FALSE;
	}

	//	overloaded code should do platform specific stuff now

	return TRUE;	
}


//------------------------------------------------------------
//	start render process
//------------------------------------------------------------
void TLRender::TRenderTarget::Draw()
{
	s32 StartingSceneCount = m_Debug_SceneCount;

	//	render the clear object if we have it
#ifndef TLRENDER_DISABLE_CLEAR
	if ( m_pRenderNodeClear )
	{
		DrawNode( m_pRenderNodeClear.GetObject(), NULL, NULL, TColour( 1.f, 1.f, 1.f, 1.f ), NULL );
	}
#endif

	//	update the camera's zone
	Bool CameraInWorldZone = TRUE;
	if ( m_pRootQuadTreeZone )
	{
		if ( m_pCamera->IsZoneOutOfDate() )
		{
			TPtr<TLMaths::TQuadTreeNode> pCameraQuadTreeNode = m_pCamera;
			m_pCamera->UpdateZone( pCameraQuadTreeNode, m_pRootQuadTreeZone );
		}

		if ( !m_pCamera->GetZone() )
			CameraInWorldZone = FALSE;
	}

	//	if the camera is outside the world zone, then all our render objects must be (assuming world zone is big enough) out of visibility too
	if ( CameraInWorldZone )
	{
		//	draw the root render object and the rest will follow
		TPtr<TRenderNode>& pRootRenderNode = GetRootRenderNode();
		if ( pRootRenderNode )
		{
			DrawNode( pRootRenderNode.GetObject(), NULL, NULL, TColour( 1.f, 1.f, 1.f, 1.f ), m_pRootQuadTreeZone ? m_pCamera.GetObject<TLMaths::TQuadTreeNode>() : NULL );
		}
	}

	//	scene count should be zero after drawing...
	if ( StartingSceneCount != m_Debug_SceneCount )
	{
		TLDebug_Break( TString("More scenes active(%d) than we started with (%d)", m_Debug_SceneCount, StartingSceneCount ) );
		m_Debug_SceneCount = 0;
	}
}


//-------------------------------------------------------
//
//-------------------------------------------------------
void TLRender::TRenderTarget::EndDraw()			
{
	if ( GetCamera()->IsOrtho() )
	{
		EndOrthoDraw();
	}
	else
	{
		EndProjectDraw();
	}

	//	clean post render list
	m_TempPostRenderList.Empty();


	//	print out node/culled count
#ifdef DEBUG_NODE_RENDERED_COUNT
	TTempString DebugString("RenderTarget ");
	GetRef().GetString( DebugString );
	DebugString.Appendf(" rendered %d nodes (%d culled)", m_Debug_NodeCount, m_Debug_NodeCulledCount );
	TLDebug_Print( DebugString );
#endif

#if defined(DEBUG_DRAW_RENDERZONES)||defined(DEBUG_DRAW_FRUSTUM)
	if ( !GetCamera()->IsOrtho() && m_pRootQuadTreeZone )
	{
		BeginSceneReset();

#ifdef DEBUG_DRAW_RENDERZONES
		Debug_DrawZone( m_pRootQuadTreeZone, 0.f, m_pCamera.GetObject() );
#endif

#ifdef DEBUG_DRAW_FRUSTUM
		TProjectCamera* pCamera = GetCamera().GetObject<TProjectCamera>();
		pCamera->CalcFrustum();

		//	really quick bodge
		Opengl::SetLineWidth( 2.f );
		Opengl::EnableDepthRead( FALSE );
		Opengl::EnableAlpha( FALSE );
		Opengl::Unbind();

		TFixedArray<float3,8>& FrustumOblongCorners = pCamera->m_FrustumOblong.GetBoxCorners();

		//	calc the shape for the zone from the frustum
		TLMaths::TBox2D FrustumBox;
		float WorldDepth = 0.f;
		float ViewDepth = pCamera->GetPosition().z + WorldDepth;
		pCamera->GetWorldFrustumPlaneBox2D( ViewDepth, FrustumBox );
		TFixedArray<float3,4> FrustumBoxCorners;
		FrustumBox.GetBoxCorners( FrustumBoxCorners, WorldDepth );

		//	green for oblong frustum
		Opengl::SetSceneColour( TColour( 0.f, 1.f, 0.f, 1.f ) );
		glBegin(GL_LINE_LOOP);
		{
			glVertex3fv( FrustumOblongCorners.ElementAt(4) );
			glVertex3fv( FrustumOblongCorners.ElementAt(5) );
			glVertex3fv( FrustumOblongCorners.ElementAt(6) );
			glVertex3fv( FrustumOblongCorners.ElementAt(7) );
		}
		glEnd();

		//	red for 2D box frustum
		Opengl::SetSceneColour( TColour( 1.f, 0.f, 0.f, 1.f ) );
		glBegin(GL_LINE_LOOP);
		{
			glVertex3fv( FrustumBoxCorners.ElementAt(0) );
			glVertex3fv( FrustumBoxCorners.ElementAt(1) );
			glVertex3fv( FrustumBoxCorners.ElementAt(2) );
			glVertex3fv( FrustumBoxCorners.ElementAt(3) );
		}
		glEnd();


		/*

		TLMaths::TBoxOB PlaneBox;
		float worldz = 0.f;
		float viewz = pCamera->GetPosition().z + worldz;
		pCamera->GetPlaneBox( viewz, PlaneBox );
		TLMaths::TBox2D Box;
		Box.Accumulate( PlaneBox.GetBoxCorners() );

		TArray<float2> BoxCorners;
		Box.GetBoxCorners( BoxCorners );
		Opengl::EnableAlpha( TRUE );
		glBegin(GL_QUADS);
		{
			glColor4f( 0.f, 1.f, 0.f, 0.5f );

			//	front
			glVertex3fv( BoxCorners[0].xyz( worldz ) );
			glVertex3fv( BoxCorners[1].xyz( worldz ) );
			glVertex3fv( BoxCorners[3].xyz( worldz ) );
			glVertex3fv( BoxCorners[2].xyz( worldz ) );
		}
		glEnd();
		*/
#endif



		EndScene();
	}
#endif
}


//-------------------------------------------------------------
//	get the render target's dimensions. we need the screen in case dimensions are max's
//-------------------------------------------------------------
void TLRender::TRenderTarget::GetSize(Type4<s32>& Size,const Type4<s32>& MaxSize) const
{
	//	init to size then correct the max-extents as neccesary
	Size = m_Size;

	if ( Size.x == g_MaxSize )			Size.x  = MaxSize.x;
	if ( Size.y == g_MaxSize )			Size.y  = MaxSize.y;
	if ( Size.Width() == g_MaxSize )	Size.Width()  = MaxSize.Width();
	if ( Size.Height() == g_MaxSize )	Size.Height()  = MaxSize.Height();
}


//-------------------------------------------------------------
//	do any recalculations we need to when the render target's size changes
//-------------------------------------------------------------
void TLRender::TRenderTarget::OnSizeChanged()
{
	//	setup view boxes of camera
	TPtr<TCamera>& pCamera = GetCamera();
	if ( pCamera )
	{
		//	todo: replace this screen pointer with proper one when screen owner gets stored on render target
		TPtr<TLRender::TScreen>& pScreen = TLRender::g_pScreenManager->GetDefaultScreen();
		if ( pScreen )
		{
			Type4<s32> RenderTargetMaxSize;
			pScreen->GetRenderTargetMaxSize( RenderTargetMaxSize );
			Type4<s32> RenderTargetSize;
			GetSize( RenderTargetSize, RenderTargetMaxSize );
			pCamera->SetRenderTargetSize( RenderTargetSize, RenderTargetMaxSize, pScreen->GetSize(), pScreen->GetScreenShape() );
		}
	}

}


//---------------------------------------------------------------
//	get world pos from 2d point inside our rendertarget size
//---------------------------------------------------------------
Bool TLRender::TRenderTarget::GetWorldRay(TLMaths::TLine& WorldRay,const Type2<s32>& RenderTargetPos,const Type4<s32>& RenderTargetSize,TScreenShape ScreenShape) const
{
	if ( !m_pCamera )
		return FALSE;

	return m_pCamera->GetWorldRay( WorldRay, RenderTargetPos, RenderTargetSize, ScreenShape );
}


//---------------------------------------------------------------
//	get world pos from 2d point inside our rendertarget size
//---------------------------------------------------------------
Bool TLRender::TRenderTarget::GetWorldPos(float3& WorldPos,float WorldDepth,const Type2<s32>& RenderTargetPos,const Type4<s32>& RenderTargetSize,TScreenShape ScreenShape) const
{
	if ( !m_pCamera )
		return FALSE;

	return m_pCamera->GetWorldPos( WorldPos, WorldDepth, RenderTargetPos, RenderTargetSize, ScreenShape );
}


// Get screen pos from 3d world pos
Bool TLRender::TRenderTarget::GetRenderTargetPos(Type2<s32>& RenderTargetPos, const float3& WorldPos,const Type4<s32>& RenderTargetSize,TScreenShape ScreenShape) const
{
	if ( !m_pCamera )
		return FALSE;

	return m_pCamera->GetRenderTargetPos( RenderTargetPos, WorldPos, RenderTargetSize, ScreenShape );
}



//---------------------------------------------------------------
//	gets the render node at the root
//---------------------------------------------------------------
void TLRender::TRenderTarget::SetRootRenderNode(const TPtr<TRenderNode>& pRenderNode)
{
	SetRootRenderNode( pRenderNode->GetNodeRef() );
}


//---------------------------------------------------------------
//	gets the render node at the root
//---------------------------------------------------------------
TPtr<TLRender::TRenderNode>& TLRender::TRenderTarget::GetRootRenderNode() const
{
	return TLRender::g_pRendergraph->FindNode( m_RootRenderNodeRef );
}


//---------------------------------------------------------------
//	check zone of node against camera's zone to determine visibility. 
//	if no scene transform is provided then we only do quick tests with no calculations. 
//	This can result in a SyncWait returned which means we need to do calculations to make sure of visibility
//---------------------------------------------------------------
SyncBool TLRender::TRenderTarget::IsRenderNodeVisible(TRenderNode& RenderNode,TPtr<TLMaths::TQuadTreeNode>*& ppRenderZoneNode,TLMaths::TQuadTreeNode* pCameraZoneNode,const TLMaths::TTransform* pSceneTransform,Bool& RenderNodeIsInsideCameraZone)
{
	//	no camera zone, so must be visible (assume no culling)
	if ( !pCameraZoneNode )
		return SyncTrue;

	Bool QuickTest = (pSceneTransform == NULL);
	SyncBool IsTransformUpToDate = RenderNode.IsWorldTransformValid();
	SyncBool IsZoneUpToDate = SyncWait;

	TLRender::TRenderZoneNode* pRenderZoneNode = NULL;

	//	find our render zone node
	if ( !ppRenderZoneNode )
	{
		ppRenderZoneNode = RenderNode.GetRenderZoneNode( GetRef() );

		//	if we have a zone, then set the up-to-date value
		if ( ppRenderZoneNode )
		{
			pRenderZoneNode = (*ppRenderZoneNode).GetObject<TLRender::TRenderZoneNode>();
			IsZoneUpToDate = pRenderZoneNode->IsZoneOutOfDate() ? SyncFalse : SyncTrue;
		}

#ifdef VISIBILITY_FRUSTUM_TEST_FIRST
		//	dont have a zone yet, the transform is NOT up to date, so cull test will fail anyway
		if ( !pRenderZoneNode && IsTransformUpToDate != SyncTrue )
			return SyncWait;
#else
		//	dont have a zone yet, if we're doing a quick test then abort and we'll create one later
		if ( !pRenderZoneNode && QuickTest )
			return SyncWait;
#endif // VISIBILITY_FRUSTUM_TEST_FIRST

		//	dont have a node for a zone yet, lets add one
		if ( !ppRenderZoneNode )
		{
			pRenderZoneNode = new TRenderZoneNode( RenderNode.GetNodeRef() );

			//	make new TPtr
			TPtr<TLMaths::TQuadTreeNode> pRenderZoneNodePtr = pRenderZoneNode;

			//	hold onto our new ZoneNode in our list
			ppRenderZoneNode = RenderNode.SetRenderZoneNode( GetRef(), pRenderZoneNodePtr );
			//	failed to add
			if ( !ppRenderZoneNode )
			{
				TLDebug_Break("Failed to add new render zone node to render node");
				return SyncFalse;
			}

			//	do has-shape test first
			if ( !pRenderZoneNode->HasZoneShape() )
			{
				if ( QuickTest )
					return SyncWait;
				else
					return SyncFalse;
			}

			//	add node to the zone tree
			if ( !m_pRootQuadTreeZone->AddNode( pRenderZoneNodePtr, m_pRootQuadTreeZone, TRUE ) )
			{
				//	node is outside of root zone...
				//	gr: assuming in quicktest the world scene transform of the render node is out of date so fails to be added to the quad tree...
				if ( QuickTest )
					return SyncWait;
				else
					return SyncFalse;
			}

			//	update up-to-date state
			IsZoneUpToDate = pRenderZoneNode->IsZoneOutOfDate() ? SyncFalse : SyncTrue;
		}
	}
	else
	{
		//	de-reference existing pointer
		pRenderZoneNode = (*ppRenderZoneNode).GetObject<TLRender::TRenderZoneNode>();
		IsZoneUpToDate = pRenderZoneNode->IsZoneOutOfDate() ? SyncFalse : SyncTrue;
	}

	//	quick frustum test first
#ifdef VISIBILITY_FRUSTUM_TEST_FIRST
	if ( IsTransformUpToDate == SyncTrue )
	{
		const TLMaths::TBox2D& FrustumBox = GetCamera()->GetZoneShape();
		if ( FrustumBox.IsValid() )
		{
			//	test bounds against frustum box
			SyncBool NodeInZoneShape = pRenderZoneNode->IsInShape( FrustumBox );

			//	if we got a valid intersection result then return the visibility
			if ( NodeInZoneShape != SyncWait )
			{
				return NodeInZoneShape;
			}
			else
			{
				//	get a syncwait if the scenetransform of the render node is out of date
				if ( !QuickTest )
				{
					TLDebug_Break("This test shouldnt fail unless we're in a quick test and the render node's scene transform is out of date");
				}
			}
		}
	}
#endif //VISIBILITY_FRUSTUM_TEST_FIRST


	//	zone needs updating
	if ( !IsZoneUpToDate )
	{
		if ( QuickTest )
			return SyncWait;

		//	update zone
		pRenderZoneNode->UpdateZone( *ppRenderZoneNode, m_pRootQuadTreeZone );
	}

	//	if the zone we are inside, is inside the camera zone, then render (this should be the most likely case)
	TLMaths::TQuadTreeZone* pRenderNodeZone = pRenderZoneNode->GetZone().GetObject();

	//	if zones are not visible to each other, not visible
	if ( !IsZoneVisible( pCameraZoneNode, pRenderNodeZone, pRenderZoneNode, RenderNodeIsInsideCameraZone ) )
		return SyncFalse;

#ifndef VISIBILITY_FRUSTUM_TEST_FIRST
	//	one final actual frustum culling test
	if ( !QuickTest )
	{
		SyncBool NodeInZoneShape = pRenderZoneNode->IsInShape( pCameraZoneNode->GetZoneShape() );
		if ( NodeInZoneShape == SyncWait )
		{
			TLDebug_Break("Check how to handle this...");
		}
		return NodeInZoneShape;
	}
#endif //!VISIBILITY_FRUSTUM_TEST_FIRST

	return SyncWait;
}

	
//---------------------------------------------------------------
//	
//---------------------------------------------------------------
Bool TLRender::TRenderTarget::IsZoneVisible(TLMaths::TQuadTreeNode* pCameraZoneNode,TLMaths::TQuadTreeZone* pZone,TLMaths::TQuadTreeNode* pZoneNode,Bool& RenderNodeIsInsideCameraZone)
{
	//	no camera node, no culling
	if ( !pCameraZoneNode )
		return TRUE;

	const TLMaths::TQuadTreeZone* pCameraZone = pCameraZoneNode->GetZone().GetObject();

	//	no render zone, assume node/camera is out outside of root zone
	if ( !pZone || !pCameraZone )
		return FALSE;

	//	our zone is directly under the camera's zone
	if ( pZone->GetParentZone().GetObject() == pCameraZone )
	{
		//	render node is below camera zone
		RenderNodeIsInsideCameraZone = TRUE;
		return TRUE;
	}
	
	//	our zone is under the camera's zone, but not directly, so it's possible our zone is NOT being intersected by the camera
	if ( pZone->IsBelowZone( pCameraZone ) )
	{
		/*
		//	loop through child zone's of camera zone
		const TPtrArray<TQuadTreeZone>&	CameraChildZones = pCameraZone->GetChildZones();
		for ( u32 c=0;	c<CameraChildZones.GetSize();	c++ )
		{
			//	are we under this
		}
		*/

		//	quick shape test...
		if ( !pCameraZoneNode->IsInZoneShape( *pZone ) )
			return FALSE;

		//	render node is below camera zone
		RenderNodeIsInsideCameraZone = TRUE;
		return TRUE;
	}
	
	//	if the camera's zone is inside this node's zone, then this node is bigger than the camera's visiblity and spans multiple zones
	if ( pCameraZone->IsBelowZone( pZone ) )
	{
		if ( pZoneNode )
		{
			//	the render node is intersecting specific zones, there's a case it's still off camera
			const TFixedArray<u32,4>& NodeZoneChildIndexes = pZoneNode->GetChildZones();

			if ( NodeZoneChildIndexes.GetSize() > 0 && NodeZoneChildIndexes.GetSize() < 4 )
			{
				Bool IsInsideChildZone = FALSE;
				for ( u32 c=0;	c<NodeZoneChildIndexes.GetSize();	c++ )
				{
					const TPtr<TLMaths::TQuadTreeZone>& pChildZone = pZone->GetChildZones().ElementAtConst( NodeZoneChildIndexes[c] );
					if ( pCameraZone->IsBelowZone( pChildZone ) )
					{
						IsInsideChildZone = TRUE;
						break;
					}
				}

				//	the zones the render node is intersecting doesnt include the one the camera is in. not visible!
				if ( !IsInsideChildZone )
					return FALSE;
			}
		}

		//	render node is above camera zone
		RenderNodeIsInsideCameraZone = FALSE;
		return TRUE;
	}

	return FALSE;
}



//---------------------------------------------------------------
//	render a render node
//---------------------------------------------------------------
Bool TLRender::TRenderTarget::DrawNode(TRenderNode* pRenderNode,TRenderNode* pParentRenderNode,const TLMaths::TTransform* pSceneTransform,TColour SceneColour,TLMaths::TQuadTreeNode* pCameraZoneNode)
{
	if ( !pRenderNode )
	{
		TLDebug_Break("Render node expected");
		return FALSE;
	}

	const TFlags<TRenderNode::RenderFlags::Flags>& RenderNodeRenderFlags = pRenderNode->GetRenderFlags();

	//	not enabled, dont render
	if ( !RenderNodeRenderFlags.IsSet( TLRender::TRenderNode::RenderFlags::Enabled ) )
		return FALSE;

	//	if node colour is reset then set a new scene colour
	if ( RenderNodeRenderFlags.IsSet(TLRender::TRenderNode::RenderFlags::ResetColour) )
	{
		SceneColour = pRenderNode->GetColour();
	}
	else
	{
		//	merge colour of scene
		if ( pRenderNode->IsColourValid() )
		{
			SceneColour *= pRenderNode->GetColour();

			//	alpha'd out (only applies if we're applying our colour, a la - IsColourValid)
			if ( SceneColour.GetAlphaf() < TLMaths_NearZero )
				return FALSE;
		}
	}

	//	check visibility of node, if not visible then skip render (and of children)
	TPtr<TLMaths::TQuadTreeNode>* ppRenderZoneNode = NULL;
	Bool RenderNodeIsInsideCameraZone = FALSE;
	SyncBool IsInCameraRenderZone = SyncWait;

	//	do cull test if enabled on node
	if ( pCameraZoneNode && RenderNodeRenderFlags.IsSet( TLRender::TRenderNode::RenderFlags::EnableCull ) )
	{
		//	pass in NULL as the scene transform to do a very quick zone test - skips calculating bounds etc
		IsInCameraRenderZone = IsRenderNodeVisible( *pRenderNode, ppRenderZoneNode, pCameraZoneNode, NULL, RenderNodeIsInsideCameraZone );

		//	after quick check we know the node is in a zone the camera cannot see
		if ( IsInCameraRenderZone == SyncFalse )
		{
			m_Debug_NodeCulledCount++;
			return FALSE;
		}
	}
	else
	{
		//	no cull testing, set as visible and mark as inside zone so we won't do any tests further down the tree
		IsInCameraRenderZone = SyncTrue;
		RenderNodeIsInsideCameraZone = TRUE;
	}

	//	do minimal calcuations to calc scene transformation - yes code is a bit of a pain, but this is very good for us :)
	//	only problem is, we can't reuse this code in another func as we lose the reference initialisation, which is the whole speed saving
	Bool ResetScene = RenderNodeRenderFlags.IsSet(TLRender::TRenderNode::RenderFlags::ResetScene);
	TLMaths::TTransform NewSceneTransform;
	const TLMaths::TTransform& NodeTransform = pRenderNode->GetTransform();
	Bool NodeTrans = NodeTransform.HasAnyTransform() || ResetScene;
	Bool SceneTrans = (pSceneTransform && !ResetScene) ? pSceneTransform->HasAnyTransform() : FALSE;
	
	//	work out which transform we are actually using... 
	const TLMaths::TTransform& SceneTransform = (NodeTrans&&SceneTrans) ? NewSceneTransform : ( (NodeTrans||!pSceneTransform) ? NodeTransform : *pSceneTransform );

	//	using a new scene transform, so calculate it
	if ( NodeTrans && SceneTrans )
	{
		NewSceneTransform = *pSceneTransform;
		NewSceneTransform.Transform( NodeTransform );
	}

	//	set latest world/scene transform
	pRenderNode->SetWorldTransform( SceneTransform );

	//	check visibility of node, if not visible then skip render (and of children)
	if ( IsInCameraRenderZone == SyncWait )
	{
		IsInCameraRenderZone = IsRenderNodeVisible( *pRenderNode, ppRenderZoneNode, pCameraZoneNode, &SceneTransform, RenderNodeIsInsideCameraZone );
		if ( IsInCameraRenderZone == SyncFalse )
		{
			m_Debug_NodeCulledCount++;
			return FALSE;
		}
	}

	//	start new scene if we change the transform of the current scene
	if ( NodeTrans )
	{
		if ( ResetScene )
		{
			BeginSceneReset();

#ifdef _DEBUG
			//	transform scene
			if ( &NodeTransform != &SceneTransform )
			{
				TLDebug_Break("Im pretty sure these should be the same - in which case use SceneTransform");
			}
#endif
		}
		else
		{
			BeginScene();
		}

		//	transform scene by node's transofrm
		Opengl::SceneTransform( NodeTransform );
	}

	//	count node as rendered
	m_Debug_NodeCount++;

	//	list of nodes to render afterwards, but not stored in the tree - debug etc
	TPtrArray<TRenderNode>& PostRenderList = m_TempPostRenderList;
	s32 FirstRenderNodeListIndex = PostRenderList.GetSize();

	//	do pre-render routine of the render object
	if ( pRenderNode->Draw( this, pParentRenderNode, PostRenderList ) )
	{
		//	get mesh
		TPtr<TLAsset::TMesh>& pMeshAsset = pRenderNode->GetMeshAsset();

		//	draw mesh!
		DrawMeshWrapper( pMeshAsset.GetObject(), pRenderNode, SceneColour, PostRenderList );
	}

	//	if this render node is in a zone under the camera's zone, then we KNOW the child nodes are going to be visible, so we dont need to provide the camera's
	//	render zone. Will just mean we can skip render zone checks
	//	gr: this isn't quite right atm, just check the flag for now
	//TLMaths::TQuadTreeNode* pChildCameraZoneNode = RenderNodeIsInsideCameraZone ? NULL : pCameraZoneNode;
	//TLMaths::TQuadTreeNode* pChildCameraZoneNode = pCameraZoneNode;
	TLMaths::TQuadTreeNode* pChildCameraZoneNode = pCameraZoneNode;

	//	check if we can skip culling for children
	if ( pChildCameraZoneNode )
	{
		//	if no culling, and children dont need to be forced to check culling, then remove camera zone for children (will skip visibility testing)
		if ( !RenderNodeRenderFlags.IsSet( TLRender::TRenderNode::RenderFlags::EnableCull ) && !RenderNodeRenderFlags.IsSet( TLRender::TRenderNode::RenderFlags::ForceCullTestChildren ) )
		{
			pChildCameraZoneNode = NULL;
		}
		else if ( RenderNodeIsInsideCameraZone )
		{
			//	our node is underneath/in same zone as the camera's zone, so we know it'll be in a visible zone
			//pChildCameraZoneNode = NULL;
		}
		else
		{
			//	our node isnt underneath the camera's zone -just visible- so we need to test again
		}
	}


	//	process children
	TPtrArray<TLRender::TRenderNode>& NodeChildren = pRenderNode->GetChildren();
	u32 ChildCount = NodeChildren.GetSize();
	if ( ChildCount > 0 )
	{
		TLMaths::TTransform ChildSceneTransform = SceneTransform;
		pRenderNode->PreDrawChildren( *this, ChildSceneTransform);

		//	render children
		for ( u32 c=0;	c<ChildCount;	c++ )
		{
			TPtr<TLRender::TRenderNode>& pChild = NodeChildren[c];

			//	draw child
			DrawNode( pChild, pRenderNode, &ChildSceneTransform, SceneColour, pChildCameraZoneNode );
		}

		pRenderNode->PostDrawChildren( *this );
	}

	//	draw our post-render nodes, deleting them as we go
	for ( s32 n=PostRenderList.GetLastIndex();	n>=FirstRenderNodeListIndex;	n-- )
	{
		//	gr: this MUST NOT be a reference. we need to make a new TPtr to keep the reference
		//		counting going. if DrawNode below empties the PostRenderList then this REFERENCE
		//		will be null'd
		//TPtr<TLRender::TRenderNode>& pChild = PostRenderList[c];
		TPtr<TLRender::TRenderNode> pChild = PostRenderList[n];

		//	draw child
		DrawNode( pChild.GetObject(), pRenderNode, &SceneTransform, SceneColour, pChildCameraZoneNode );

		//	remove from list
		PostRenderList.RemoveAt(n);
	}
	

	//	clean up/restore scene
	if ( NodeTrans )
	{
		EndScene();
	}

	return TRUE;
}


//-------------------------------------------------------------
//	
//-------------------------------------------------------------
void TLRender::TRenderTarget::DrawMeshWrapper(const TLAsset::TMesh* pMesh,TRenderNode* pRenderNode, TColour SceneColour,TPtrArray<TRenderNode>& PostRenderList)
{
#ifdef _DEBUG
	TFlags<TRenderNode::RenderFlags::Flags> RenderNodeRenderFlags = pRenderNode->GetRenderFlags();

	//	modify render object flags for debug stuff
	RenderNodeRenderFlags.Set( Debug_ForceRenderFlagsOn() );
	RenderNodeRenderFlags.Clear( Debug_ForceRenderFlagsOff() );

	//	render everything's bound box
	//RenderNodeRenderFlags.Set( TRenderNode::RenderFlags::Debug_WorldBoundsBox );

#else
	TFlags<TRenderNode::RenderFlags::Flags>& RenderNodeRenderFlags = pRenderNode->GetRenderFlags();
#endif

	//	mesh renders
	if ( pMesh && !pMesh->IsEmpty() )
	{
		//	do first non-wireframe render
		#ifdef _DEBUG
		if ( !RenderNodeRenderFlags.IsSet( TRenderNode::RenderFlags::Debug_Wireframe ) )
		#endif
		{
			Bool HasAlpha = pMesh->HasAlpha();

			//	grab texture as required
			TLAsset::TTexture* pTexture = NULL;
			//if ( pRenderNode->HasTexture() )
			if ( pRenderNode->GetTextureRef().IsValid() )
			{
				pTexture = pRenderNode->GetTextureAsset();

				//	missing texture - try and use debug one
				if ( !pTexture )
					pTexture = TLAsset::GetAsset<TLAsset::TTexture>("d_texture");
			}

			//	enable alpha if texture is alpha'd
			if ( pTexture )
				HasAlpha |= pTexture->HasAlphaChannel();

			//	get add blend mode
			Bool AddBlending = RenderNodeRenderFlags.IsSet( TRenderNode::RenderFlags::AddBlending );
			HasAlpha |= AddBlending;

			//	make sure wireframe is off
			Opengl::EnableWireframe(FALSE);

			//	set the scene colour, and force enable alpha if the mesh needs it
			Opengl::SetSceneColour( SceneColour, HasAlpha, AddBlending );

			DrawMesh( *pMesh, pTexture, pRenderNode, RenderNodeRenderFlags, HasAlpha );
		}

		//	render wireframe and/or outline
		#ifdef _DEBUG
		{
			if ( RenderNodeRenderFlags.IsSet( TRenderNode::RenderFlags::Debug_Outline ) || RenderNodeRenderFlags.IsSet( TRenderNode::RenderFlags::Debug_Wireframe ) )
			{
				//	setup specific params
				Opengl::EnableWireframe(TRUE);
				
				if ( !RenderNodeRenderFlags.IsSet( TRenderNode::RenderFlags::Debug_ColourWireframe ) )
					Opengl::SetSceneColour( TColour( 1.f, 1.f, 1.f, 1.f ) );

				Opengl::SetLineWidth( 1.f );

				TFlags<TRenderNode::RenderFlags::Flags> WireframeRenderFlags = RenderNodeRenderFlags;
				WireframeRenderFlags.Clear( TRenderNode::RenderFlags::UseVertexColours );
				WireframeRenderFlags.Clear( TRenderNode::RenderFlags::UseMeshLineWidth );
				WireframeRenderFlags.Clear( TRenderNode::RenderFlags::DepthRead );

				DrawMesh( *pMesh, NULL, pRenderNode, WireframeRenderFlags, FALSE );
			}
		}
		#endif
	}


	//	non-mesh debug renders
	#ifdef _DEBUG
	{
		//	render a cross at 0,0,0 (center of node)
		if ( RenderNodeRenderFlags.IsSet( TRenderNode::RenderFlags::Debug_Position ) )
		{
			//	setup specific params
			Opengl::EnableWireframe( FALSE );
			Opengl::SetLineWidth( 2.f );
			Opengl::EnableAlpha( FALSE );

			//	get the debug cross
			TLAsset::TMesh* pMesh = TLAsset::GetAsset<TLAsset::TMesh>("d_cross");
			if ( pMesh )
				DrawMesh( *pMesh, NULL, pRenderNode, RenderNodeRenderFlags, FALSE );
		}


		//	get a list of datums to debug-render
#ifdef DEBUG_RENDER_DATUMS_IN_WORLD
		TPtrArray<TLMaths::TShape> RenderDatums;
#else
		TFixedArray<const TLMaths::TShape*,100> RenderDatums;
#endif

		if ( RenderNodeRenderFlags.IsSet( TRenderNode::RenderFlags::Debug_Datums ) )
		{
#ifdef DEBUG_RENDER_DATUMS_IN_WORLD
			//	todo :)
			//pRenderNode->GetLocalDatums( RenderDatums );
#else
			pRenderNode->GetLocalDatums( RenderDatums );
#endif
		}
		else
		{
			for ( u32 i=0;	i<pRenderNode->Debug_GetDebugRenderDatums().GetSize();	i++ )
			{
			#ifdef DEBUG_RENDER_DATUMS_IN_WORLD
				TPtr<TLMaths::TShape> pDatum = pRenderNode->GetWorldDatum( pRenderNode->Debug_GetDebugRenderDatums()[i] );
			#else
				const TLMaths::TShape* pDatum = pRenderNode->GetLocalDatum( pRenderNode->Debug_GetDebugRenderDatums()[i] );
			#endif
				RenderDatums.Add( pDatum );
			}

			//	add flagged datums
		#ifdef DEBUG_RENDER_DATUMS_IN_WORLD
			if ( RenderNodeRenderFlags.IsSet( TRenderNode::RenderFlags::Debug_LocalBoundsBox ) )
				RenderDatums.Add( pRenderNode->GetWorldDatum( TLRender_TRenderNode_DatumBoundsBox ) );

			if ( RenderNodeRenderFlags.IsSet( TRenderNode::RenderFlags::Debug_LocalBoundsSphere ) )
				RenderDatums.Add( pRenderNode->GetWorldDatum( TLRender_TRenderNode_DatumBoundsSphere ) );
		#else
			if ( RenderNodeRenderFlags.IsSet( TRenderNode::RenderFlags::Debug_LocalBoundsBox ) )
				RenderDatums.Add( pRenderNode->GetLocalDatum( TLRender_TRenderNode_DatumBoundsBox ) );

			if ( RenderNodeRenderFlags.IsSet( TRenderNode::RenderFlags::Debug_LocalBoundsSphere ) )
				RenderDatums.Add( pRenderNode->GetLocalDatum( TLRender_TRenderNode_DatumBoundsSphere ) );
		#endif
		}

		//	setup scene if we have some datums to render
		if ( RenderDatums.GetSize())
		{
			TFlags<TRenderNode::RenderFlags::Flags> DebugDatumRenderFlags = pRenderNode->GetRenderFlags();
			DebugDatumRenderFlags.Set( TRenderNode::RenderFlags::Debug_Wireframe );
			DebugDatumRenderFlags.Set( TRenderNode::RenderFlags::Debug_Outline );
			DebugDatumRenderFlags.Clear( TRenderNode::RenderFlags::DepthRead );
		
			Opengl::EnableWireframe(TRUE);
			Opengl::SetSceneColour( TColour( 1.f, 1.f, 1.f, 1.f ) );
			Opengl::SetLineWidth( 1.f );

			for ( u32 i=0;	i<RenderDatums.GetSize();	i++ )
			{
				const TLMaths::TShape* pDatum = RenderDatums[i];
				if ( pDatum )
				{
#ifdef DEBUG_RENDER_DATUMS_IN_WORLD
					DrawMeshShape( *pDatum, pRenderNode, DebugDatumRenderFlags, TRUE );
#else
					DrawMeshShape( *pDatum, pRenderNode, DebugDatumRenderFlags, FALSE );
#endif
				}
			}
		}
	}
	#endif
}


//-------------------------------------------------------------
//	render mesh asset
//-------------------------------------------------------------
void TLRender::TRenderTarget::DrawMesh(const TLAsset::TMesh& Mesh,const TLAsset::TTexture* pTexture,const TRenderNode* pRenderNode,const TFlags<TRenderNode::RenderFlags::Flags>& RenderFlags,Bool HasAlpha)
{
	const TArray<float3>& Vertexes = Mesh.GetVertexes();

	//	bind vertex data
	Opengl::BindVertexes( &Vertexes );

	//	ignore UV vertex data if flag is not set or if we have no texture
	if ( pTexture && RenderFlags( TRenderNode::RenderFlags::UseVertexUVs ) )
	{
		Opengl::BindUVs( Mesh.GetUVsNotEmpty() );
	}
	else
	{
		Opengl::BindUVs( NULL );
	}


	//	ignore colour vertex data if flag is not set
	if ( RenderFlags( TRenderNode::RenderFlags::UseVertexColours ) )
	{
		enum TColourMode
		{
			Colour_TColour,
			Colour_TColour24,
			Colour_TColour32,
			Colour_TColour64
		};

		//	pick most-desired mode
		TColourMode ColourMode = HasAlpha ? Colour_TColour32 : Colour_TColour24;
		if ( RenderFlags( TRenderNode::RenderFlags::UseFloatColours ) )
			ColourMode = Colour_TColour;

		//	force colour modes
		#ifdef FORCE_COLOUR
			ColourMode = FORCE_COLOUR;
		#endif
		
		//	bind to colours
		if ( ColourMode == Colour_TColour24 )
		{
			const TArray<TColour24>* pColours24 = Mesh.GetColours24NotEmpty();
			if ( pColours24 )
				Opengl::BindColours( pColours24 );
			else
				ColourMode = Colour_TColour;
		}
		else if ( ColourMode == Colour_TColour32 )
		{
			const TArray<TColour32>* pColours32 = Mesh.GetColours32NotEmpty();
			if ( pColours32 )
				Opengl::BindColours( pColours32 );
			else
				ColourMode = Colour_TColour;
		}
		else if ( ColourMode == Colour_TColour64 )
		{
			const TArray<TColour64>* pColours64 = Mesh.GetColours64NotEmpty();
			if ( pColours64 )
				Opengl::BindColours( pColours64 );
			else
				ColourMode = Colour_TColour;
		}

		//	do float mode last in case we had to resort to it from lack of colours
		if ( ColourMode == Colour_TColour )
		{
			//	gr: always have float colours (if any at all)
			Opengl::BindColours( Mesh.GetColoursNotEmpty() );
		}
	}
	else
	{
		Opengl::BindColoursNull();
	}


	//	bind texture
	Opengl::BindTexture( pTexture );

	//	enable/disable depth test
	Opengl::EnableDepthRead( RenderFlags( TRenderNode::RenderFlags::DepthRead ) );

	//	enable/disable depth writing
	Opengl::EnableDepthWrite( RenderFlags( TRenderNode::RenderFlags::DepthWrite ) );


	const TArray<TLAsset::TMesh::Triangle>* pTriangles	= Mesh.GetTrianglesNotEmpty();
	const TArray<TLAsset::TMesh::Tristrip>* pTristrips	= Mesh.GetTristripsNotEmpty();
	const TArray<TLAsset::TMesh::Trifan>* pTrifans			= Mesh.GetTrifansNotEmpty();
	const TArray<TLAsset::TMesh::Line>* pLines				= Mesh.GetLinesNotEmpty();
	const TArray<TLAsset::TMesh::Linestrip>* pLinestrips	= Mesh.GetLinestripsNotEmpty();


	//	setup line width if required
	if ( (pLines || pLinestrips) && RenderFlags( TRenderNode::RenderFlags::UseMeshLineWidth ) ) 
	{
		float LineWidth = pRenderNode->GetLineWidth();
		if ( LineWidth < 1.f )
		{
			//	convert world-space line width to pixel size
			const float3& NodePos = pRenderNode->GetWorldPosConst();
			LineWidth = GetCamera()->GetScreenSizeFromWorldSize( Mesh.GetLineWidth(), NodePos.z );

			//	min width
			if ( LineWidth < 1.f )
				LineWidth = 1.f;
		}
		
		Opengl::SetLineWidth( LineWidth );
	}
		
	//	draw primitives
	if ( pTriangles )
		Opengl::DrawPrimitives( Opengl::Platform::GetPrimTypeTriangle(), *pTriangles );
	if ( pTristrips )
		Opengl::DrawPrimitives( Opengl::Platform::GetPrimTypeTristrip(), *pTristrips );
	if ( pTrifans )
		Opengl::DrawPrimitives( Opengl::Platform::GetPrimTypeTrifan(), *pTrifans );
	if ( pLinestrips )
		Opengl::DrawPrimitives( Opengl::Platform::GetPrimTypeLinestrip(), *pLinestrips );
	if ( pLines )
		Opengl::DrawPrimitives( Opengl::Platform::GetPrimTypeLine(), *pLines );
	
	//	draw points like a primitive
	if ( RenderFlags( TRenderNode::RenderFlags::Debug_Points ) )
	{
		Opengl::SetPointSize( 8.f );
		Opengl::EnablePointSprites(FALSE);
		Opengl::DrawPrimitivePoints( &Vertexes );
	}

	//	draw points as point sprites
	if ( RenderFlags( TRenderNode::RenderFlags::UsePointSprites ) )
	{
		//	convert world-space point size to pixel size
		const float3& NodePos = pRenderNode->GetWorldPosConst();
		float PointSize = GetCamera()->GetScreenSizeFromWorldSize( pRenderNode->GetPointSpriteSize(), NodePos.z );

/*
		static PFNGLPOINTPARAMETERFARBPROC  glPointParameterfARB  = NULL;
		static PFNGLPOINTPARAMETERFVARBPROC glPointParameterfvARB = NULL;
		if ( !glPointParameterfARB || !glPointParameterfvARB )
		{
			glPointParameterfARB  = (PFNGLPOINTPARAMETERFARBPROC)wglGetProcAddress("glPointParameterfARB");
			glPointParameterfvARB = (PFNGLPOINTPARAMETERFVARBPROC)wglGetProcAddress("glPointParameterfvARB");
		}
		
		float quadratic[] =  { 1.0f, 0.0f, 0.01f };
		glPointParameterfvARB( GL_POINT_DISTANCE_ATTENUATION_ARB, quadratic );

		// Query for the max point size supported by the hardware
		float maxSize = 0.0f;
		glGetFloatv( GL_POINT_SIZE_MAX_ARB, &maxSize );
		glPointParameterfARB( GL_POINT_FADE_THRESHOLD_SIZE_ARB, 60.0f );

		glPointParameterfARB( GL_POINT_SIZE_MIN_ARB, 1.f );
		glPointParameterfARB( GL_POINT_SIZE_MAX_ARB, maxSize );
		*/

		//	enable point sprites
		Opengl::EnablePointSprites(TRUE);

		//	set point-size UV mapping (otherwise will just be uv's of 0,0)
		//	gr: only change this if we have a texture - saves a state change
		if ( pTexture )
			Opengl::EnablePointSizeUVMapping(TRUE); 

		//	clamp size
		Opengl::ClampPointSpriteSize( PointSize );
		Opengl::SetPointSize( PointSize );

		//	draw points
		Opengl::DrawPrimitivePoints( &Vertexes );

		//	undo texture env changes
		if ( pTexture )
			Opengl::EnablePointSizeUVMapping(FALSE); 
		
	}

}



void TLRender::TRenderTarget::DrawMeshShape(const TLMaths::TShape& Shape,const TLRender::TRenderNode* pRenderNode,const TFlags<TLRender::TRenderNode::RenderFlags::Flags>& RenderFlags,Bool ResetScene)
{
	if ( !Shape.IsValid() )
		return;

	//	save off current render state
	if ( ResetScene )
		BeginSceneReset();
	else
		BeginScene();
		
	//	possible a little expensive... generate a mesh for the bounds...
	TLAsset::TMesh ShapeMesh("Bounds");
	ShapeMesh.GenerateShape( Shape );

	//	then render our temporary mesh
	DrawMesh( ShapeMesh, NULL, pRenderNode, RenderFlags, FALSE );

	//	when using a temporary mesh, make sure all vertex caches are unbound/uncached
	Opengl::Unbind();

	EndScene();
}

//-------------------------------------------------------------
//	
//-------------------------------------------------------------
void TLRender::TRenderTarget::SetRootQuadTreeZone(TPtr<TLMaths::TQuadTreeZone>& pQuadTreeZone)
{
	if ( m_pRootQuadTreeZone == pQuadTreeZone )
		return;

	//	clean up the old one
	if ( m_pRootQuadTreeZone )
	{
		m_pRootQuadTreeZone->Shutdown();
		m_pRootQuadTreeZone = NULL;
	}

	//	create new root zone
	m_pRootQuadTreeZone = pQuadTreeZone;

	if ( m_pRootQuadTreeZone )
	{
#ifdef PREDIVIDE_RENDER_ZONES
		//	divide it all now
		m_pRootQuadTreeZone->DivideAll( m_pRootQuadTreeZone );
#endif
	}

	//	invalidate camera's zone
	m_pCamera->SetZoneOutOfDate();
}

	
//--------------------------------------------------
//	
//--------------------------------------------------
void TLRender::TRenderTarget::Debug_DrawZone(TPtr<TLMaths::TQuadTreeZone>& pZone,float zDepth,TLMaths::TQuadTreeNode* pCameraZoneNode)
{
	TRenderNode TempRenderNode("xxx");
	TempRenderNode.SetMeshRef("d_quad");

	if ( pCameraZoneNode )
	{
		Bool Dummy;
		if ( !IsZoneVisible( pCameraZoneNode, pZone.GetObject(), NULL, Dummy ) )
			return;
	}

	TempRenderNode.SetAlpha( 0.3f );
	if ( pCameraZoneNode )
	{
		if ( pZone.GetObject() == pCameraZoneNode->GetZone().GetObject() )
		{
			TempRenderNode.GetRenderFlags().Clear( TLRender::TRenderNode::RenderFlags::UseVertexColours );
			TempRenderNode.GetRenderFlags().Clear( TLRender::TRenderNode::RenderFlags::DepthRead );
			TempRenderNode.SetAlpha( 1.f );
		}
	}

	const TLMaths::TBox2D& ZoneBox = pZone->GetShape();
	TempRenderNode.SetTranslate( ZoneBox.GetMin().xyz(zDepth) );
	TempRenderNode.SetScale( ZoneBox.GetSize().xyz(1.f) );

	DrawNode( &TempRenderNode, NULL, NULL, TColour(1.f,1.f,1.f,1.f), NULL );

	for ( u32 z=0;	z<pZone->GetChildZones().GetSize();	z++ )
	{
		TPtr<TLMaths::TQuadTreeZone>& pChildZone = pZone->GetChildZones().ElementAt(z);
		Debug_DrawZone( pChildZone, zDepth + 0.01f, pCameraZoneNode );
	}
	
}

//--------------------------------------------------
//	same as GetWorldViewBox but can be used before a render
//--------------------------------------------------
const TLMaths::TBox2D& TLRender::TRenderTarget::GetWorldViewBox(TPtr<TScreen>& pScreen,float WorldDepth)
{
	/*
	//	get render target size
	Type4<s32> Size;
	Type4<s32> MaxSize;
	pScreen->GetRenderTargetMaxSize( MaxSize );
	GetSize( Size, MaxSize );

	//	calc viewport sizes and boxes etc will be valid
	m_pCamera->SetRenderTargetSize( Size, pScreen->GetScreenShape() );
	*/

	return GetWorldViewBox( WorldDepth );
}

//--------------------------------------------------
//	
//--------------------------------------------------
void TLRender::TRenderTarget::SetClearColour(const TColour& Colour)
{
	//	update colour of clear node
	if ( m_pRenderNodeClear )
	{
		m_pRenderNodeClear->SetColour( Colour );
	}
	
	//	set new clear colour
	m_ClearColour = Colour;
}



//--------------------------------------------------
//	
//--------------------------------------------------
void TLRender::TRenderTarget::SetScreenZ(u8 NewZ)
{
	if ( m_ScreenZ != NewZ )
	{
		m_ScreenZ = NewZ;
		TLRender::g_pScreenManager->GetDefaultScreen()->OnRenderTargetZChanged( *this );
	}
}


