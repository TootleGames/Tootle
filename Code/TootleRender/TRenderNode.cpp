#include "TRenderNode.h"
#include "TRendergraph.h"


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





TLRender::TRenderZoneNode::TRenderZoneNode(TRefRef RenderNodeRef) :
	m_RenderNodeRef		( RenderNodeRef )
{
}


//------------------------------------------------------------------
//	test if we are inside this zone's shape
//------------------------------------------------------------------
SyncBool TLRender::TRenderZoneNode::IsInShape(const TLMaths::TBox2D& Shape)
{
	//	grab render node
	TPtr<TLRender::TRenderNode>& pRenderNode = TLRender::g_pRendergraph->FindNode( m_RenderNodeRef );

	if ( !pRenderNode )
	{
#ifdef _DEBUG
		TTempString Debug_String("TRenderZoneNode is linked to a node ");
		m_RenderNodeRef.GetString( Debug_String );
		Debug_String.Append(" that doesnt exist?");
		TLDebug_Print(Debug_String);
#endif
		return SyncFalse;
	}

	const TLMaths::TBox2D& ZoneShape = Shape;

	//	test world pos first, quickest test :)
	Bool WorldPosIsValid;
	const float3& WorldPos = pRenderNode->GetWorldPos( WorldPosIsValid );
	if ( !WorldPosIsValid )
	{
		//	gr: if world pos is not valid, then MOST likely this is being called as a zone has been split and we're testing to
		//		see if the node needs moving. As we've not rendered yet, we might be out of date... all of the other bounds
		//		should be out of date too, so just wait and we'll move when we can...
		//TLDebug_Break("World pos is not valid during RenderNode Zone test");
		return SyncWait;
	}

	if ( ZoneShape.GetIntersection( WorldPos ) )
		return SyncTrue;

	Bool AnyBoundsValid = FALSE;
/*
	//	find a valid world bounds to test with (go in fastest order...)
	const TLMaths::TSphere& WorldBoundsSphere = pRenderNode->GetWorldBoundsSphere();
	if ( WorldBoundsSphere.IsValid() )
	{
		if ( ZoneShape.GetIntersection( WorldBoundsSphere ) )
			return SyncTrue;
		AnyBoundsValid = TRUE;
	}
*/
	const TLMaths::TBox& WorldBoundsBox = pRenderNode->GetWorldBoundsBox();
	if ( WorldBoundsBox.IsValid() )
	{
		if ( ZoneShape.GetIntersection( WorldBoundsBox ) )
			return SyncTrue;
		AnyBoundsValid = TRUE;
	}

	//	none of the bounds are valid, this is bad
	if ( !AnyBoundsValid )
	{
#ifdef _DEBUG
		//	if there is no mesh, then this is understandable and wont throw up an error
		TPtr<TLAsset::TMesh>& pMesh = pRenderNode->GetMeshAsset();
		if ( pMesh )
		{
			//	gr: if we get here whilst the zone we're in is splitting, then we return Wait, 
			//	which means the node isn't ready to be moved around
			//	this is a case of the zone is splitting, but our stuff is invalidated, so cant tell
			//	where we are any more
			//	although in this case I have, the ZoneOutOfDate == FALSE, but our world bounds ARE invalid?
			//TLDebug_Break("No valid world bounds shapes during RenderNode Zone test");
			return SyncWait;
		}
#endif
	}

	//	none of our bounds shapes intersected this zone's shape
	return SyncFalse;
}


//---------------------------------------------------------------
//	calculate all the world bounds we need to to do a zone test
//---------------------------------------------------------------
void TLRender::TRenderZoneNode::CalcWorldBounds(TLRender::TRenderNode* pRenderNode,const TLMaths::TTransform& SceneTransform)
{
	pRenderNode->CalcWorldBoundsBox( SceneTransform );
	pRenderNode->CalcWorldBoundsSphere( SceneTransform );
	//pRenderNode->CalcWorldBoundsCapsule( SceneTransform );
}







TLRender::TRenderNode::TRenderNode(TRefRef RenderNodeRef,TRefRef TypeRef) :
	TLGraph::TGraphNode<TLRender::TRenderNode>	( RenderNodeRef, TypeRef ),
	m_Data				( "Data" ),
	m_LineWidth			( 0.f ),
	m_WorldPosValid		( FALSE ),
	m_Colour			( 1.f, 1.f, 1.f, 1.f )
{
	//	setup defualt render flags
	m_RenderFlags.Set( RenderFlags::DepthRead );
	m_RenderFlags.Set( RenderFlags::DepthWrite );
	m_RenderFlags.Set( RenderFlags::Enabled );
	m_RenderFlags.Set( RenderFlags::UseVertexColours );
	m_RenderFlags.Set( RenderFlags::UseMeshLineWidth );
	m_RenderFlags.Set( RenderFlags::EnableCull );
}



//------------------------------------------------------------
//	copy render object DOES NOT COPY CHILDREN or parent! just properties
//------------------------------------------------------------
void TLRender::TRenderNode::Copy(const TRenderNode& OtherRenderNode)
{
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
TPtr<TLAsset::TMesh>& TLRender::TRenderNode::GetMeshAsset()
{
	//	re-fetch mesh if we need to
	if ( GetMeshRef().IsValid() && !m_pMeshCache )
	{
		m_pMeshCache = TLAsset::GetAsset( GetMeshRef(), TRUE );
		
		//	got a mesh, check it's the right type
#ifdef _DEBUG
		if ( m_pMeshCache )
		{
			if ( m_pMeshCache->GetAssetType() != "mesh" )
			{
				TLDebug_Break("MeshRef of render node is not a mesh");
				m_pMeshCache = NULL;
			}
		}
#endif
	}

	return m_pMeshCache;
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
	
	//	copy last valid bounds box
	m_LastWorldBoundsBox = m_WorldBoundsBox;

	//	invalidate parent's bounds if our world bounds have changed
	/*
	if ( TRUE )
	{
		if ( GetParent() )
		{
			SetBoundsInvalid(
		}
	}
	*/

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
	TPtr<TLAsset::TMesh>& pMesh = GetMeshAsset();
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

		if ( ChildBounds.IsValid() && !pChild->GetRenderFlags().IsSet(RenderFlags::ResetScene) )
		{
			//	gr: need to omit translate?
			ChildBounds.Transform( pChild->GetTransform() );

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
	if ( !InvalidateFlags(InvalidateLocalBounds) && !InvalidateFlags(InvalidateWorldBounds) && !InvalidateFlags(InvalidateWorldPos) )
		return;

	Bool ThisLocalBoundsChanged = FALSE;
	Bool ThisWorldBoundsChanged = FALSE;
	Bool HasSetRenderZoneInvalid = FALSE;

	//	if invalidating local, world must be invalidated
	if ( InvalidateFlags(InvalidateWorldBounds) || InvalidateFlags(InvalidateLocalBounds) )
	{
		if ( m_WorldBoundsBox.IsValid() )
		{
			Debug_PrintInvalidate( this, "world", "box" );
			m_WorldBoundsBox.SetInvalid();
			ThisWorldBoundsChanged = TRUE;
		}

		if ( m_WorldBoundsSphere.IsValid() )
		{
			Debug_PrintInvalidate( this, "world", "sphere" );
			m_WorldBoundsSphere.SetInvalid();
			ThisWorldBoundsChanged = TRUE;
		}

		if ( m_WorldBoundsCapsule.IsValid() )
		{
			Debug_PrintInvalidate( this, "world", "capsule" );
			m_WorldBoundsCapsule.SetInvalid();
			ThisWorldBoundsChanged = TRUE;
		}

		//	invalidate the zone of our RenderNodeZones - if our world bounds has changed then we
		//	may have moved to a new zone
		if ( !HasSetRenderZoneInvalid )
		{
			for ( u32 z=0;	z<m_RenderZoneNodes.GetSize();	z++ )
				m_RenderZoneNodes.ElementAt(z)->SetZoneOutOfDate();
			HasSetRenderZoneInvalid = TRUE;
		}
	}

	//	invalidate world pos
	if ( InvalidateFlags(InvalidateWorldPos) )
	{
		m_WorldPosValid = FALSE;
		if ( !HasSetRenderZoneInvalid )
		{
			for ( u32 z=0;	z<m_RenderZoneNodes.GetSize();	z++ )
				m_RenderZoneNodes.ElementAt(z)->SetZoneOutOfDate();
			HasSetRenderZoneInvalid = TRUE;
		}
	}

	//	invalidate local bounds
	if ( InvalidateFlags(InvalidateLocalBounds) )
	{
		if ( m_LocalBoundsBox.IsValid()  )
		{
			Debug_PrintInvalidate( this, "local", "box" );
			m_LocalBoundsBox.SetInvalid();
			ThisLocalBoundsChanged = TRUE;
		}

		if ( m_LocalBoundsSphere.IsValid()  )
		{
			Debug_PrintInvalidate( this, "local", "Sphere" );
			m_LocalBoundsSphere.SetInvalid();
			ThisLocalBoundsChanged = TRUE;
		}

		if ( m_LocalBoundsCapsule.IsValid()  )
		{
			Debug_PrintInvalidate( this, "local", "Capsule" );
			m_LocalBoundsCapsule.SetInvalid();
			ThisLocalBoundsChanged = TRUE;
		}
	}

	//	invalidate parent if local changes
	//	gr: invalidate if either change
	if ( ( ThisLocalBoundsChanged || ThisWorldBoundsChanged || InvalidateFlags(ForceInvalidateParents) ) && InvalidateFlags(InvalidateParents) )
	{
		TPtr<TRenderNode> pParent = GetParent();
		if ( pParent )
		{
			//	when invalidating our world bounds, the parent's LOCAL must invalidate, but not it's world
			TInvalidateFlags ParentInvalidateFlags = InvalidateFlags;
			if ( InvalidateFlags(InvalidateWorldBounds) )
			{
				ParentInvalidateFlags.Set( InvalidateLocalBounds );
				ParentInvalidateFlags.Clear( InvalidateWorldBounds );
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
//	calculate our new world position from the latest scene transform
//------------------------------------------------------------
void TLRender::TRenderNode::CalcWorldPos(const TLMaths::TTransform& SceneTransform)
{
	//	"center" of local node
	m_WorldPos.Set( m_Transform.GetTranslate() );
	
	//	transform
	SceneTransform.TransformVector( m_WorldPos );

	//	is valid!
	m_WorldPosValid = TRUE;
}

//------------------------------------------------------------
//	
//------------------------------------------------------------
const TLMaths::TSphere& TLRender::TRenderNode::CalcWorldBoundsSphere(const TLMaths::TTransform& SceneTransform)
{
	//	world bounds is already valid
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
	
	//	copy last valid bounds sphere
	m_LastWorldBoundsSphere = m_WorldBoundsSphere;

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
	
	//	copy last valid bounds sphere
	m_LastWorldBoundsCapsule = m_WorldBoundsCapsule;

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
	TPtr<TLAsset::TMesh>& pMesh = GetMeshAsset();
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

		if ( ChildBounds.IsValid() && !pChild->GetRenderFlags().IsSet(RenderFlags::ResetScene) )
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
	TPtr<TLAsset::TMesh>& pMesh = GetMeshAsset();
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

		if ( ChildBounds.IsValid() && !pChild->GetRenderFlags().IsSet(RenderFlags::ResetScene) )
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
	if ( !GetRenderFlags().IsSet( RenderFlags::ResetScene ) )
	{
		SetBoundsInvalid( TInvalidateFlags( InvalidateLocalBounds, InvalidateParents, ForceInvalidateParents ) );
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
		/*
#ifdef _DEBUG
		TLDebug_Print("Init message data: ");
		pMessage->Debug_PrintTree();
#endif
		*/

		Bool TransformChanged = FALSE;

		if ( pMessage->ImportData("Translate", m_Transform.GetTranslate() ) == SyncTrue )
		{
			m_Transform.SetTranslateValid();
			TransformChanged = TRUE;
		}

		if ( pMessage->ImportData("Scale", m_Transform.GetScale() ) == SyncTrue )
		{
			m_Transform.SetScaleValid();
			TransformChanged = TRUE;
		}

		//	transform has been set
		if ( TransformChanged )
			OnTransformChanged();

		pMessage->ImportData("LineWidth", m_LineWidth );

		if ( pMessage->ImportData("MeshRef", m_MeshRef ) == SyncTrue )
		{
			//	start loading the asset in case we havent loaded it already
			TLAsset::LoadAsset( m_MeshRef );

			//	mesh ref changed
			OnMeshRefChanged();
		}

		//	get render flags to set
		TPtrArray<TBinaryTree> FlagChildren;
		if ( pMessage->GetChildren("RFSet", FlagChildren ) )
		{
			u32 RenderFlagIndex = 0;
			for ( u32 f=0;	f<FlagChildren.GetSize();	f++ )
			{
				FlagChildren[f]->ResetReadPos();
				if ( FlagChildren[f]->Read( RenderFlagIndex ) )
					GetRenderFlags().Set( (RenderFlags::Flags)RenderFlagIndex );
			}
			FlagChildren.Empty();
		}

		//	get render flags to clear
		if ( pMessage->GetChildren("RFClear", FlagChildren ) )
		{
			u32 RenderFlagIndex = 0;
			for ( u32 f=0;	f<FlagChildren.GetSize();	f++ )
			{
				FlagChildren[f]->ResetReadPos();
				if ( FlagChildren[f]->Read( RenderFlagIndex ) )
					GetRenderFlags().Clear( (RenderFlags::Flags)RenderFlagIndex );
			}
			FlagChildren.Empty();
		}

		//	import colour
		if ( pMessage->ImportData("Colour", m_Colour ) )
			OnColourChanged();
	}

	//	do inherited init
	TLGraph::TGraphNode<TLRender::TRenderNode>::Initialise( pMessage );
}


