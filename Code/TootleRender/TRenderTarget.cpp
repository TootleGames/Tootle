#include "TRenderTarget.h"
#include "TScreen.h"
#include <TootleCore/TPtr.h>



//-----------------------------------------------------------
//	
//-----------------------------------------------------------
TLRender::TRenderTarget::TRenderTarget(const TRef& Ref) :
	m_Size				( g_MaxSize, g_MaxSize, g_MaxSize, g_MaxSize ),
	m_Ref				( Ref ),
	m_Debug_SceneCount	( 0 ),
	m_Debug_PolyCount	( 0 ),
	m_Debug_VertexCount	( 0 )
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
	//	render target disabled
	if ( !GetFlags()(Flag_Enabled) )
		return FALSE;

	//	need a camera
	if ( !m_pCamera )
		return FALSE;

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
		DrawNode( m_pRenderNodeClear.GetObject(), NULL, NULL );
	}

	//	draw the root render object and the rest will follow
	if ( m_pRootRenderNode )
	{
		DrawNode( m_pRootRenderNode.GetObject(), NULL, NULL );
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


//-------------------------------------------------------
//	apply camera transform to the scene
//-------------------------------------------------------
void TLRender::TRenderTarget::TranslateCamera()
{
	TPtr<TCamera>& pCamera = GetCamera();
	if ( !pCamera )
	{
		TLDebug_Break("Camera expected" );
		return;
	}

	//	make up a camera transform
	TLMaths::TTransform CameraTransform;

	if ( pCamera->IsOrtho() )
	{
		TLRender::TOrthoCamera* pOrthoCamera = GetCamera().GetObject<TLRender::TOrthoCamera>();
		CameraTransform.SetTranslate( pOrthoCamera->GetPosition() );
	}
	else
	{
		TLRender::TProjectCamera* pProjectCamera = GetCamera().GetObject<TLRender::TProjectCamera>();
		CameraTransform.SetMatrix( pProjectCamera->GetCameraMatrix(this) );
	}

	//	and apply transform
	Translate( CameraTransform );
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
	TLRender::TOrthoCamera* pCamera = GetCamera().GetObject<TLRender::TOrthoCamera>();
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

	m_pCamera->GetWorldRay( WorldRay, RenderTargetPos, RenderTargetSize );

	return TRUE;
}



//---------------------------------------------------------------
//	render a render node
//---------------------------------------------------------------
Bool TLRender::TRenderTarget::DrawNode(TRenderNode* pRenderNode,TRenderNode* pParentRenderNode,const TLMaths::TTransform* pSceneTransform)
{
	if ( !pRenderNode )
	{
		TLDebug_Break("Render node expected");
		return FALSE;
	}

	//	not enabled, dont render
	if ( !pRenderNode->GetRenderFlags().IsSet( TLRender::TRenderNode::RenderFlags::Enabled ) )
		return FALSE;

	//	merge colours
	TColour SceneColour = pRenderNode->GetColour();
	if ( pRenderNode->GetRenderFlags().IsSet( TLRender::TRenderNode::RenderFlags::MergeColour ) && pParentRenderNode )
	{
		//	merge colours
		SceneColour *= pParentRenderNode->GetColour();
	}

	//	check alpha on colour
	if ( !SceneColour.IsVisible() )
		return FALSE;

	const TFlags<TRenderNode::RenderFlags::Flags>& RenderNodeRenderFlags = pRenderNode->GetRenderFlags();
	Bool ResetScene = RenderNodeRenderFlags.IsSet(TLRender::TRenderNode::RenderFlags::ResetScene);

	//	do minimal calcuations to calc scene transformation - yes code is a bit of a pain, but this is very good for us :)
	//	only problem is, we can't reuse this code in another func as we lose the reference initialisation, which is the whole point
	TLMaths::TTransform NewSceneTransform;
	Bool NodeTrans = pRenderNode->GetTransform().HasAnyTransform();
	Bool SceneTrans = (pSceneTransform && !ResetScene) ? pSceneTransform->HasAnyTransform() : FALSE;
	const TLMaths::TTransform& SceneTransform = (NodeTrans&&SceneTrans) ? NewSceneTransform : ( NodeTrans ? pRenderNode->GetTransform() : *pSceneTransform );
	if ( NodeTrans && SceneTrans )
	{
		NewSceneTransform = *pSceneTransform;
		NewSceneTransform.Transform( pRenderNode->GetTransform() );
	}

	//	start new scene if we change the translation
	if ( NodeTrans )
	{
		if ( ResetScene )
			BeginSceneReset();
		else
			BeginScene();
		
		//	transform scene by node's transofrm
		Translate( pRenderNode->GetTransform() );
	}

	//	set scene colour
	SetSceneColour( SceneColour );

	//	list of nodes to render afterwards, but not stored in the tree
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

		//	array of temporary render nodes to render (debug stuff etc)
		TLAsset::TMesh* pMesh = pMeshAsset.GetObject();

		//	draw mesh!
		DrawMeshWrapper( pMesh, pRenderNode, SceneTransform, PostRenderList );
	}

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
		DrawNode( pChild.GetObject(), pRenderNode, &SceneTransform );

		#ifndef TLGRAPH_OWN_CHILDREN
		pChild = pChild->GetNext();
		#endif
	}

	//	draw our post-render nodes, deleting them as we go
	for ( s32 n=PostRenderList.LastIndex();	n>=FirstRenderNodeListIndex;	n-- )
	{
		//	gr: this MUST NOT be a reference. we need to make a new TPtr to keep the reference
		//		counting going. if DrawNode below empties the PostRenderList then this REFERENCE
		//		will be null'd
		//TPtr<TLRender::TRenderNode>& pChild = PostRenderList[c];
		TPtr<TLRender::TRenderNode> pChild = PostRenderList[n];

		TLMaths::TTransform TempSceneTransform = SceneTransform;

		//	draw child
		DrawNode( pChild.GetObject(), pRenderNode, &TempSceneTransform );

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
Bool TLRender::TRenderTarget::DrawMeshWrapper(TLAsset::TMesh* pMesh,TRenderNode* pRenderNode,const TLMaths::TTransform& SceneTransform,TPtrArray<TRenderNode>& PostRenderList)
{
	TFlags<TRenderNode::RenderFlags::Flags> DebugRenderNodeRenderFlags = pRenderNode->GetRenderFlags();
//	TFlags<TRenderNode::RenderFlags::Flags>& RenderNodeRenderFlags = TLDebug::IsEnabled() ? DebugRenderNodeRenderFlags : pRenderNode->GetRenderFlags();
	TFlags<TRenderNode::RenderFlags::Flags>& RenderNodeRenderFlags = DebugRenderNodeRenderFlags;
	
	//	modify render object flags for debug stuff
	DebugRenderNodeRenderFlags.Set( Debug_ForceRenderFlagsOn() );
	DebugRenderNodeRenderFlags.Clear( Debug_ForceRenderFlagsOff() );

	/*
	if ( !pMesh )
	{
		TLDebug_Break("Mesh expected");
		return TLRender::Draw_Error;
	}
	*/

	if ( pMesh )
	{
		//	do first render
		DrawMesh( *pMesh, pRenderNode, &RenderNodeRenderFlags );
	}

	//	render outline
	if ( pMesh && RenderNodeRenderFlags.IsSet( TRenderNode::RenderFlags::Debug_Outline ) )
	{
		//	save off current render state
//XXX		BeginScene();
		
		//	setup specific flags
		TFlags<TRenderNode::RenderFlags::Flags> RenderFlags = pRenderNode->GetRenderFlags();
		RenderFlags.Set( TRenderNode::RenderFlags::Debug_Wireframe );
		RenderFlags.Clear( TRenderNode::RenderFlags::DepthRead );

		DrawMesh( *pMesh, pRenderNode, &RenderFlags );

//XXX		EndScene();
	}

	//	render local bounds box in current [render object's] transform
	if ( RenderNodeRenderFlags.IsSet( TRenderNode::RenderFlags::Debug_LocalBoundsBox ) )
	{
		const TLMaths::TBox& RenderNodeBounds = pRenderNode->CalcLocalBoundsBox();
		if ( RenderNodeBounds.IsValid() )
		{
			TFlags<TRenderNode::RenderFlags::Flags> RenderFlags = pRenderNode->GetRenderFlags();
			RenderFlags.Set( TRenderNode::RenderFlags::Debug_Wireframe );
			RenderFlags.Clear( TRenderNode::RenderFlags::DepthRead );
			RenderFlags.Clear( TRenderNode::RenderFlags::EnableVBO );
			
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
			RenderFlags.Clear( TRenderNode::RenderFlags::EnableVBO );

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
			pTempRenderNode->GetRenderFlags().Clear( TRenderNode::RenderFlags::AffectsParentBounds );
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
			pTempRenderNode->GetRenderFlags().Clear( TRenderNode::RenderFlags::AffectsParentBounds );
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
			RenderFlags.Clear( TRenderNode::RenderFlags::EnableVBO );
	
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
			RenderFlags.Clear( TRenderNode::RenderFlags::EnableVBO );

			DrawMeshShape( RenderNodeBounds, pRenderNode, RenderFlags, TRUE );
		}
	}

	return TRUE;
}
