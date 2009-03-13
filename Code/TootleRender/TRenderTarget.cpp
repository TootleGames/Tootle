#include "TRenderTarget.h"
#include "TScreen.h"
#include <TootleCore/TPtr.h>
#include "TScreenManager.h"



#if defined(_DEBUG) && !defined(TL_TARGET_IPOD)
//#define DEBUG_DRAW_RENDERZONES
#define DEBUG_DRAW_FRUSTUM
//#define DEBUG_NODE_RENDERED_COUNT
#endif


//#define PREDIVIDE_RENDER_ZONES


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
	m_Debug_NodeCulledCount	( 0 )
{
	//	set default flags
	m_Flags.Set( Flag_Enabled );
	m_Flags.Set( Flag_ClearColour );
	m_Flags.Set( Flag_ClearDepth );
	m_Flags.Set( Flag_AntiAlias );

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

	//	get the size... fails if it's too small to be of any use
	Type4<s32> RenderTargetSize;
	GetSize( RenderTargetSize, RenderTargetMaxSize );
	if ( RenderTargetSize.Width() <= 2 || RenderTargetSize.Height() <= 2 )
		return FALSE;

	//	if we have no root render node, and not going to be clearing the screen, skip render
	Bool WillClear = GetFlags()( Flag_ClearColour ) && (m_ClearColour.GetAlpha() > 0.f);
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
	Type4<s32> ViewportSize;
	Opengl::GetViewportSize( ViewportSize, ViewportMaxSize, RenderTargetSize, RenderTargetMaxSize, Screen.GetScreenShape() );
	glViewport( ViewportSize.Left(), ViewportSize.Top(), ViewportSize.Width(), ViewportSize.Height() );
	Opengl::Debug_CheckForError();

	//	todo: only enable if size is not full screen - maybe cache current scene viewport?
	Opengl::EnableScissor( TRUE );
	glScissor( ViewportSize.Left(), ViewportSize.Top(), ViewportSize.Width(), ViewportSize.Height() );
	Opengl::Debug_CheckForError();

	//	calculate new view sizes etc for this viewport
	TPtr<TLRender::TCamera>& pCamera = GetCamera();
	pCamera->SetViewportSize( ViewportSize, Screen.GetScreenShape() );
	pCamera->SetRenderTargetSize( RenderTargetSize, Screen.GetScreenShape() );

	//	do projection vs orthographic setup
	if ( GetCamera()->IsOrtho() )
	{
		if ( !BeginOrthoDraw( pCamera.GetObject<TLRender::TOrthoCamera>(), Screen.GetScreenShape() ) )
			return FALSE;
	}
	else
	{
		if ( !BeginProjectDraw( pCamera.GetObject<TLRender::TProjectCamera>(), Screen.GetScreenShape() ) )
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
	if ( m_pRenderNodeClear )
	{
		DrawNode( m_pRenderNodeClear.GetObject(), NULL, NULL, TColour( 1.f, 1.f, 1.f, 1.f ), NULL );
	}

	//	update the camera's zone
	Bool CameraInWorldZone = TRUE;
	if ( m_pRootQuadTreeZone )
	{
		if ( m_pCamera->IsZoneOutOfDate() )
			m_pCamera->UpdateZone( m_pCamera, m_pRootQuadTreeZone );

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

		glBegin(GL_LINES);
		{	
			/*	
			Opengl::SetSceneColour( TColour( 1.f, 0.f, 0.f, 1.f ) );

			//	front
			glVertex3fv( pCamera->m_Frustum.GetBox().GetBoxCorners().ElementAt(0) );
			glVertex3fv( pCamera->m_Frustum.GetBox().GetBoxCorners().ElementAt(1) );

			glVertex3fv( pCamera->m_Frustum.GetBox().GetBoxCorners().ElementAt(1) );
			glVertex3fv( pCamera->m_Frustum.GetBox().GetBoxCorners().ElementAt(2) );

			glVertex3fv( pCamera->m_Frustum.GetBox().GetBoxCorners().ElementAt(2) );
			glVertex3fv( pCamera->m_Frustum.GetBox().GetBoxCorners().ElementAt(3) );

			glVertex3fv( pCamera->m_Frustum.GetBox().GetBoxCorners().ElementAt(3) );
			glVertex3fv( pCamera->m_Frustum.GetBox().GetBoxCorners().ElementAt(0) );
		*/
			Opengl::SetSceneColour( TColour( 0.f, 1.f, 0.f, 1.f ) );

			//	rear
			glVertex3fv( pCamera->m_Frustum.GetBox().GetBoxCorners().ElementAt(4) );
			glVertex3fv( pCamera->m_Frustum.GetBox().GetBoxCorners().ElementAt(5) );

			glVertex3fv( pCamera->m_Frustum.GetBox().GetBoxCorners().ElementAt(5) );
			glVertex3fv( pCamera->m_Frustum.GetBox().GetBoxCorners().ElementAt(6) );

			glVertex3fv( pCamera->m_Frustum.GetBox().GetBoxCorners().ElementAt(6) );
			glVertex3fv( pCamera->m_Frustum.GetBox().GetBoxCorners().ElementAt(7) );

			glVertex3fv( pCamera->m_Frustum.GetBox().GetBoxCorners().ElementAt(7) );
			glVertex3fv( pCamera->m_Frustum.GetBox().GetBoxCorners().ElementAt(4) );

			/*
			Opengl::SetSceneColour( TColour( 1.f, 1.f, 0.f, 1.f ) );

			//	joins
			glVertex3fv( pCamera->m_Frustum.GetBox().GetBoxCorners().ElementAt(0) );
			glVertex3fv( pCamera->m_Frustum.GetBox().GetBoxCorners().ElementAt(4) );

			glVertex3fv( pCamera->m_Frustum.GetBox().GetBoxCorners().ElementAt(1) );
			glVertex3fv( pCamera->m_Frustum.GetBox().GetBoxCorners().ElementAt(5) );

			glVertex3fv( pCamera->m_Frustum.GetBox().GetBoxCorners().ElementAt(2) );
			glVertex3fv( pCamera->m_Frustum.GetBox().GetBoxCorners().ElementAt(6) );

			glVertex3fv( pCamera->m_Frustum.GetBox().GetBoxCorners().ElementAt(3) );
			glVertex3fv( pCamera->m_Frustum.GetBox().GetBoxCorners().ElementAt(7) );
		*/
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
	//	initialise the ortho camera's box (useful for when we want to use the box straight after initialisation but before we do a render)
	TPtr<TCamera>& pCamera = GetCamera();
	if ( pCamera )
	{
		//	calc render target size
		//	todo: replace this screen pointer with proper one when screen owner gets stored on render target
		TPtr<TLRender::TScreen>& pScreen = TLRender::g_pScreenManager->GetDefaultScreen();
		if ( pScreen )
		{
			Type4<s32> RenderTargetMaxSize;
			pScreen->GetRenderTargetMaxSize( RenderTargetMaxSize );
			Type4<s32> RenderTargetSize;
			GetSize( RenderTargetSize, RenderTargetMaxSize );
			pCamera->SetRenderTargetSize( RenderTargetSize, pScreen->GetScreenShape() );
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
SyncBool TLRender::TRenderTarget::IsRenderNodeVisible(TRenderNode* pRenderNode,TPtr<TLMaths::TQuadTreeNode>*& ppRenderZoneNode,TLMaths::TQuadTreeNode* pCameraZoneNode,const TLMaths::TTransform* pSceneTransform,Bool& RenderNodeIsInsideCameraZone)
{
	//	no camera zone, so must be visible (assume no culling)
	if ( !pCameraZoneNode )
		return SyncTrue;

	Bool QuickTest = (pSceneTransform == NULL);

	//	find our render zone node
	if ( !ppRenderZoneNode )
	{
		ppRenderZoneNode = pRenderNode->GetRenderZoneNode( GetRef() );

		//	dont have a zone yet, if we're doing a quick test then abort and we'll create one later
		if ( !ppRenderZoneNode && QuickTest )
			return SyncWait;

		//	dont have a node for a zone yet, lets add one
		if ( !ppRenderZoneNode )
		{
			TRenderZoneNode* pRenderZoneNode = new TRenderZoneNode( pRenderNode->GetNodeRef() );
			TPtr<TLMaths::TQuadTreeNode> pRenderZoneNodePtr = pRenderZoneNode;
	
			//	add node to the zone tree
			if ( !m_pRootQuadTreeZone->AddNode( pRenderZoneNodePtr, m_pRootQuadTreeZone, TRUE ) )
			{
				//	node is outside of root zone...
				return SyncFalse;
			}

			//	debug the added result
			/*
#ifdef _DEBUG
			//	count number of zones down from the root we are
			u32 Count=0;
			TPtr<TLMaths::TQuadTreeZone> pZone = pRenderZoneNode->GetZone()->GetParentZone();
			while ( pZone )
			{
				pZone = pZone->GetParentZone();
				Count++;				
			}

			TTempString DebugString("Node ");
			pRenderNode->GetNodeRef().GetString( DebugString );
			DebugString.Appendf(" added to render zone %d down from root",Count);
			TLDebug_Print( DebugString );
#endif
		*/

			//	hold onto our new ZoneNode in our list
			ppRenderZoneNode = pRenderNode->SetRenderZoneNode( GetRef(), pRenderZoneNodePtr );

			//	failed to add
			if ( !ppRenderZoneNode )
			{
				TLDebug_Break("Failed to add new render zone node to render node");
				return SyncFalse;
			}
		}
	}


	TPtr<TLMaths::TQuadTreeNode>& pRenderZoneNodePtr = (*ppRenderZoneNode);
	TLRender::TRenderZoneNode* pRenderZoneNode = pRenderZoneNodePtr.GetObject<TLRender::TRenderZoneNode>();

	//	zone needs updating
	if ( pRenderZoneNode->IsZoneOutOfDate() )
	{
		if ( QuickTest )
			return SyncWait;

		//	update zone
		pRenderZoneNode->UpdateZone( *ppRenderZoneNode, m_pRootQuadTreeZone );

/*
	#ifdef _DEBUG
		//	count number of zones down from the root we are
		s32 Count=-1;
		TPtr<TLMaths::TQuadTreeZone> pZone = pRenderZoneNode->GetZone();
		while ( pZone )
		{
			pZone = pZone->GetParentZone();
			Count++;				
		}

		TTempString DebugString("Node ");
		pRenderNode->GetNodeRef().GetString( DebugString );
		DebugString.Appendf(" in render zone %d down from root",Count);
		TLDebug_Print( DebugString );
#endif
		*/
	}

	//	if the zone we are inside, is inside the camera zone, then render (this should be the most likely case)
	TLMaths::TQuadTreeZone* pRenderNodeZone = pRenderZoneNode->GetZone().GetObject();

	//	if zones are not visible to each other, not visible
	if ( !IsZoneVisible( pCameraZoneNode, pRenderNodeZone, pRenderZoneNode, RenderNodeIsInsideCameraZone ) )
		return SyncFalse;

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
		if ( !pCameraZoneNode->IsInZone( *pZone ) )
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

	Bool ResetScene = RenderNodeRenderFlags.IsSet(TLRender::TRenderNode::RenderFlags::ResetScene);
	if ( ResetScene )
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
			if ( SceneColour.GetAlpha() < TLMaths::g_NearZero )
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
		IsInCameraRenderZone = IsRenderNodeVisible( pRenderNode, ppRenderZoneNode, pCameraZoneNode, NULL, RenderNodeIsInsideCameraZone );

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
		IsInCameraRenderZone = IsRenderNodeVisible( pRenderNode, ppRenderZoneNode, pCameraZoneNode, &SceneTransform, RenderNodeIsInsideCameraZone );
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

			//	transform scene
			if ( &NodeTransform != &SceneTransform )
			{
				TLDebug_Break("Im pretty sure these should be the same - in which case use SceneTransform");
			}
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
		DrawMeshWrapper( pMeshAsset.GetObject(), pRenderNode, SceneTransform, SceneColour, PostRenderList );
	}

	//	if this render node is in a zone under the camera's zone, then we KNOW the child nodes are going to be visible, so we dont need to provide the camera's
	//	render zone. Will just mean we can skip render zone checks
	//	gr: this isn't quite right atm, just check the flag for now
	//TLMaths::TQuadTreeNode* pChildCameraZoneNode = RenderNodeIsInsideCameraZone ? NULL : pCameraZoneNode;
	//TLMaths::TQuadTreeNode* pChildCameraZoneNode = pCameraZoneNode;
	TLMaths::TQuadTreeNode* pChildCameraZoneNode = pCameraZoneNode;
	if ( !RenderNodeRenderFlags.IsSet( TLRender::TRenderNode::RenderFlags::EnableCull ) && !RenderNodeRenderFlags.IsSet( TLRender::TRenderNode::RenderFlags::ForceCullTestChildren ) )
		pChildCameraZoneNode = NULL;

	//	render children
#ifdef TLGRAPH_OWN_CHILDREN
	TPtrArray<TLRender::TRenderNode>& NodeChildren = pRenderNode->GetChildren();
	for ( u32 c=0;	c<NodeChildren.GetSize();	c++ )
	{
		TPtr<TLRender::TRenderNode>& pChild = NodeChildren[c];
#else
	TPtr<TRenderNode> pChild = pRenderNode->GetChildFirst();
	while ( pChild )
	{
#endif
		//	draw child
		DrawNode( pChild.GetObject(), pRenderNode, &SceneTransform, SceneColour, pChildCameraZoneNode );

		#ifndef TLGRAPH_OWN_CHILDREN
		pChild = pChild->GetNext();
		#endif
	}

	//	draw our post-render nodes, deleting them as we go
	for ( s32 n=PostRenderList.GetLastIndex();	n>=FirstRenderNodeListIndex;	n-- )
	{
		//	gr: this MUST NOT be a reference. we need to make a new TPtr to keep the reference
		//		counting going. if DrawNode below empties the PostRenderList then this REFERENCE
		//		will be null'd
		//TPtr<TLRender::TRenderNode>& pChild = PostRenderList[c];
		TPtr<TLRender::TRenderNode> pChild = PostRenderList[n];

		//	gr: redundant? dont need a temp... this transform wont be changed...
		TLMaths::TTransform TempSceneTransform = SceneTransform;

		//	draw child
		DrawNode( pChild.GetObject(), pRenderNode, &TempSceneTransform, SceneColour, pChildCameraZoneNode );

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
void TLRender::TRenderTarget::DrawMeshWrapper(TLAsset::TMesh* pMesh,TRenderNode* pRenderNode,const TLMaths::TTransform& SceneTransform,TColour SceneColour,TPtrArray<TRenderNode>& PostRenderList)
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
		if ( !RenderNodeRenderFlags.IsSet( TRenderNode::RenderFlags::Debug_Wireframe ) )
		{
			//	make sure wireframe is off
			Opengl::EnableWireframe(FALSE);

			//	set the scene colour, and force enable alpha if the mesh needs it
			Opengl::SetSceneColour( SceneColour, pMesh->HasAlpha() );

			DrawMesh( *pMesh, pRenderNode, RenderNodeRenderFlags );
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

				DrawMesh( *pMesh, pRenderNode, WireframeRenderFlags );
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
			TPtr<TLAsset::TAsset>& pAsset = TLAsset::GetAsset("d_cross",TRUE);
			if ( pAsset && pAsset->GetAssetType() == "mesh" )
			{
				TLAsset::TMesh* pMesh = pAsset.GetObject<TLAsset::TMesh>();
				DrawMesh( *pMesh, pRenderNode, RenderNodeRenderFlags );
			}
		}

		//	render local bounds box in current [render object's] transform
		if ( RenderNodeRenderFlags.IsSet( TRenderNode::RenderFlags::Debug_LocalBoundsBox ) )
		{
			const TLMaths::TBox& RenderNodeBounds = pRenderNode->CalcLocalBoundsBox();
			if ( RenderNodeBounds.IsValid() )
			{
				TFlags<TRenderNode::RenderFlags::Flags> RenderFlags = pRenderNode->GetRenderFlags();
				RenderFlags.Set( TRenderNode::RenderFlags::Debug_Wireframe );
				RenderFlags.Set( TRenderNode::RenderFlags::Debug_Outline );
				RenderFlags.Clear( TRenderNode::RenderFlags::DepthRead );
				
				DrawMeshShape( RenderNodeBounds, pRenderNode, RenderFlags, FALSE );
			}
		}

		//	render world bounds box (outside current transform)
		if ( RenderNodeRenderFlags.IsSet( TRenderNode::RenderFlags::Debug_WorldBoundsBox ) )
		{
			const TLMaths::TBox& RenderNodeBounds = pRenderNode->GetWorldBoundsBox();
			if ( RenderNodeBounds.IsValid() )
			{
				TFlags<TRenderNode::RenderFlags::Flags> RenderFlags = pRenderNode->GetRenderFlags();
			
				//	setup specific params
				Opengl::EnableWireframe(TRUE);
				Opengl::SetSceneColour( TColour( 1.f, 1.f, 1.f, 1.f ) );
				Opengl::SetLineWidth( 1.f );

				DrawMeshShape( RenderNodeBounds, pRenderNode, RenderFlags, TRUE );
			}
		}

			//	render local bounds box in current [render object's] transform
		if ( RenderNodeRenderFlags.IsSet( TRenderNode::RenderFlags::Debug_LocalBoundsSphere ) )
		{
			const TLMaths::TSphere& RenderNodeBounds = pRenderNode->CalcLocalBoundsSphere();

			if ( RenderNodeBounds.IsValid() )
			{
				TPtr<TRenderNode> pTempRenderNode = new TRenderNode;
				pTempRenderNode->Copy( *pRenderNode );
				pTempRenderNode->ClearDebugRenderFlags();
				pTempRenderNode->GetRenderFlags().Set( TRenderNode::RenderFlags::Debug_Wireframe );
				//pTempRenderNode->GetRenderFlags().Set( TRenderNode::RenderFlags::ResetScene );	//	world bounds so reset scene
				pTempRenderNode->GetRenderFlags().Clear( TRenderNode::RenderFlags::DepthRead );
				pTempRenderNode->SetScale( RenderNodeBounds.GetRadius() );
				pTempRenderNode->SetTranslate( RenderNodeBounds.GetPos() );
				pTempRenderNode->SetMeshRef( "d_sphere" );
				PostRenderList.Add( pTempRenderNode );
			}
		}

		//	render world bounds box (outside current transform)
		if ( RenderNodeRenderFlags.IsSet( TRenderNode::RenderFlags::Debug_WorldBoundsSphere ) )
		{
			const TLMaths::TSphere& RenderNodeBounds = pRenderNode->GetWorldBoundsSphere();
			if ( RenderNodeBounds.IsValid() )
			{
				TPtr<TRenderNode> pTempRenderNode = new TRenderNode;
				pTempRenderNode->Copy( *pRenderNode );
				pTempRenderNode->ClearDebugRenderFlags();
				pTempRenderNode->GetRenderFlags().Set( TRenderNode::RenderFlags::Debug_Wireframe );
				pTempRenderNode->GetRenderFlags().Set( TRenderNode::RenderFlags::ResetScene );	//	world bounds so reset scene
				pTempRenderNode->GetRenderFlags().Clear( TRenderNode::RenderFlags::DepthRead );
				pTempRenderNode->SetScale( RenderNodeBounds.GetRadius() );
				pTempRenderNode->SetTranslate( RenderNodeBounds.GetPos() );
				pTempRenderNode->SetMeshRef( "d_sphere" );
				PostRenderList.Add( pTempRenderNode );
			}

		}

	}
	#endif
}


//-------------------------------------------------------------
//	render mesh asset
//-------------------------------------------------------------
void TLRender::TRenderTarget::DrawMesh(TLAsset::TMesh& Mesh,const TRenderNode* pRenderNode,const TFlags<TRenderNode::RenderFlags::Flags>& RenderFlags)
{
	const TArray<float3>* pVertexes = &Mesh.GetVertexes();
	const TArray<TColour>* pColours						= &Mesh.GetColours();
	const TArray<TLAsset::TMesh::Triangle>* pTriangles	= &Mesh.GetTriangles();
	const TArray<TLAsset::TMesh::Tristrip>* pTristrips	= &Mesh.GetTristrips();
	const TArray<TLAsset::TMesh::Trifan>* pTrifans		= &Mesh.GetTrifans();
	const TArray<TLAsset::TMesh::Line>* pLines			= &Mesh.GetLines();
	
	//	ignore colour vertex data if flag is not set
	if ( !RenderFlags( TRenderNode::RenderFlags::UseVertexColours ) )
		pColours = NULL;

	//	bind vertex data
	Opengl::Platform::BindVertexes( pVertexes );
	Opengl::Platform::BindColours( pColours );

	//	enable/disable depth test
	Opengl::EnableDepthRead( RenderFlags( TRenderNode::RenderFlags::DepthRead ) );

	//	enable/disable depth writing
	Opengl::EnableDepthWrite( RenderFlags( TRenderNode::RenderFlags::DepthWrite ) );

	//	setup line width if required
	if ( pLines && pLines->GetSize() > 0 && RenderFlags( TRenderNode::RenderFlags::UseMeshLineWidth ) ) 
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
		
		Opengl::SetLineWidth( LineWidth );
	}
		
	//	draw primitives
	Opengl::DrawPrimitives( Opengl::Platform::GetPrimTypeTriangle(),	pTriangles );
	Opengl::DrawPrimitives( Opengl::Platform::GetPrimTypeTristrip(),	pTristrips );
	Opengl::DrawPrimitives( Opengl::Platform::GetPrimTypeTrifan(),		pTrifans );
	Opengl::DrawPrimitives( Opengl::Platform::GetPrimTypeLineStrip(),	pLines );
	
	//	draw points like a primitive
	if ( RenderFlags( TRenderNode::RenderFlags::Debug_Points ) )
	{
		Opengl::SetPointSize( 8.f );
		Opengl::DrawPrimitivePoints( pVertexes );
	}

}



//-------------------------------------------------------------
//	
//-------------------------------------------------------------
void TLRender::TRenderTarget::SetRootQuadTreeZone(const TLMaths::TBox2D& ZoneShape)
{
	//	clean up the old one
	if ( m_pRootQuadTreeZone )
	{
		m_pRootQuadTreeZone->Shutdown();
		m_pRootQuadTreeZone = NULL;
	}

	//	create new root zone
	m_pRootQuadTreeZone = new TLMaths::TQuadTreeZone( ZoneShape, TLPtr::GetNullPtr<TLMaths::TQuadTreeZone>() );

#ifdef PREDIVIDE_RENDER_ZONES
	//	divide it all now
	m_pRootQuadTreeZone->DivideAll( m_pRootQuadTreeZone );
#endif

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

	DrawNode( &TempRenderNode, NULL, NULL, 1.f, NULL );

	for ( u32 z=0;	z<pZone->GetChildZones().GetSize();	z++ )
	{
		TPtr<TLMaths::TQuadTreeZone>& pChildZone = pZone->GetChildZones().ElementAt(z);
		Debug_DrawZone( pChildZone, zDepth + 0.01f, pCameraZoneNode );
	}
	
}


//--------------------------------------------------
//	the world-space box for the extents at the edges of the screen.
//--------------------------------------------------
const TLMaths::TBox2D& TLRender::TRenderTarget::GetWorldViewBox(float WorldDepth) const
{
	if ( m_pCamera->IsOrtho() )
	{
		const TOrthoCamera* pCamera = m_pCamera.GetObject<TOrthoCamera>();
		return pCamera->GetOrthoRenderTargetBox();
	}
	else
	{
		//	we make this up on the fly but for the sake of keeping this function fast
		//	for ortho we return a reference to a static box
		static TLMaths::TBox2D g_ProjectionWorldViewBox;

		//	get box at this depth
		const TProjectCamera* pCamera = m_pCamera.GetObject<TProjectCamera>();
		float ViewDepth = pCamera->GetPosition().z + WorldDepth;
		pCamera->GetWorldFrustumPlaneBox2D( ViewDepth, g_ProjectionWorldViewBox );

		return g_ProjectionWorldViewBox;
	}
}


//--------------------------------------------------
//	same as GetWorldViewBox but can be used before a render
//--------------------------------------------------
const TLMaths::TBox2D& TLRender::TRenderTarget::GetWorldViewBox(TPtr<TScreen>& pScreen,float WorldDepth)
{
	//	get render target size
	Type4<s32> Size;
	Type4<s32> MaxSize;
	pScreen->GetRenderTargetMaxSize( MaxSize );
	GetSize( Size, MaxSize );

	//	calc viewport sizes and boxes etc will be valid
	m_pCamera->SetRenderTargetSize( Size, pScreen->GetScreenShape() );

	return GetWorldViewBox( WorldDepth );
}

