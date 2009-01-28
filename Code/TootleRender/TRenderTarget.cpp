#include "TRenderTarget.h"
#include "TScreen.h"
#include <TootleCore/TPtr.h>

//	gr: because I made the platform opengl calls inline (for super speed) we have to include the platform header
#if defined(TL_TARGET_PC)
	#include "PC/PCRender.h"
#elif defined(TL_TARGET_IPOD)
	#include "IPod/IPodRender.h"
#else
	#error unknown platform
#endif

//#define DEBUG_DRAW_RENDERZONES
//#define DEBUG_DRAW_FRUSTUM
#define ENABLE_ALPHA			//	enable alphas (on by default)


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
}



//------------------------------------------------------------
//	dont allow render without a camera
//------------------------------------------------------------
Bool TLRender::TRenderTarget::BeginDraw(const Type4<s32>& MaxSize)			
{
	m_Debug_NodeCount = 0;
	m_Debug_NodeCulledCount = 0;

	//	render target disabled
	if ( !GetFlags()(Flag_Enabled) )
		return FALSE;

	//	need a camera
	if ( !m_pCamera )
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


	return TRUE;	
}


//------------------------------------------------------------
//	start render process
//------------------------------------------------------------
void TLRender::TRenderTarget::Draw()
{
	s32 StartingSceneCount = m_Debug_SceneCount;

	TPtr<TRenderNode> pNullParent;

	//	render the clear object if we have it
	if ( m_pRenderNodeClear )
	{
		DrawNode( m_pRenderNodeClear.GetObject(), NULL, NULL, 1.f, NULL );
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
			DrawNode( pRootRenderNode.GetObject(), NULL, NULL, 1.f, m_pRootQuadTreeZone ? m_pCamera.GetObject<TLMaths::TQuadTreeNode>() : NULL );
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
	if ( !GetCamera() )
		return;
	
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
#ifdef _DEBUG
	if ( m_pRootQuadTreeZone )
	{
		TTempString DebugString("RenderTarget ");
		GetRef().GetString( DebugString );
		DebugString.Appendf(" rendered %d nodes (%d culled)", m_Debug_NodeCount, m_Debug_NodeCulledCount );
		TLDebug_Print( DebugString );
	}
#endif

#if defined(DEBUG_DRAW_RENDERZONES)||defined(DEBUG_DRAW_FRUSTUM)
	if ( !m_pCamera->IsOrtho() && m_pRootQuadTreeZone )
	{
		BeginSceneReset();

#ifdef DEBUG_DRAW_RENDERZONES
		Debug_DrawZone( m_pRootQuadTreeZone, 0.f, m_pCamera.GetObject() );
#endif

#ifdef DEBUG_DRAW_FRUSTUM
		TProjectCamera* pCamera = GetCamera().GetObject<TProjectCamera>();
		pCamera->CalcFrustum();

		//	really quick bodge
		glLineWidth(20.f);
		glDisable( GL_DEPTH_TEST );
/*
		glBegin(GL_LINES);
		{	
			glColor3f( 1.f, 0.f, 0.f );

			//	front
			glVertex3fv( pCamera->m_Frustum.GetBox().GetBoxCorners().ElementAt(0) );
			glVertex3fv( pCamera->m_Frustum.GetBox().GetBoxCorners().ElementAt(1) );

			glVertex3fv( pCamera->m_Frustum.GetBox().GetBoxCorners().ElementAt(1) );
			glVertex3fv( pCamera->m_Frustum.GetBox().GetBoxCorners().ElementAt(2) );

			glVertex3fv( pCamera->m_Frustum.GetBox().GetBoxCorners().ElementAt(2) );
			glVertex3fv( pCamera->m_Frustum.GetBox().GetBoxCorners().ElementAt(3) );

			glVertex3fv( pCamera->m_Frustum.GetBox().GetBoxCorners().ElementAt(3) );
			glVertex3fv( pCamera->m_Frustum.GetBox().GetBoxCorners().ElementAt(0) );

			glColor3f( 1.f, 1.f, 1.f );
			//	rear
			glVertex3fv( pCamera->m_Frustum.GetBox().GetBoxCorners().ElementAt(4) );
			glVertex3fv( pCamera->m_Frustum.GetBox().GetBoxCorners().ElementAt(5) );

			glVertex3fv( pCamera->m_Frustum.GetBox().GetBoxCorners().ElementAt(5) );
			glVertex3fv( pCamera->m_Frustum.GetBox().GetBoxCorners().ElementAt(6) );

			glVertex3fv( pCamera->m_Frustum.GetBox().GetBoxCorners().ElementAt(6) );
			glVertex3fv( pCamera->m_Frustum.GetBox().GetBoxCorners().ElementAt(7) );

			glVertex3fv( pCamera->m_Frustum.GetBox().GetBoxCorners().ElementAt(7) );
			glVertex3fv( pCamera->m_Frustum.GetBox().GetBoxCorners().ElementAt(4) );

			glColor3f( 1.f, 1.f, 0.f );
			//	joins
			glVertex3fv( pCamera->m_Frustum.GetBox().GetBoxCorners().ElementAt(0) );
			glVertex3fv( pCamera->m_Frustum.GetBox().GetBoxCorners().ElementAt(4) );

			glVertex3fv( pCamera->m_Frustum.GetBox().GetBoxCorners().ElementAt(1) );
			glVertex3fv( pCamera->m_Frustum.GetBox().GetBoxCorners().ElementAt(5) );

			glVertex3fv( pCamera->m_Frustum.GetBox().GetBoxCorners().ElementAt(2) );
			glVertex3fv( pCamera->m_Frustum.GetBox().GetBoxCorners().ElementAt(6) );

			glVertex3fv( pCamera->m_Frustum.GetBox().GetBoxCorners().ElementAt(3) );
			glVertex3fv( pCamera->m_Frustum.GetBox().GetBoxCorners().ElementAt(7) );

		}
		glEnd();
*/

		TLMaths::TBoxOB PlaneBox;
		float worldz = 0.f;
		float viewz = pCamera->GetPosition().z + worldz;
		pCamera->GetPlaneBox( viewz, PlaneBox );
		TLMaths::TBox2D Box;
		Box.Accumulate( PlaneBox.GetBoxCorners() );

		TArray<float2> BoxCorners;
		Box.GetBoxCorners( BoxCorners );

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
#endif



		EndScene();
	}
#endif
}


//-------------------------------------------------------
//	setup projection mode
//-------------------------------------------------------
Bool TLRender::TRenderTarget::BeginProjectDraw(const Type4<s32>& Size)
{
	if ( !GetCamera() )
		return FALSE;

	return TRUE;
}

//-------------------------------------------------------
//
//-------------------------------------------------------
void TLRender::TRenderTarget::EndProjectDraw()
{
}


//-------------------------------------------------------
//	setup ortho projection mode
//-------------------------------------------------------
Bool TLRender::TRenderTarget::BeginOrthoDraw(const Type4<s32>& Size)
{
	if ( !GetCamera() )
		return FALSE;

	return TRUE;
}

//-------------------------------------------------------
//
//-------------------------------------------------------
void TLRender::TRenderTarget::EndOrthoDraw()
{
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
//	convert our relative size to the opengl viewport size (upside down) - returns FALSE if too small to be seen
//-------------------------------------------------------------
Bool TLRender::TRenderTarget::GetViewportSize(Type4<s32>& ViewportSize,const Type4<s32>& MaxSize)
{
	//	work out the render target's rendering size (viewport)
	GetSize( ViewportSize, MaxSize );

	//	opengl viewport goes from bottom to top, so realign inside the parent rect
	//	gr: this is wrong I think... reinvestigate when I work out what i needs to do
//	ViewportSize.y = MaxSize.y - ( Size.y + Size.Height() );
//	ViewportSize.Height() = Size.Height();

	//	too small to render into!
	if ( ViewportSize.Width() <= 1 || ViewportSize.Height() <= 1 )
		return FALSE;

	return TRUE;
}


//-------------------------------------------------------------
//	get the orthographic dimensions (0-100 on width). returns FALSE if not using an ortho camera
//-------------------------------------------------------------
Bool TLRender::TRenderTarget::GetOrthoSize(Type4<float>& OrthoSize,const Type4<s32>& MaxSize)
{
	//	get the ortho camera
	TPtr<TCamera>& pCameraPtr = GetCamera();
	TLRender::TOrthoCamera* pCamera = pCameraPtr ? pCameraPtr.GetObject<TLRender::TOrthoCamera>() : NULL;
	if ( !pCamera )
		return FALSE;

	if ( !pCamera->IsOrtho() )
		return FALSE;

	//	convert our size to ortho size
	Type4<s32> Size;
	GetSize( Size, MaxSize );

	pCamera->GetOrthoSize( OrthoSize, Size );

	return TRUE;
}


//---------------------------------------------------------------
//	get world pos from 2d point inside our rendertarget size
//---------------------------------------------------------------
Bool TLRender::TRenderTarget::GetWorldRay(TLMaths::TLine& WorldRay,const Type2<s32>& RenderTargetPos,const Type4<s32>& RenderTargetSize) const
{
	if ( !m_pCamera )
		return FALSE;

	return m_pCamera->GetWorldRay( WorldRay, RenderTargetPos, RenderTargetSize );
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
	
			//	update scene transform before we update our zone to ensure correct(latest) bounds is calculated
			pRenderZoneNode->CalcWorldBounds( pRenderNode, *pSceneTransform );

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

		//	update scene transform before we update our zone to ensure correct(latest) bounds is calculated
		pRenderZoneNode->CalcWorldBounds( pRenderNode, *pSceneTransform );

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
		//	update scene transform before we update our zone to ensure correct(latest) bounds is calculated
		pRenderZoneNode->CalcWorldBounds( pRenderNode, *pSceneTransform );

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
Bool TLRender::TRenderTarget::DrawNode(TRenderNode* pRenderNode,TRenderNode* pParentRenderNode,const TLMaths::TTransform* pSceneTransform,float SceneAlpha,TLMaths::TQuadTreeNode* pCameraZoneNode)
{
	if ( !pRenderNode )
	{
		TLDebug_Break("Render node expected");
		return FALSE;
	}

	//	not enabled, dont render
	if ( !pRenderNode->GetRenderFlags().IsSet( TLRender::TRenderNode::RenderFlags::Enabled ) )
		return FALSE;

	const TFlags<TRenderNode::RenderFlags::Flags>& RenderNodeRenderFlags = pRenderNode->GetRenderFlags();
	Bool ResetScene = RenderNodeRenderFlags.IsSet(TLRender::TRenderNode::RenderFlags::ResetScene);

	if ( ResetScene )
		SceneAlpha = 1.f;
	
	//	merge alpha of scene
	SceneAlpha *= pRenderNode->GetAlpha();

	//	alpha'd out
	if ( SceneAlpha < TLMaths::g_NearZero )
		return FALSE;

	//	check visibility of node, if not visible then skip render (and of children)
	TPtr<TLMaths::TQuadTreeNode>* ppRenderZoneNode = NULL;

	//	pass in NULL as the scene transform to do a very quick zone test
	Bool RenderNodeIsInsideCameraZone = FALSE;
	SyncBool IsInCameraRenderZone = IsRenderNodeVisible( pRenderNode, ppRenderZoneNode, pCameraZoneNode, NULL, RenderNodeIsInsideCameraZone );

	//	after quick check we know the node is in a zone the camera cannot see
	if ( IsInCameraRenderZone == SyncFalse )
	{
		m_Debug_NodeCulledCount++;
		return FALSE;
	}

	//	do minimal calcuations to calc scene transformation - yes code is a bit of a pain, but this is very good for us :)
	//	only problem is, we can't reuse this code in another func as we lose the reference initialisation, which is the whole speed saving
	TLMaths::TTransform NewSceneTransform;
	Bool NodeTrans = pRenderNode->GetTransform().HasAnyTransform();
	Bool SceneTrans = (pSceneTransform && !ResetScene) ? pSceneTransform->HasAnyTransform() : FALSE;
	const TLMaths::TTransform& SceneTransform = (NodeTrans&&SceneTrans) ? NewSceneTransform : ( (NodeTrans||(!pSceneTransform)) ? pRenderNode->GetTransform() : *pSceneTransform );
	if ( NodeTrans && SceneTrans )
	{
		NewSceneTransform = *pSceneTransform;
		NewSceneTransform.Transform( pRenderNode->GetTransform() );
	}

	//	calc latest world pos for node
	if ( !pRenderNode->IsWorldPosValid() )
	{
		pRenderNode->CalcWorldPos( SceneTransform );
	}

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
			if ( &pRenderNode->GetTransform() != &SceneTransform )
			{
				TLDebug_Break("Im pretty sure these should be the same - in which case use SceneTransform");
			}
		}
		else
		{
			BeginScene();
		}

		//	transform scene by node's transofrm
		Opengl::SceneTransform( pRenderNode->GetTransform() );
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
		TPtr<TLAsset::TMesh> pMeshAsset;
		pRenderNode->GetMeshAsset( pMeshAsset );
		
		//	always calculate some things
		if ( RenderNodeRenderFlags.IsSet( TRenderNode::RenderFlags::CalcWorldBoundsBox ) )
			pRenderNode->CalcWorldBoundsBox( SceneTransform );

		if ( RenderNodeRenderFlags.IsSet( TRenderNode::RenderFlags::CalcWorldBoundsSphere ) )
			pRenderNode->CalcWorldBoundsSphere( SceneTransform );

		//	draw mesh!
		DrawMeshWrapper( pMeshAsset.GetObject(), pRenderNode, SceneTransform, SceneAlpha, PostRenderList );
	}

	//	if this render node is in a zone under the camera's zone, then we KNOW the child nodes are going to be visible, so we dont need to provide the camera's
	//	render zone. Will just mean we can skip render zone checks
	//TPtr<TLMaths::TQuadTreeZone>& pChildCameraRenderZone = RenderNodeIsInsideCameraZone ? TLPtr::GetNullPtr<TLMaths::TQuadTreeZone>() : pCameraRenderZone;
	TLMaths::TQuadTreeNode* pChildCameraZoneNode = pCameraZoneNode;

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
		DrawNode( pChild.GetObject(), pRenderNode, &SceneTransform, SceneAlpha, pChildCameraZoneNode );

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

		TLMaths::TTransform TempSceneTransform = SceneTransform;

		//	draw child
		DrawNode( pChild.GetObject(), pRenderNode, &TempSceneTransform, SceneAlpha, pChildCameraZoneNode );

		//	remove from list
		PostRenderList.RemoveAt(n);
	}

	//	clean up scene
	if ( NodeTrans )
	{
		EndScene();
	}

	return TRUE;
}


//-------------------------------------------------------------
//	
//-------------------------------------------------------------
void TLRender::TRenderTarget::DrawMeshWrapper(TLAsset::TMesh* pMesh,TRenderNode* pRenderNode,const TLMaths::TTransform& SceneTransform,float SceneAlpha,TPtrArray<TRenderNode>& PostRenderList)
{
#ifdef _DEBUG
	TFlags<TRenderNode::RenderFlags::Flags> RenderNodeRenderFlags = pRenderNode->GetRenderFlags();

	//	modify render object flags for debug stuff
	RenderNodeRenderFlags.Set( Debug_ForceRenderFlagsOn() );
	RenderNodeRenderFlags.Clear( Debug_ForceRenderFlagsOff() );
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

			#ifdef ENABLE_ALPHA
			{
				//	enable alpha - update the scene alpha (recursing alpha)
				if ( SceneAlpha < TLMaths::g_NearOne )
					Opengl::SetSceneAlpha( SceneAlpha );
				else
					//	full-opaque scene colour, but might need to enable alpha for the mesh
					Opengl::EnableAlpha( pMesh->HasAlpha() );
			}
			#endif

			DrawMesh( *pMesh, pRenderNode, RenderNodeRenderFlags );
		}

		//	render wireframe and/or outline
		if ( RenderNodeRenderFlags.IsSet( TRenderNode::RenderFlags::Debug_Outline ) || RenderNodeRenderFlags.IsSet( TRenderNode::RenderFlags::Debug_Wireframe ) )
		{
			//	setup specific params
			Opengl::EnableWireframe(TRUE);
			Opengl::SetSceneAlpha( 1.f );
			Opengl::EnableAlpha( FALSE );
			Opengl::SetLineWidth( 1.f );

			TFlags<TRenderNode::RenderFlags::Flags> WireframeRenderFlags = RenderNodeRenderFlags;
			WireframeRenderFlags.Clear( TRenderNode::RenderFlags::UseVertexColours );
			WireframeRenderFlags.Clear( TRenderNode::RenderFlags::UseMeshLineWidth );

			DrawMesh( *pMesh, pRenderNode, WireframeRenderFlags );
		}
	}


	//	non-mesh debug renders
	#ifdef _DEBUG
	{
		//	render local bounds box in current [render object's] transform
		if ( RenderNodeRenderFlags.IsSet( TRenderNode::RenderFlags::Debug_LocalBoundsBox ) )
		{
			const TLMaths::TBox& RenderNodeBounds = pRenderNode->CalcLocalBoundsBox();
			if ( RenderNodeBounds.IsValid() )
			{
				TFlags<TRenderNode::RenderFlags::Flags> RenderFlags = pRenderNode->GetRenderFlags();
				RenderFlags.Set( TRenderNode::RenderFlags::Debug_Wireframe );
				RenderFlags.Clear( TRenderNode::RenderFlags::DepthRead );
				
				DrawMeshShape( RenderNodeBounds, pRenderNode, RenderFlags, FALSE );
			}
		}

		//	render world bounds box (outside current transform)
		if ( RenderNodeRenderFlags.IsSet( TRenderNode::RenderFlags::Debug_WorldBoundsBox ) )
		{
			const TLMaths::TBox& RenderNodeBounds = pRenderNode->CalcWorldBoundsBox( SceneTransform );
			if ( RenderNodeBounds.IsValid() )
			{
				TFlags<TRenderNode::RenderFlags::Flags> RenderFlags = pRenderNode->GetRenderFlags();
				RenderFlags.Set( TRenderNode::RenderFlags::Debug_Wireframe );
				RenderFlags.Clear( TRenderNode::RenderFlags::DepthRead );

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
			const TLMaths::TSphere& RenderNodeBounds = pRenderNode->CalcWorldBoundsSphere( SceneTransform );
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

		//	render local bounds box in current [render object's] transform
		if ( RenderNodeRenderFlags.IsSet( TRenderNode::RenderFlags::Debug_LocalBoundsCapsule ) )
		{
			const TLMaths::TCapsule& RenderNodeBounds = pRenderNode->CalcLocalBoundsCapsule();
			if ( RenderNodeBounds.IsValid() )
			{
				TFlags<TRenderNode::RenderFlags::Flags> RenderFlags = pRenderNode->GetRenderFlags();
				RenderFlags.Set( TRenderNode::RenderFlags::Debug_Wireframe );
				RenderFlags.Clear( TRenderNode::RenderFlags::DepthRead );
		
				DrawMeshShape( RenderNodeBounds, pRenderNode, RenderFlags, FALSE );
			}
		}

		//	render world bounds box (outside current transform)
		if ( RenderNodeRenderFlags.IsSet( TRenderNode::RenderFlags::Debug_WorldBoundsCapsule ) )
		{
			const TLMaths::TCapsule& RenderNodeBounds = pRenderNode->CalcWorldBoundsCapsule( SceneTransform );
			if ( RenderNodeBounds.IsValid() )
			{
				TFlags<TRenderNode::RenderFlags::Flags> RenderFlags = pRenderNode->GetRenderFlags();
				RenderFlags.Set( TRenderNode::RenderFlags::Debug_Wireframe );
				RenderFlags.Clear( TRenderNode::RenderFlags::DepthRead );

				DrawMeshShape( RenderNodeBounds, pRenderNode, RenderFlags, TRUE );
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