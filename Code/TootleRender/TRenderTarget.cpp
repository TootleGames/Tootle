#include "TRenderTarget.h"
#include "TScreen.h"
#include "TScreenManager.h"
#include "TRenderGraph.h"

#if defined(TL_USER_GRAHAM)
	#define VERIFY_MESH_BEFORE_DRAW
#endif

//#define DEBUG_RENDER_DATUMS_IN_WORLD	//	if not defined, renders datums in local space which is faster (in world space will show up transform multiplication errors)
#define DEBUG_RENDER_ALL_DATUMS			FALSE
#define DEBUG_DATUMS_FORCE_RECALC		FALSE	//	when true will force the render node code to use the GetWorldTransform call that goes UP the render tree
#define DEBUG_ALWAYS_RESET_SCENE		FALSE	//	if things render incorrectly it suggests our calculated scene transform is incorrect

#if defined(_DEBUG) && !defined(TL_TARGET_IPOD) && !defined(TL_TARGET_IPAD)
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
	m_ScreenZ			( 0 ),
	m_pRootQuadTreeZone	( NULL )
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
//	start render process
//------------------------------------------------------------
void TLRender::TRenderTarget::Draw(TLRaster::TRasteriser& Rasteriser)
{
	s32 StartingSceneCount = m_Debug_SceneCount;

	//	render the clear object if we have it
#ifndef TLRENDER_DISABLE_CLEAR
	if ( m_pRenderNodeClear )
	{
		DrawNode( Rasteriser, *m_pRenderNodeClear, NULL, NULL, TColour( 1.f, 1.f, 1.f, 1.f ), NULL );
	}
#endif

	//	update the camera's zone
	Bool CameraInWorldZone = TRUE;
	if ( m_pRootQuadTreeZone )
	{
		if ( m_pCamera->IsZoneOutOfDate() )
		{
			TLMaths::TQuadTreeNode* pCameraQuadTreeNode = m_pCamera;
			m_pCamera->UpdateZone( *m_pRootQuadTreeZone );
		}

		if ( !m_pCamera->GetZone() )
			CameraInWorldZone = FALSE;
	}

	//	if the camera is outside the world zone, then all our render objects must be (assuming world zone is big enough) out of visibility too
	if ( CameraInWorldZone )
	{
		//	draw the root render object and the rest will follow
		TRenderNode* pRootRenderNode = GetRootRenderNode();
		if ( pRootRenderNode )
		{
			DrawNode( Rasteriser, *pRootRenderNode, NULL, NULL, TColour( 1.f, 1.f, 1.f, 1.f ), m_pRootQuadTreeZone ? m_pCamera.GetObjectPointer() : NULL );
		}
	}

	//	scene count should be zero after drawing...
	if ( StartingSceneCount != m_Debug_SceneCount )
	{
		TLDebug_Break( TString("More scenes active(%d) than we started with (%d)", m_Debug_SceneCount, StartingSceneCount ) );
		m_Debug_SceneCount = 0;
	}
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
			Type4<s32> RenderTargetMaxSize = pScreen->GetRenderTargetMaxSize();
			Type4<s32> ViewportSize = pScreen->GetViewportSize();
			Type4<s32> RenderTargetSize;
			GetSize( RenderTargetSize, RenderTargetMaxSize );
			pCamera->SetRenderTargetSize( RenderTargetSize, RenderTargetMaxSize, ViewportSize, pScreen->GetScreenShape() );
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
TLRender::TRenderNode* TLRender::TRenderTarget::GetRootRenderNode() const
{
	return TLRender::g_pRendergraph->FindNode( m_RootRenderNodeRef );
}


//---------------------------------------------------------------
//	check zone of node against camera's zone to determine visibility. 
//	if no scene transform is provided then we only do quick tests with no calculations. 
//	This can result in a SyncWait returned which means we need to do calculations to make sure of visibility
//---------------------------------------------------------------
SyncBool TLRender::TRenderTarget::IsRenderNodeVisible(TRenderNode& RenderNode,TLMaths::TQuadTreeNode*& pRenderZoneNode,TLMaths::TQuadTreeNode* pCameraZoneNode,const TLMaths::TTransform* pSceneTransform,Bool& RenderNodeIsInsideCameraZone)
{
	//	no camera zone, so must be visible (assume no culling)
	if ( !pCameraZoneNode )
		return SyncTrue;

	Bool QuickTest = (pSceneTransform == NULL);
	SyncBool IsTransformUpToDate = RenderNode.IsWorldTransformValid();
	SyncBool IsZoneUpToDate = SyncWait;

	//	find our render zone node
	if ( !pRenderZoneNode )
	{
		pRenderZoneNode = RenderNode.GetRenderZoneNode( GetRef() );

		//	if we have a zone, then set the up-to-date value
		if ( pRenderZoneNode )
			IsZoneUpToDate = pRenderZoneNode->IsZoneOutOfDate() ? SyncFalse : SyncTrue;

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
		if ( !pRenderZoneNode )
		{
			pRenderZoneNode = new TRenderZoneNode( RenderNode.GetNodeRef() );

			//	hold onto our new ZoneNode in our list
			if( !RenderNode.SetRenderZoneNode( GetRef(), pRenderZoneNode ) )
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
			if ( !m_pRootQuadTreeZone->AddNode( *pRenderZoneNode, TRUE ) )
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
		pRenderZoneNode->UpdateZone( *m_pRootQuadTreeZone );
	}

	//	if the zone we are inside, is inside the camera zone, then render (this should be the most likely case)
	TLMaths::TQuadTreeZone* pRenderNodeZone = pRenderZoneNode->GetZone();

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

	const TLMaths::TQuadTreeZone* pCameraZone = pCameraZoneNode->GetZone();

	//	no render zone, assume node/camera is out outside of root zone
	if ( !pZone || !pCameraZone )
		return FALSE;

	//	our zone is directly under the camera's zone
	if ( pZone->GetParentZone() == pCameraZone )
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
Bool TLRender::TRenderTarget::DrawNode(TLRaster::TRasteriser& Rasteriser,TRenderNode& RenderNode,TRenderNode* pParentRenderNode,const TLMaths::TTransform* pSceneTransform,TColour SceneColour,TLMaths::TQuadTreeNode* pCameraZoneNode)
{
	const TFlags<TRenderNode::RenderFlags::Flags>& RenderNodeRenderFlags = RenderNode.GetRenderFlags();

	//	not enabled, dont render
	if ( !RenderNodeRenderFlags.IsSet( TLRender::TRenderNode::RenderFlags::Enabled ) )
		return FALSE;

	//	if node colour is reset then set a new scene colour
	if ( RenderNodeRenderFlags.IsSet(TLRender::TRenderNode::RenderFlags::ResetColour) )
	{
		SceneColour = RenderNode.GetColour();
	}
	else
	{
		//	merge colour of scene
		if ( RenderNode.IsColourValid() )
		{
			SceneColour *= RenderNode.GetColour();

			//	alpha'd out (only applies if we're applying our colour, a la - IsColourValid)
			if ( SceneColour.GetAlphaf() < TLMaths_NearZero )
				return FALSE;
		}
	}

	//	do an initial fast-visibility test of the render node before calculating the scene transform
	TLMaths::TQuadTreeNode* pRenderZoneNode = NULL;
	Bool RenderNodeIsInsideCameraZone = FALSE;
	SyncBool IsInCameraRenderZone = SyncWait;

	//	do cull test if enabled on node
	if ( pCameraZoneNode && RenderNodeRenderFlags.IsSet( TLRender::TRenderNode::RenderFlags::EnableCull ) )
	{
		//	pass in NULL as the scene transform to do a very quick zone test - skips calculating bounds etc
		IsInCameraRenderZone = IsRenderNodeVisible( RenderNode, pRenderZoneNode, pCameraZoneNode, NULL, RenderNodeIsInsideCameraZone );

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
	const TLMaths::TTransform& NodeTransform = RenderNode.GetTransform();
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
	RenderNode.SetWorldTransform( SceneTransform );

	//	full visibility check of node, if not visible then skip render (and of children)
	if ( IsInCameraRenderZone == SyncWait )
	{
		IsInCameraRenderZone = IsRenderNodeVisible( RenderNode, pRenderZoneNode, pCameraZoneNode, &SceneTransform, RenderNodeIsInsideCameraZone );
		if ( IsInCameraRenderZone == SyncFalse )
		{
			m_Debug_NodeCulledCount++;
			return FALSE;
		}
	}

	//	count node as rendered
	m_Debug_NodeCount++;

	//	get raster data from node
	{
		TFixedArray<TLRaster::TRasterData,20> MeshRenderData;
		TFixedArray<TLRaster::TRasterSpriteData,20> SpriteRenderData;
		TPtrArray<TLAsset::TMesh> TemporaryMeshes;

		//	generic render of the node
		const TLRaster::TRasterData* pMainRaster = RenderNode.Render( MeshRenderData, SpriteRenderData, SceneColour );

		//	render debug stuff on the node
		#if defined(_DEBUG)
		{
			RenderNode.Debug_Render( MeshRenderData, SpriteRenderData, pMainRaster, TemporaryMeshes );
		}
		#endif //	_DEBUG

		//	render this raster data
		Rasteriser.Render( MeshRenderData );
		Rasteriser.Render( SpriteRenderData );

		//	send temporary meshes to the rasteriser so they're still valid until the rasteriser is finished
		Rasteriser.StoreTemporaryMeshes( TemporaryMeshes );
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
	TPointerArray<TLRender::TRenderNode>& NodeChildren = RenderNode.GetChildren();
	u32 ChildCount = NodeChildren.GetSize();
	if ( ChildCount > 0 )
	{
		TLMaths::TTransform ChildSceneTransform = SceneTransform;
		RenderNode.PreDrawChildren( *this, ChildSceneTransform);

		//	render children
		for ( u32 c=0;	c<ChildCount;	c++ )
		{
			TLRender::TRenderNode* pChild = NodeChildren[c];

			//	draw child
			DrawNode( Rasteriser, *pChild, &RenderNode, &ChildSceneTransform, SceneColour, pChildCameraZoneNode );
		}

		RenderNode.PostDrawChildren( *this );
	}

	return TRUE;
}



//-------------------------------------------------------------
//	
//-------------------------------------------------------------
void TLRender::TRenderTarget::SetRootQuadTreeZone(TLMaths::TQuadTreeZone* pQuadTreeZone)
{
	if ( m_pRootQuadTreeZone == pQuadTreeZone )
		return;

	//	clean up the old one
	if ( m_pRootQuadTreeZone )
	{
		m_pRootQuadTreeZone->Shutdown();
		TLMemory::Delete( m_pRootQuadTreeZone );
	}

	//	create new root zone
	m_pRootQuadTreeZone = pQuadTreeZone;

	if ( m_pRootQuadTreeZone )
	{
#ifdef PREDIVIDE_RENDER_ZONES
		//	divide it all now
		m_pRootQuadTreeZone->DivideAll();
#endif
	}

	//	invalidate camera's zone
	m_pCamera->SetZoneOutOfDate();
}

	
//--------------------------------------------------
//	
//--------------------------------------------------
void TLRender::TRenderTarget::Debug_DrawZone(TLRaster::TRasteriser& Rasteriser,TLMaths::TQuadTreeZone& Zone,float zDepth,TLMaths::TQuadTreeNode* pCameraZoneNode)
{
	TRenderNode TempRenderNode("xxx");
	TempRenderNode.SetMeshRef("d_quad");

	if ( pCameraZoneNode )
	{
		Bool Dummy;
		if ( !IsZoneVisible( pCameraZoneNode, &Zone, NULL, Dummy ) )
			return;
	}

	TempRenderNode.SetAlpha( 0.3f );
	if ( pCameraZoneNode )
	{
		if ( &Zone == pCameraZoneNode->GetZone() )
		{
			TempRenderNode.GetRenderFlags().Clear( TLRender::TRenderNode::RenderFlags::UseVertexColours );
			TempRenderNode.GetRenderFlags().Clear( TLRender::TRenderNode::RenderFlags::DepthRead );
			TempRenderNode.SetAlpha( 1.f );
		}
	}

	const TLMaths::TBox2D& ZoneBox = Zone.GetShape();
	TempRenderNode.SetTranslate( ZoneBox.GetMin().xyz(zDepth) );
	TempRenderNode.SetScale( ZoneBox.GetSize().xyz(1.f) );

	DrawNode( Rasteriser, TempRenderNode, NULL, NULL, TColour(1.f,1.f,1.f,1.f), NULL );

	for ( u32 z=0;	z<Zone.GetChildZones().GetSize();	z++ )
	{
		TLMaths::TQuadTreeZone& ChildZone = *(Zone.GetChildZones().ElementAt(z));
		Debug_DrawZone( Rasteriser, ChildZone, zDepth + 0.01f, pCameraZoneNode );
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


