#include "TRenderNode.h"
#include "TRendergraph.h"

#include <TootleScene/TScenegraph.h>

//#define DEBUG_PrintBoundsInvalidationChanges


//	if defined we re-calculate the bounds box of the render node. Need to balance this CPU vs GPU cost.
//	if not defined we only do a box test if the current one is up to date (i.e. something else needed it so it's calculated)
#define RECALC_BOX_FOR_RENDERZONE_TEST



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
	m_RenderNodeRef		( RenderNodeRef ),
	m_pRenderNode		( TLRender::g_pRendergraph->FindNode( m_RenderNodeRef ) )
{
}


//------------------------------------------------------------------
//	test if we are inside this zone's shape
//------------------------------------------------------------------
SyncBool TLRender::TRenderZoneNode::IsInShape(const TLMaths::TBox2D& Shape)
{
	//	get c-pointer
	TLRender::TRenderNode* pRenderNode = m_pRenderNode;
	if ( !pRenderNode )
	{
		TLDebug_Break("Missing render node should have been caught in HasZoneShape - if this is during a zone-split then this TPtr should be already set...");
		return SyncFalse;
	}

	const TLMaths::TBox2D& ZoneShape = Shape;

	//	test world pos first, quickest test :)
	SyncBool WorldPosIsValid;
	const float3& WorldPos = pRenderNode->GetWorldPos( WorldPosIsValid );
	if ( WorldPosIsValid != SyncTrue )
	{
		//	gr: if world pos is not valid, then MOST likely this is being called as a zone has been split and we're testing to
		//		see if the node needs moving. As we've not rendered yet, we might be out of date... all of the other bounds
		//		should be out of date too, so just wait and we'll move when we can...
		//TLDebug_Break("World pos is not valid during RenderNode Zone test");
		return SyncWait;
	}

	if ( ZoneShape.GetIntersection( WorldPos ) )
		return SyncTrue;

	//	test sphere first as it's fastest
	const TLMaths::TSphere2D& WorldBoundsSphere = pRenderNode->GetWorldBoundsSphere2D().GetSphere();

	//	if bounds are not valid then we probably don't have a LOCAL bounds inside the render node, 
	//	so cannot produce a world bounds. Most likely means we have no mesh
	if ( !WorldBoundsSphere.IsValid() )
	{
#ifdef _DEBUG
		//	if there is no mesh, then this is understandable and wont throw up an error
		TPtr<TLAsset::TMesh>& pMesh = pRenderNode->GetMeshAsset(TRUE);
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
		//	no bounds, and world pos is not inside shape, so fail
		return SyncFalse;
	}
		
	//	outside sphere - so fail
	if ( !ZoneShape.GetIntersection( WorldBoundsSphere ) )
		return SyncFalse;
	

	//	even if we're inside the bounds sphere do a tighter check with the box
	SyncBool BoundsBoxValid;
#ifdef RECALC_BOX_FOR_RENDERZONE_TEST
	//	get latest bounds box (may need calculation)
	const TLMaths::TBox2D& WorldBoundsBox = pRenderNode->GetWorldBoundsBox2D().GetBox();
	BoundsBoxValid = WorldBoundsBox.IsValid() ? SyncTrue : SyncFalse;
#else
	//	get current box - may be out of date (syncwait) or uncalculated (false)
	const TLMaths::TBox2D& WorldBoundsBox = pRenderNode->GetWorldBoundsBox2D(BoundsBoxValid);
#endif
	if ( BoundsBoxValid == SyncTrue )
	{
		//	outside box - not visible
		if ( !ZoneShape.GetIntersection( WorldBoundsBox ) )
			return SyncFalse;
	}

	//	inside sphere, and inside box
	return SyncTrue;
}



//------------------------------------------------------------------
//	Do initial tests to see if the shape intersections will never work
//------------------------------------------------------------------
Bool TLRender::TRenderZoneNode::HasZoneShape()
{
	//	grab render node if we don't have it
	if ( !m_pRenderNode )
	{
		m_pRenderNode = TLRender::g_pRendergraph->FindNode( m_RenderNodeRef );

		//	no render node, never gonna intersect with the shape
		if ( !m_pRenderNode )
		{
	#ifdef _DEBUG
			TTempString Debug_String("TRenderZoneNode is linked to a node ");
			m_RenderNodeRef.GetString( Debug_String );
			Debug_String.Append(" that doesnt exist?");
			TLDebug_Print(Debug_String);
	#endif
			return FALSE;
		}
	}

	//	no up-to-date world transform, bail out
	if ( m_pRenderNode->IsWorldTransformValid() != SyncTrue )
		return FALSE;

	return TRUE;
}







TLRender::TRenderNode::TRenderNode(TRefRef RenderNodeRef,TRefRef TypeRef) :
	TLGraph::TGraphNode<TLRender::TRenderNode>	( RenderNodeRef, TypeRef ),
	m_Data						( "Data" ),
	m_LineWidth					( 0.f ),
	m_WorldPosValid				( SyncFalse ),
	m_Colour					( 1.f, 1.f, 1.f, 1.f ),
	m_WorldTransformValid		( SyncFalse )
{
	//	setup defualt render flags
	m_RenderFlags.Set( RenderFlags::DepthRead );
	m_RenderFlags.Set( RenderFlags::DepthWrite );
	m_RenderFlags.Set( RenderFlags::Enabled );
	m_RenderFlags.Set( RenderFlags::UseVertexColours );
	m_RenderFlags.Set( RenderFlags::UseVertexUVs );
	m_RenderFlags.Set( RenderFlags::UseMeshLineWidth );
	m_RenderFlags.Set( RenderFlags::EnableCull );
	m_RenderFlags.Set( RenderFlags::InvalidateBoundsByChildren );
}



//------------------------------------------------------------
//	copy render object DOES NOT COPY CHILDREN or parent! just properties
//------------------------------------------------------------
void TLRender::TRenderNode::Copy(const TRenderNode& OtherRenderNode)
{
	TLDebug_Break("still used? - code is out of date");
	/*
	m_Transform				= OtherRenderNode.m_Transform;
	m_Colour				= OtherRenderNode.m_Colour;
	m_LocalBoundsBox		= OtherRenderNode.m_LocalBoundsBox;
	m_WorldBoundsBox		= OtherRenderNode.m_WorldBoundsBox;
	m_LocalBoundsSphere		= OtherRenderNode.m_LocalBoundsSphere;
	m_WorldBoundsSphere		= OtherRenderNode.m_WorldBoundsSphere;
	m_RenderFlags			= OtherRenderNode.m_RenderFlags;
	m_MeshRef				= OtherRenderNode.m_MeshRef;
	m_Data					= OtherRenderNode.m_Data;

	SetRenderNodeRef( OtherRenderNode.GetRenderNodeRef() );
	*/
}

//------------------------------------------------------------
//	default behaviour fetches the mesh from the asset lib with our mesh ref
//------------------------------------------------------------
TPtr<TLAsset::TMesh>& TLRender::TRenderNode::GetMeshAsset(Bool BlockLoad)
{
	//	re-fetch mesh if we need to
	if ( GetMeshRef().IsValid() && !m_pMeshCache )
	{
		m_pMeshCache = TLAsset::LoadAsset( GetMeshRef(), BlockLoad, TRef_Static4(M,e,s,h) );
	}

	return m_pMeshCache;
}

//------------------------------------------------------------
//	default behaviour fetches the mesh from the asset lib with our mesh ref
//------------------------------------------------------------
TPtr<TLAsset::TTexture>& TLRender::TRenderNode::GetTextureAsset(Bool BlockLoad)
{
	//	re-fetch mesh if we need to
	if ( GetTextureRef().IsValid() && !m_pTextureCache )
	{
		m_pTextureCache = TLAsset::LoadAsset( GetTextureRef(), BlockLoad, TRef_Static(T,e,x,t,u) );
	}

	return m_pTextureCache;
}


//------------------------------------------------------------
//	pre-draw routine for a render object
//------------------------------------------------------------
Bool TLRender::TRenderNode::Draw(TRenderTarget* pRenderTarget,TRenderNode* pParent,TPtrArray<TRenderNode>& PostRenderList)
{
	//	base type aborts drawing early if no mesh ref assigned
	if ( m_MeshRef.IsValid() )
		return TRUE;

	return FALSE;
}		


//------------------------------------------------------------
//	mark out bounds box as invalid, and invalidate parents bounds too
//------------------------------------------------------------
void TLRender::TRenderNode::SetBoundsInvalid(const TInvalidateFlags& InvalidateFlags)
{
	//	if we're set NOT to invalidate on child changes, dont
	if ( InvalidateFlags( FromChild ) && !GetRenderFlags().IsSet( RenderFlags::InvalidateBoundsByChildren ) )
	{
		//	unless it's forced
		if ( !InvalidateFlags( ForceInvalidateParentsLocalBounds ) )
			return;
	}

	Bool InvWorld = InvalidateFlags(InvalidateWorldBounds);
	Bool InvLocal = InvalidateFlags(InvalidateLocalBounds);
	Bool InvPos = InvalidateFlags(InvalidateWorldPos);

	//	no change
	if ( !InvLocal && !InvWorld && !InvPos )
		return;

	Bool ThisLocalBoundsChanged = FALSE;
	Bool ThisWorldBoundsChanged = FALSE;
	Bool HasSetRenderZoneInvalid = FALSE;


	//	invalidate local bounds
	if ( InvLocal )
	{
		//	if any are valid, then at least must change when we invalidate them
		ThisLocalBoundsChanged |= m_BoundsBox.IsLocalShapeValid() ||
									m_BoundsBox2D.IsLocalShapeValid() ||
									m_BoundsSphere.IsLocalShapeValid() ||
									m_BoundsSphere2D.IsLocalShapeValid();

		//	now just blindly invalidate shapes
		m_BoundsBox.SetLocalShapeInvalid();
		m_BoundsBox2D.SetLocalShapeInvalid();
		m_BoundsSphere.SetLocalShapeInvalid();
		m_BoundsSphere2D.SetLocalShapeInvalid();
	}

	//	invalidating world TRANSFORM...
	if ( InvWorld )
	{
		//	downgrade validation of world shapes, transform and pos only if requested
		ThisWorldBoundsChanged |= SetWorldTransformOld( InvPos, TRUE, TRUE );
	}
	else if ( InvLocal && ThisLocalBoundsChanged )
	{
		//	just invalidating our world SHAPE, not our transform or pos
		ThisWorldBoundsChanged |= SetWorldTransformOld( FALSE, FALSE, TRUE );
	}

	//	invalidate zones if required
	if ( ThisWorldBoundsChanged )
	{
		Debug_PrintInvalidate( this, "local", "all" );

		//	invalidate the zone of our RenderNodeZones - if our world bounds has changed then we
		//	may have moved to a new zone
		if ( !HasSetRenderZoneInvalid && m_RenderZoneNodes.GetSize() )
		{
			for ( u32 z=0;	z<m_RenderZoneNodes.GetSize();	z++ )
				m_RenderZoneNodes.ElementAt(z)->SetZoneOutOfDate();
				
			HasSetRenderZoneInvalid = TRUE;
		}
	}

	//	invalidate world pos
	if ( InvPos )
	{
		if ( m_WorldPosValid == SyncTrue )
		{
			m_WorldPosValid = SyncWait;
			ThisWorldBoundsChanged = TRUE;

			if ( !HasSetRenderZoneInvalid && m_RenderZoneNodes.GetSize() )
			{
				for ( u32 z=0;	z<m_RenderZoneNodes.GetSize();	z++ )
					m_RenderZoneNodes.ElementAt(z)->SetZoneOutOfDate();
				HasSetRenderZoneInvalid = TRUE;
			}
		}
	}

	//	invalidate parent if local changes
	if ( (ThisWorldBoundsChanged&&InvalidateFlags(InvalidateParentLocalBounds)) || InvalidateFlags(ForceInvalidateParentsLocalBounds) )
	{
		TRenderNode* pParent = GetParent();
		if ( pParent )
		{
			//	get parent's invalidate flags
			TInvalidateFlags ParentInvalidateFlags;
			ParentInvalidateFlags.Set( FromChild );
			ParentInvalidateFlags.Set( InvalidateLocalBounds );

			//	gr: unless explicitly set, dont invalidate all of the parent's children
			if ( ParentInvalidateFlags(InvalidateParentsChildrenWorldBounds) )
				ParentInvalidateFlags.Set( InvalidateChildWorldBounds );
			if ( ParentInvalidateFlags(InvalidateParentsChildrenWorldPos) )
				ParentInvalidateFlags.Set( InvalidateChildWorldPos );

			//	invalidate parent
			pParent->SetBoundsInvalid(ParentInvalidateFlags);
		}
	}

	//	invalidate world bounds of children
	if ( HasChildren() && ThisWorldBoundsChanged && (InvalidateFlags(InvalidateChildWorldBounds) || InvalidateFlags(InvalidateChildWorldPos) )  )
	{
		if ( !ThisWorldBoundsChanged )
		{
			//TLDebug_Break("Possible optimisation?");
		}

		//	calculate child's invalidate flags
		TInvalidateFlags ChildInvalidateFlags;
		ChildInvalidateFlags.Set( FromParent );

		if ( InvalidateFlags(InvalidateChildWorldBounds) )
		{
			ChildInvalidateFlags.Set( InvalidateWorldBounds );
			ChildInvalidateFlags.Set( InvalidateChildWorldBounds );
		}

		if ( InvalidateFlags(InvalidateChildWorldPos) )
		{
			ChildInvalidateFlags.Set( InvalidateWorldPos );
			ChildInvalidateFlags.Set( InvalidateChildWorldPos );
		}

#ifdef TLGRAPH_OWN_CHILDREN
		TPtrArray<TLRender::TRenderNode>& NodeChildren = GetChildren();
		for ( u32 c=0;	c<NodeChildren.GetSize();	c++ )
		{
			TLRender::TRenderNode* pChild = NodeChildren[c];
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
const float3& TLRender::TRenderNode::GetWorldPos()
{
	//	validity up to date
	if ( m_WorldPosValid == m_WorldTransformValid )
		return m_WorldPos;

	//	cant calculate
	if ( m_WorldTransformValid == SyncFalse )
	{
		m_WorldPosValid = SyncFalse;
		return m_WorldPos;
	}

	//	calc new world pos

	//	"center" of local node is the base world position
	if ( m_Transform.HasTranslate() )
		m_WorldPos.Set( m_Transform.GetTranslate() );
	else
		m_WorldPos.Set( 0.f, 0.f, 0.f );

	//	transform
	m_WorldTransform.Transform( m_WorldPos );

	//	is as valid as the transform we just applied
	m_WorldPosValid = m_WorldTransformValid;

	return m_WorldPos;
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
}


void TLRender::TRenderNode::OnAdded()
{
	TLGraph::TGraphNode<TLRender::TRenderNode>::OnAdded();
/*	//	gr: printing out too often atm
#ifdef _DEBUG
	TTempString RefString;
	GetRenderNodeRef().GetString( RefString );
	RefString.Append(" added to graph... invalidating...");
	TLDebug_Print( RefString );
#endif
*/
	//	invalidate bounds of self IF child affects bounds
	if ( !GetRenderFlags().IsSet( RenderFlags::ResetScene ) )
	{
		SetBoundsInvalid( TInvalidateFlags( InvalidateLocalBounds, ForceInvalidateParentsLocalBounds ) );
	}
}


void TLRender::TRenderNode::OnMoved(const TPtr<TLRender::TRenderNode>& pOldParent)
{
	TLGraph::TGraphNode<TLRender::TRenderNode>::OnMoved(pOldParent);

	//	invalidate bounds of self IF child affects bounds
	if ( !GetRenderFlags().IsSet( RenderFlags::ResetScene ) )
	{
		SetBoundsInvalid( TInvalidateFlags( InvalidateLocalBounds, ForceInvalidateParentsLocalBounds ) );
	}
}


//---------------------------------------------------------
//	generic render node init
//---------------------------------------------------------
void TLRender::TRenderNode::Initialise(TLMessaging::TMessage& Message)
{
	//	read init data
	/*
#ifdef _DEBUG
		TLDebug_Print("Init message data: ");
		Message.Debug_PrintTree();
#endif
		*/

	TRef SceneNodeRef;

	//	need to subscribe to a scene node - todo: expand to get all children like this
	if ( Message.ImportData("SubTo",SceneNodeRef) )
	{
		TPtr<TLScene::TSceneNode>& pSceneNode = TLScene::g_pScenegraph->FindNode(SceneNodeRef);
		if ( pSceneNode )
		{
			this->SubscribeTo( pSceneNode );
		}
		else
		{
			TLDebug_Break("Node instructed to subscribe to a scene node that doesn't exist");
		}
	}

	//	need to publish to a scene node - todo: expand to get all children like this
	if ( Message.ImportData("PubTo",SceneNodeRef) )
	{
		TPtr<TLScene::TSceneNode>& pSceneNode = TLScene::g_pScenegraph->FindNode(SceneNodeRef);
		if ( pSceneNode )
		{
			pSceneNode->SubscribeTo( this );
		}
		else
		{
			TLDebug_Break("Node instructed to publish to a scene node that doesn't exist");
		}
	}



	if(Message.ImportData("Owner", m_OwnerSceneNode))
	{
		// Get the scenegraph node
		TPtr<TLScene::TSceneNode>& pOwner = TLScene::g_pScenegraph->FindNode(m_OwnerSceneNode);

		if(pOwner.IsValid())
		{
			pOwner->SubscribeTo(this);
			SubscribeTo(pOwner);

			/*
			TPtr<TLMessaging::TEventChannel>& pEventChannel = pOwner->FindEventChannel(TRef_Static(O,n,T,r,a));

			if(pEventChannel)
			{
				// Subscribe to the scene node owners transform channel
				SubscribeTo(pEventChannel);

				// Subscribe the 'scene' node owner to this node so we can sen audio change messages
				pOwner->SubscribeTo(this);
			}
			*/
		}
	}


	u8 TransformChangedBits = m_Transform.ImportData( Message );

	//	transform has been set
	OnTransformChanged(TransformChangedBits);

	Message.ImportData("LineWidth", m_LineWidth );

	if ( Message.ImportData("MeshRef", m_MeshRef ) == SyncTrue )
	{
		//	start loading the asset in case we havent loaded it already
		TLAsset::LoadAsset( m_MeshRef );

		//	mesh ref changed
		OnMeshRefChanged();
	}

	if ( Message.ImportData("TextureRef", m_TextureRef ) == SyncTrue )
	{
		//	start loading the asset in case we havent loaded it already
		TLAsset::LoadAsset( m_TextureRef );

		//	texture ref changed
		OnTextureRefChanged();
	}

	//	get render flags to set
	TPtrArray<TBinaryTree> FlagChildren;
	if ( Message.GetChildren("RFSet", FlagChildren ) )
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
	if ( Message.GetChildren("RFClear", FlagChildren ) )
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
	if ( Message.ImportData("Colour", m_Colour ) )
		OnColourChanged();

	//	import attach datum
	TRef AttachDatum;
	if ( Message.ImportData("Attach", AttachDatum ) )
		SetAttachDatum( AttachDatum );

	//	do inherited init
	TLGraph::TGraphNode<TLRender::TRenderNode>::Initialise( Message );
}

//---------------------------------------------------------
//	no updates for render nodes!
//---------------------------------------------------------
void TLRender::TRenderNode::Update(float Timestep)
{
	TLDebug_Break("Render nodes should not be updated!");
}


//---------------------------------------------------------
//	clean-up any TPtrs back to us so we will be deallocated
//---------------------------------------------------------
void TLRender::TRenderNode::Shutdown()
{
	//	these contain TPtr's back to us, so we need to clear them
	m_RenderZoneNodes.Empty();

	//	inherited cleanup
	TLGraph::TGraphNode<TLRender::TRenderNode>::Shutdown();
}


//---------------------------------------------------------
//	
//---------------------------------------------------------
void TLRender::TRenderNode::ProcessMessage(TLMessaging::TMessage& Message)
{
	//	gr: only apply the change if it comes from our owner scene node
	//	"OnTransform"
	if ( Message.GetMessageRef() == TRef_Static(O,n,T,r,a) && Message.GetSenderRef() == GetOwnerSceneNodeRef() && GetOwnerSceneNodeRef().IsValid() )
	{
		u8 TransformChangedBits = m_Transform.ImportData( Message );
		OnTransformChanged(TransformChangedBits);
		return;
	}
	else if(Message.GetMessageRef() == TRef("SetTransform"))
	{
		//	overwrite our transform
		u8 TransformChangedBits = m_Transform.ImportData( Message );
		OnTransformChanged(TransformChangedBits);

		// Colour property import
		TColour newcol;
		if(Message.ImportData("Colour", newcol))
			SetColour(newcol);

		return;
	}
	else if(Message.GetMessageRef() == TRef("DoTransform"))
	{
		//	read sent transform
		TLMaths::TTransform Transform;
		Transform.ImportData( Message );
		
		//	modify our existing transform by this transform
		u8 TransformChangedBits = m_Transform.TransformHasChanged( Transform );
		OnTransformChanged( TransformChangedBits );
		return;
	}

	//	do inherited init
	TLGraph::TGraphNode<TLRender::TRenderNode>::ProcessMessage( Message );
}


//---------------------------------------------------------
//	set new world transform
//---------------------------------------------------------
void TLRender::TRenderNode::SetWorldTransform(const TLMaths::TTransform& SceneTransform)
{
	//	if old transform is still valid, dont do anything
	if ( m_WorldTransformValid == SyncTrue )
	{
		//	SceneTransform should match our m_WorldTransform
		return;
	}

	//	assume there were changes if any parts are valid
	//	todo: more extensive check in case these transforms haven't changed at all?
	//	should be caught with the invalidation system really
	if ( m_WorldTransform.HasAnyTransform() || SceneTransform.HasAnyTransform() )
	{
		//	downgrade the valid status of our world shapes/datums (to "old") if they were valid...
		SetWorldTransformOld();
	}

	//	store new world transform
	m_WorldTransform = SceneTransform;
	m_WorldTransformValid = SyncTrue;

}


//---------------------------------------------------------
//	downgrade all world shape/transform states from valid to old. returns if anything was downgraded
//---------------------------------------------------------
Bool TLRender::TRenderNode::SetWorldTransformOld(Bool SetPosOld,Bool SetTransformOld,Bool SetShapesOld)
{
	Bool Changed = FALSE;

	if ( SetTransformOld && m_WorldTransformValid == SyncTrue )
	{
		m_WorldTransformValid = SyncWait;
		Changed = TRUE;
	}

	if ( SetPosOld && m_WorldPosValid == SyncTrue )
	{
		m_WorldPosValid = SyncWait;
		Changed = TRUE;
	}

	if ( SetShapesOld )
	{
		//	if any of the bounds shapes WERE up to date, they won't be after this
		Changed |= m_BoundsBox.IsWorldShapeValid() || 
					m_BoundsBox2D.IsWorldShapeValid() || 
					m_BoundsSphere.IsWorldShapeValid() || 
					m_BoundsSphere2D.IsWorldShapeValid();

		m_BoundsBox.SetWorldShapeOld();
		m_BoundsBox2D.SetWorldShapeOld();
		m_BoundsSphere.SetWorldShapeOld();
		m_BoundsSphere2D.SetWorldShapeOld();
	}

	return Changed;
}


//---------------------------------------------------------
//	return the world transform. will explicitly calculate the world transform if out of date. 
//	This is a bit rendundant as it's 
//	calculated via the render but sometimes we need it outside of that. 
//	If WorldTransform is Valid(TRUE) then this is not recalculated. 
//	THe root render node should be provided (but in reality not a neccessity, see trac: http://grahamgrahamreeves.getmyip.com:1984/Trac/wiki/KnownIssues )
//---------------------------------------------------------
const TLMaths::TTransform& TLRender::TRenderNode::GetWorldTransform(TRenderNode* pRootNode,Bool ForceCalculation)
{
	//	doesn't require recalculation
	if ( !ForceCalculation && m_WorldTransformValid == SyncTrue )
		return m_WorldTransform;

	//	get our parent's world transform
	TRenderNode* pParent = GetParent();

	//	no parent, or we are the root, then our transform *is* the world transform...
	if ( !pParent || this == pRootNode )
	{
		this->SetWorldTransform( GetTransform() );
		return m_WorldTransform;
	}

	//	if we don't inherit transforms then stop here - our world transform is the same as our local transform
	if ( GetRenderFlags().IsSet(TLRender::TRenderNode::RenderFlags::ResetScene) )
	{
		SetWorldTransform( GetTransform() );
		return m_WorldTransform;
	}
	
	//	recalculate our parent's world transform
	const TLMaths::TTransform& ParentWorldTransform = pParent->GetWorldTransform( pRootNode );

	//	we can now calculate our transform based on our parent.
	if ( pParent->IsWorldTransformValid() != SyncTrue )
	{
		TLDebug_Break("error - parent couldn't calculate it's world transform... we can't calcualte ours.");
		return m_WorldTransform;
	}

	//	just inherit parent's if no local transform
	if ( !GetTransform().HasAnyTransform() )
	{
		SetWorldTransform( ParentWorldTransform );
	}
	else
	{
		//	get the current scene transform (parent's)...
		TLMaths::TTransform NewWorldTransform = ParentWorldTransform;

		//	...and change it by our tranform
		NewWorldTransform.Transform( GetTransform() );

		//	set new world transform
		SetWorldTransform( NewWorldTransform );
	}

	return m_WorldTransform;
}


//---------------------------------------------------------
//	change the datum we're attached to. Sets the data and does an immediate translate as required
//---------------------------------------------------------
void TLRender::TRenderNode::SetAttachDatum(TRefRef DatumRef)
{
	//	no longer valid, remove from data if it's there
	if ( !DatumRef.IsValid() )
	{
		GetData().RemoveChild("AttachDatum");
		return;
	}

	//	write changes to data 
	TPtr<TBinaryTree>& pData = GetData().GetChild("AttachDatum");
	if ( pData )
	{
		pData->Empty();
		pData->Write( DatumRef );
	}
	else
	{
		GetData().ExportData("AttachDatum", DatumRef );
	}

	//	do an immediate transform if we can
	TLRender::TRenderNode* pParent = GetParent();
	if ( pParent )
	{
		//	get position of datum
		const TLMaths::TShape* pDatum = pParent->GetLocalDatum( DatumRef );
		TLRender::TRenderNode* pDatumParent = pParent;

		//	gr: bodge - look at parent's parent. This is to get around a node in a scheme which uses a parent datum
		//			but the menu system shoves in an intermediate node to hold the sceheme.
		//			we can remove this when we get a nicer scheme manager which doesnt rely on nodes so we can import/remove
		//			schemes a bit easier without having intermediate nodes like this
		//		could make this permanant though? so we cna grab a datum further up the tree (assuming there is no transforming)
		while ( !pDatum )
		{
			pDatumParent = pDatumParent->GetParent();
			if ( !pDatumParent )
				break;
			
			//	don't use if the parent has transform
			//if ( pDatumParent->GetTransform().HasScale() || pDatumParent->GetTransform().HasRotation() )
			//	break;

			pDatum = pDatumParent->GetLocalDatum( DatumRef );
		}

		if ( pDatum )
		{
			SetTranslate( pDatum->GetCenter() );

			//	debug that we used a different parent
			if ( pDatumParent != pParent )
			{
				TTempString Debug_String("Failed to find datum ");
				DatumRef.GetString( Debug_String );
				Debug_String.Append(" to attach ");
				GetNodeRef().GetString( Debug_String );
				Debug_String.Append(" on to parent render node (");
				pParent->GetNodeRef().GetString( Debug_String );
				Debug_String.Append("). Used a grand parent instead: ");
				pDatumParent->GetNodeRef().GetString( Debug_String );
				TLDebug_Print( Debug_String );
			}
		}
		else
		{
			TTempString Debug_String("Failed to find datum ");
			DatumRef.GetString( Debug_String );
			Debug_String.Append(" to attach ");
			GetNodeRef().GetString( Debug_String );
			Debug_String.Append(" on to parent render node (");
			pParent->GetNodeRef().GetString( Debug_String );
			Debug_String.Append("). setup \"async attach to datum\" routine? or just wait for a OnDatumChanged message?");
			TLDebug_Break( Debug_String );
		}
	}
	else
	{
		TLDebug_Break("Set attach datum to a node with no parent...");
	}		
}


//---------------------------------------------------------
//	get the position of a datum in local space. returns FALSE if no such datum
//---------------------------------------------------------
Bool TLRender::TRenderNode::GetLocalDatumPos(TRefRef DatumRef,float3& Position)
{
	//	get datum
	const TLMaths::TShape* pDatum = GetLocalDatum( DatumRef );
	if ( !pDatum )
		return FALSE;

	//	get position
	Position = pDatum->GetCenter();
	return TRUE;
}


//---------------------------------------------------------
//	get the position of a datum in local space. returns FALSE if no such datum. Currently will recalc the world transform if it's out of date
//---------------------------------------------------------
Bool TLRender::TRenderNode::GetWorldDatumPos(TRefRef DatumRef,float3& Position)
{
	//	get local pos of datum first
	if ( !GetLocalDatumPos( DatumRef, Position ) )
		return FALSE;

	//	get the world transform (this will recalc it if out of date)
	const TLMaths::TTransform& WorldTransform = GetWorldTransform();

	//	transform is out of date so cant use it
	if ( IsWorldTransformValid() != SyncTrue )
		return FALSE;

	//	transform the position by world transform
	WorldTransform.Transform( Position );
	
	return TRUE;
}

//---------------------------------------------------------
//	extract a datum  and transform it into a new world space shape
//---------------------------------------------------------
TPtr<TLMaths::TShape> TLRender::TRenderNode::GetWorldDatum(TRefRef DatumRef)
{
	//	get local pos of datum first
	const TLMaths::TShape* pLocalDatum = GetLocalDatum( DatumRef );				//	extract a datum from our mesh - unless a special ref is used to get bounds shapes
	if ( !pLocalDatum )
		return NULL;

	//	get the world transform (this will recalc it if out of date)
	const TLMaths::TTransform& WorldTransform = GetWorldTransform();

	//	transform is out of date so cant use it
	if ( IsWorldTransformValid() != SyncTrue )
		return NULL;

	//	transform the datum by world transform into a new datum shape
	TPtr<TLMaths::TShape> pNewShape = pLocalDatum->Transform( WorldTransform, TLPtr::GetNullPtr<TLMaths::TShape>() );
	return pNewShape;
}