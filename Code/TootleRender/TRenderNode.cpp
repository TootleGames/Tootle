#include "TRenderNode.h"


//#define DEBUG_PrintBoundsInvalidationChanges





void Debug_PrintInvalidate(const TLRender::TRenderNode* pObject,const char* pSpaceType,const char* pShapeType)
{
#ifdef DEBUG_PrintBoundsInvalidationChanges
	TTempString DebugString;
	pObject->GetRenderNodeRef().GetString( DebugString );
	DebugString.Appendf(" %s %s invalidated", pSpaceType, pShapeType );
	TLDebug_Print( DebugString );
#endif
}


void Debug_PrintCalculating(const TLRender::TRenderNode* pObject,const char* pSpaceType,const char* pShapeType)
{
#ifdef DEBUG_PrintBoundsInvalidationChanges
	TTempString DebugString;
	pObject->GetRenderNodeRef().GetString( DebugString );
	DebugString.Appendf(" %s %s calculating...", pSpaceType, pShapeType );
	TLDebug_Print( DebugString );
#endif
}

TLRender::TRenderNode::TRenderNode(TRefRef RenderNodeRef,TRefRef TypeRef) :
	TLGraph::TGraphNode<TLRender::TRenderNode>	( RenderNodeRef, TypeRef ),
	m_Colour			( 1.f, 1.f, 1.f, 1.f ),
	m_Data				( "Data" ),
	m_LineWidth			( 0.f )
{
	//	setup defualt render flags
	m_RenderFlags.Set( RenderFlags::DepthRead );
	m_RenderFlags.Set( RenderFlags::DepthWrite );
	m_RenderFlags.Set( RenderFlags::AffectsParentBounds );
	m_RenderFlags.Set( RenderFlags::EnableVBO );
	m_RenderFlags.Set( RenderFlags::EnableFixedVerts );
	m_RenderFlags.Set( RenderFlags::Enabled );
	m_RenderFlags.Set( RenderFlags::MergeColour );	
	m_RenderFlags.Set( RenderFlags::UseVertexColours );	
}



//------------------------------------------------------------
//	copy render object DOES NOT COPY CHILDREN or parent! just properties
//------------------------------------------------------------
void TLRender::TRenderNode::Copy(const TRenderNode& OtherRenderNode)
{
//	m_Position				= OtherRenderNode.m_Position;
//	m_Rotation				= OtherRenderNode.m_Rotation;
//	m_Scale					= OtherRenderNode.m_Scale;
	m_Transform				= OtherRenderNode.m_Transform;
	m_Colour				= OtherRenderNode.m_Colour;
	m_LocalBoundsBox		= OtherRenderNode.m_LocalBoundsBox;
	m_WorldBoundsBox		= OtherRenderNode.m_WorldBoundsBox;
	m_LocalBoundsSphere		= OtherRenderNode.m_LocalBoundsSphere;
	m_WorldBoundsSphere		= OtherRenderNode.m_WorldBoundsSphere;
	m_LocalBoundsCapsule	= OtherRenderNode.m_LocalBoundsCapsule;
	m_WorldBoundsCapsule	= OtherRenderNode.m_WorldBoundsCapsule;
	m_RenderFlags			= OtherRenderNode.m_RenderFlags;
	m_MeshRef				= OtherRenderNode.m_MeshRef;
	m_Data					= OtherRenderNode.m_Data;

	SetRenderNodeRef( OtherRenderNode.GetRenderNodeRef() );
}

//------------------------------------------------------------
//	default behaviour fetches the mesh from the asset lib with our mesh ref
//------------------------------------------------------------
void TLRender::TRenderNode::GetMeshAsset(TPtr<TLAsset::TMesh>& pMesh)
{
	if ( !m_pMeshCache && GetMeshRef().IsValid() )
	{
		m_pMeshCache = TLAsset::GetAsset( GetMeshRef(), TRUE );
		
		if ( m_pMeshCache )
			if ( m_pMeshCache->GetAssetType() != "mesh" )
				m_pMeshCache = NULL;
	}

	pMesh = m_pMeshCache;
}


//------------------------------------------------------------
//	pre-draw routine for a render object
//------------------------------------------------------------
Bool TLRender::TRenderNode::Draw(TRenderTarget* pRenderTarget,TRenderNode* pParent,TPtrArray<TRenderNode>& PostRenderList)
{
	//	gr: todo: merge flags, colours etc from parent
	return TRUE;
}		

//------------------------------------------------------------
//	calculate our local bounds box (accumulating children) if out of date and return it
//	SceneMatrix and SceneScale include OUR local matrix/scale
//------------------------------------------------------------
const TLMaths::TBox& TLRender::TRenderNode::CalcWorldBoundsBox(const TLMaths::TTransform& SceneTransform)
{
	//	world bounds is valid
	if ( m_WorldBoundsBox.IsValid() )
		return m_WorldBoundsBox;

	Debug_PrintCalculating( this, "world", "box" );

	//	get/recalc local bounds box
	CalcLocalBoundsBox();

	//	if bounds is invalid, it's not going to affect the world bounds
	if ( !m_LocalBoundsBox.IsValid() )
		return m_WorldBoundsBox;

	//	tranform our local bounds into the world bounds
	m_WorldBoundsBox = m_LocalBoundsBox;

	//	transform box
	m_WorldBoundsBox.Transform( SceneTransform );
	//TransformBounds( m_WorldBoundsBox, *pSceneMatrix, *pSceneScale );

	return m_WorldBoundsBox;
}


//------------------------------------------------------------
//	return our current local bounds box and calculate if invalid
//------------------------------------------------------------
const TLMaths::TBox& TLRender::TRenderNode::CalcLocalBoundsBox()
{
	//	if bounds is valid, doesnt need recalculating
	if ( m_LocalBoundsBox.IsValid() )
		return m_LocalBoundsBox;
	
	Debug_PrintCalculating( this, "local", "box" );

	//	get bounds from mesh
	TPtr<TLAsset::TMesh> pMesh;
	GetMeshAsset( pMesh );
	if ( pMesh )
	{
		TLMaths::TBox& MeshBounds = pMesh->CalcBoundsBox();
		
		//	copy bounds of mesh to use as our own
		if ( MeshBounds.IsValid() )
		{
			m_LocalBoundsBox = MeshBounds;
		}
	}

	//	no children, just return what we have
	if ( !HasChildren() )
	{
		/*//	gr: this doesnt work, doesnt get invalidated when mesh does turn up...
		//	no bounds from mesh, and no children, so make up a VALID, but empty bounds box
		if ( !m_LocalBoundsBox.IsValid() )
		{
			Debug_PrintCalculating( this, "local", "EMPTY BOX" );
			m_LocalBoundsBox.Set( float3(0,0,0), float3(0,0,0) );
		}
		*/
		
		return m_LocalBoundsBox;
	}

	//	accumulate children's bounds
#ifdef TLGRAPH_OWN_CHILDREN
	TPtrArray<TLRender::TRenderNode>& NodeChildren = GetChildren();
	for ( u32 c=0;	c<NodeChildren.GetSize();	c++ )
	{
		TPtr<TLRender::TRenderNode>& pChild = NodeChildren[c];
#else
	TPtr<TRenderNode> pChild = GetChildFirst();
	while ( pChild )
	{
#endif
		//	get child's bounds
		TLMaths::TBox ChildBounds = pChild->CalcLocalBoundsBox();
		if ( !ChildBounds.IsValid() )
		{
			#ifndef TLGRAPH_OWN_CHILDREN
			pChild = pChild->GetNext();
			#endif
			continue;
		}

		if ( ChildBounds.IsValid() && pChild->GetRenderFlags().IsSet(RenderFlags::AffectsParentBounds) )
		{
			//	gr: need to omit translate?
			ChildBounds.Transform( pChild->GetTransform() );
			/*
			//	transform the child by it's own matrix (not ours)
			TLMaths::TMatrix ChildMatrix;
			pChild->GetMatrix( ChildMatrix );
			float3 ChildScale = pChild->GetScale();
			pChild->TransformBounds( ChildBounds, ChildMatrix, ChildScale );
			*/

			//	accumulate child
			m_LocalBoundsBox.Accumulate( ChildBounds );
		}

		#ifndef TLGRAPH_OWN_CHILDREN
		pChild = pChild->GetNext();
		#endif
	}

	//	all done! invalid or not, this is our bounds
	return m_LocalBoundsBox;
}


//------------------------------------------------------------
//	mark out bounds box as invalid, and invalidate parents bounds too
//------------------------------------------------------------
void TLRender::TRenderNode::SetBoundsInvalid(const TInvalidateFlags& InvalidateFlags)
{
	//	no change
	if ( !InvalidateFlags(InvalidateLocal) && !InvalidateFlags(InvalidateWorld) )
		return;

	Bool ThisLocalChanged = FALSE;
	Bool ThisWorldChanged = FALSE;

	//	if invalidating local, world must be invalidated
	if ( InvalidateFlags(InvalidateWorld) || InvalidateFlags( InvalidateLocal ) )
	{
		if ( m_WorldBoundsBox.IsValid() )
		{
			Debug_PrintInvalidate( this, "world", "box" );
			m_WorldBoundsBox.SetInvalid();
			ThisWorldChanged = TRUE;
		}

		if ( m_WorldBoundsSphere.IsValid() )
		{
			Debug_PrintInvalidate( this, "world", "sphere" );
			m_WorldBoundsSphere.SetInvalid();
			ThisWorldChanged = TRUE;
		}

		if ( m_WorldBoundsCapsule.IsValid() )
		{
			Debug_PrintInvalidate( this, "world", "capsule" );
			m_WorldBoundsCapsule.SetInvalid();
			ThisWorldChanged = TRUE;
		}
	}

	if ( InvalidateFlags(InvalidateLocal) )
	{
		if ( m_LocalBoundsBox.IsValid()  )
		{
			Debug_PrintInvalidate( this, "local", "box" );
			m_LocalBoundsBox.SetInvalid();
			ThisLocalChanged = TRUE;
		}

		if ( m_LocalBoundsSphere.IsValid()  )
		{
			Debug_PrintInvalidate( this, "local", "Sphere" );
			m_LocalBoundsSphere.SetInvalid();
			ThisLocalChanged = TRUE;
		}

		if ( m_LocalBoundsCapsule.IsValid()  )
		{
			Debug_PrintInvalidate( this, "local", "Capsule" );
			m_LocalBoundsCapsule.SetInvalid();
			ThisLocalChanged = TRUE;
		}
	}

	//	invalidate parent if local changes
	//	gr: invalidate if either change
	if ( ( ThisLocalChanged || ThisWorldChanged ) && InvalidateFlags(InvalidateParents) )
	{
		TPtr<TRenderNode> pParent = GetParent();
		if ( pParent )
		{
			//	when invalidating our world bounds, the parent's LOCAL must invalidate, but not it's world
			TInvalidateFlags ParentInvalidateFlags = InvalidateFlags;
			if ( InvalidateFlags(InvalidateWorld) )
			{
				ParentInvalidateFlags.Set( InvalidateLocal );
				ParentInvalidateFlags.Clear( InvalidateWorld );
			}

			pParent->SetBoundsInvalid(ParentInvalidateFlags);
		}
	}

	//	invalidate world bounds of children
	if ( InvalidateFlags(InvalidateChildren) && HasChildren() )
	{
		//	stop recursion overflow by not getting children to try and invalidate us again
		TInvalidateFlags ChildInvalidateFlags = InvalidateFlags;
		ChildInvalidateFlags.Clear( InvalidateParents );

#ifdef TLGRAPH_OWN_CHILDREN
		TPtrArray<TLRender::TRenderNode>& NodeChildren = GetChildren();
		for ( u32 c=0;	c<NodeChildren.GetSize();	c++ )
		{
			TPtr<TLRender::TRenderNode>& pChild = NodeChildren[c];
#else
		TPtr<TRenderNode> pChild = GetChildFirst();
		while ( pChild )
		{
#endif
			pChild->SetBoundsInvalid( ChildInvalidateFlags );
			
			#ifndef TLGRAPH_OWN_CHILDREN
			pChild = pChild->GetNext();
			#endif
		}
	}
}


//------------------------------------------------------------
//	
//------------------------------------------------------------
const TLMaths::TSphere& TLRender::TRenderNode::CalcWorldBoundsSphere(const TLMaths::TTransform& SceneTransform)
{
	//	world bounds is valid
	if ( m_WorldBoundsSphere.IsValid() )
		return m_WorldBoundsSphere;

	//	get/recalc local bounds box
	CalcLocalBoundsSphere();

	//	if bounds is invalid, it's not going to affect the world bounds
	if ( !m_LocalBoundsSphere.IsValid() )
		return m_WorldBoundsSphere;

	//	tranform our local bounds into the world bounds
	m_WorldBoundsSphere = m_LocalBoundsSphere;

	//	transform box
	m_WorldBoundsSphere.Transform( SceneTransform );

	return m_WorldBoundsSphere;
}

//------------------------------------------------------------
//	
//------------------------------------------------------------
const TLMaths::TCapsule& TLRender::TRenderNode::CalcWorldBoundsCapsule(const TLMaths::TTransform& SceneTransform)
{
	//	world bounds is valid
	if ( m_WorldBoundsCapsule.IsValid() )
		return m_WorldBoundsCapsule;

	//	get/recalc local bounds box
	CalcLocalBoundsCapsule();

	//	if bounds is invalid, it's not going to affect the world bounds
	if ( !m_LocalBoundsCapsule.IsValid() )
		return m_WorldBoundsCapsule;

	//	tranform our local bounds into the world bounds
	m_WorldBoundsCapsule = m_LocalBoundsCapsule;

	//	transform box
	m_WorldBoundsCapsule.Transform( SceneTransform );

	return m_WorldBoundsCapsule;
}


//------------------------------------------------------------
//	
//------------------------------------------------------------
const TLMaths::TSphere& TLRender::TRenderNode::CalcLocalBoundsSphere()
{
	//	if bounds is valid, doesnt need recalculating
	if ( m_LocalBoundsSphere.IsValid() )
		return m_LocalBoundsSphere;

	//	get bounds from mesh
	TPtr<TLAsset::TMesh> pMesh;
	GetMeshAsset( pMesh );
	if ( pMesh )
	{
		TLMaths::TSphere& MeshBounds = pMesh->CalcBoundsSphere();
		
		//	copy bounds of mesh to use as our own
		if ( MeshBounds.IsValid() )
		{
			m_LocalBoundsSphere = MeshBounds;
		}
	}

	//	no children, just return what we have
	if ( !HasChildren() )
		return m_LocalBoundsSphere;

	//	accumulate children's bounds
#ifdef TLGRAPH_OWN_CHILDREN
	TPtrArray<TLRender::TRenderNode>& NodeChildren = GetChildren();
	for ( u32 c=0;	c<NodeChildren.GetSize();	c++ )
	{
		TPtr<TLRender::TRenderNode>& pChild = NodeChildren[c];
#else
	TPtr<TRenderNode> pChild = GetChildFirst();
	while ( pChild )
	{
#endif
		//	get child's bounds
		TLMaths::TSphere ChildBounds = pChild->CalcLocalBoundsSphere();
		if ( !ChildBounds.IsValid() )
		{
			#ifndef TLGRAPH_OWN_CHILDREN
			pChild = pChild->GetNext();
			#endif
			continue;
		}

		if ( ChildBounds.IsValid() && pChild->GetRenderFlags().IsSet(RenderFlags::AffectsParentBounds) )
		{
			//	gr: omit translate?
			ChildBounds.Transform( pChild->GetTransform() );

			//	accumulate child
			m_LocalBoundsSphere.Accumulate( ChildBounds );
		}

		#ifndef TLGRAPH_OWN_CHILDREN
		pChild = pChild->GetNext();
		#endif
	}

	//	all done! invalid or not, this is our bounds
	return m_LocalBoundsSphere;
}


//------------------------------------------------------------
//	
//------------------------------------------------------------
const TLMaths::TCapsule& TLRender::TRenderNode::CalcLocalBoundsCapsule()
{
	//	if bounds is valid, doesnt need recalculating
	if ( m_LocalBoundsCapsule.IsValid() )
		return m_LocalBoundsCapsule;

	//	get bounds from mesh
	TPtr<TLAsset::TMesh> pMesh;
	GetMeshAsset( pMesh );
	if ( pMesh )
	{
		TLMaths::TCapsule& MeshBounds = pMesh->CalcBoundsCapsule();
		
		//	copy bounds of mesh to use as our own
		if ( MeshBounds.IsValid() )
		{
			m_LocalBoundsCapsule = MeshBounds;
		}
	}

	//	no children, just return what we have
	if ( !HasChildren() )
		return m_LocalBoundsCapsule;

	//	accumulate children's bounds
#ifdef TLGRAPH_OWN_CHILDREN
	TPtrArray<TLRender::TRenderNode>& NodeChildren = GetChildren();
	for ( u32 c=0;	c<NodeChildren.GetSize();	c++ )
	{
		TPtr<TLRender::TRenderNode>& pChild = NodeChildren[c];
#else
	TPtr<TRenderNode> pChild = GetChildFirst();
	while ( pChild )
	{
#endif
		//	get child's bounds
		TLMaths::TCapsule ChildBounds = pChild->CalcLocalBoundsCapsule();
		if ( !ChildBounds.IsValid() )
		{
			#ifndef TLGRAPH_OWN_CHILDREN
			pChild = pChild->GetNext();
			#endif
			continue;
		}

		if ( ChildBounds.IsValid() && pChild->GetRenderFlags().IsSet(RenderFlags::AffectsParentBounds) )
		{
			//	gr: omit translate?
			ChildBounds.Transform( pChild->GetTransform() );

			//	accumulate child
			m_LocalBoundsCapsule.Accumulate( ChildBounds );
		}

		#ifndef TLGRAPH_OWN_CHILDREN
		pChild = pChild->GetNext();
		#endif
	}

	//	all done! invalid or not, this is our bounds
	return m_LocalBoundsCapsule;
}


void TLRender::TRenderNode::ClearDebugRenderFlags()						
{
	m_RenderFlags.Clear( RenderFlags::Debug_Wireframe );
	m_RenderFlags.Clear( RenderFlags::Debug_Points );
	m_RenderFlags.Clear( RenderFlags::Debug_Outline );

	m_RenderFlags.Clear( RenderFlags::Debug_LocalBoundsBox );
	m_RenderFlags.Clear( RenderFlags::Debug_WorldBoundsBox );
	m_RenderFlags.Clear( RenderFlags::Debug_LocalBoundsSphere );
	m_RenderFlags.Clear( RenderFlags::Debug_WorldBoundsSphere );
	m_RenderFlags.Clear( RenderFlags::Debug_LocalBoundsCapsule );
	m_RenderFlags.Clear( RenderFlags::Debug_WorldBoundsCapsule );
}


	
void TLRender::TRenderNode::OnAdded()
{
	TLGraph::TGraphNode<TLRender::TRenderNode>::OnAdded();

	TTempString RefString;
	GetRenderNodeRef().GetString( RefString );
	RefString.Append(" added to graph... invalidating...");
	TLDebug_Print( RefString );

	//	invalidate bounds of self IF child affects bounds
	if ( GetRenderFlags().IsSet( RenderFlags::AffectsParentBounds ) )
	{
		SetBoundsInvalid( TInvalidateFlags( InvalidateLocal, InvalidateParents ) );
	}
}


//---------------------------------------------------------
//	generic render node init
//---------------------------------------------------------
void TLRender::TRenderNode::Initialise(TPtr<TLMessaging::TMessage>& pMessage)
{
	//	read init data
	if ( pMessage.IsValid() )
	{
		TLDebug_Print("Init message data: ");
		pMessage->Debug_PrintTree();

		if ( pMessage->ImportData("Translate", m_Transform.GetTranslate() ) == SyncTrue )
			m_Transform.SetTranslateValid();

		if ( pMessage->ImportData("Scale", m_Transform.GetScale() ) == SyncTrue )
			m_Transform.SetScaleValid();

		if ( pMessage->ImportData("MeshRef", m_MeshRef ) == SyncTrue )
		{
			//	start loading the asset in case we havent loaded it already
			TLAsset::LoadAsset( m_MeshRef );

			//	mesh ref changed
			OnMeshRefChanged();
		}
	}

	//	do inherited init
	TLGraph::TGraphNode<TLRender::TRenderNode>::Initialise( pMessage );
}


