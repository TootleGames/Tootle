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
		TLAsset::TMesh* pMesh = pRenderNode->GetMeshAsset();
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
	m_LineWidth					( 0.f ),
	m_PointSpriteSize			( 1.f ),
	m_WorldPosValid				( SyncFalse ),
	m_Colour					( 1.f, 1.f, 1.f, 1.f ),
	m_WorldTransformValid		( SyncFalse ),
	m_AttachDatumValid			( TRUE ),
	m_pMeshCache				( NULL ),
	m_pTextureCache				( NULL )
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
	m_BoundsBox		= OtherRenderNode.m_BoundsBox;
	m_BoundsBox2D		= OtherRenderNode.m_BoundsBox2D;
	m_BoundsSphere		= OtherRenderNode.m_BoundsSphere;
	m_BoundsSphere2D		= OtherRenderNode.m_BoundsSphere2D;
	m_RenderFlags			= OtherRenderNode.m_RenderFlags;
	m_MeshRef				= OtherRenderNode.m_MeshRef;
	m_Data					= OtherRenderNode.m_Data;

	SetRenderNodeRef( OtherRenderNode.GetRenderNodeRef() );
	*/
}

//------------------------------------------------------------
//	default behaviour fetches the mesh from the asset lib with our mesh ref
//------------------------------------------------------------
TLAsset::TMesh* TLRender::TRenderNode::GetMeshAsset()
{
	//	re-fetch mesh if we need to
	if ( GetMeshRef().IsValid() && !m_pMeshCache )
	{
		m_pMeshCache = TLAsset::GetAssetPtr<TLAsset::TMesh>( GetMeshRef() ).GetObjectPointer();
	}

	return m_pMeshCache;
}

//------------------------------------------------------------
//	default behaviour fetches the mesh from the asset lib with our mesh ref
//------------------------------------------------------------
TLAsset::TTexture* TLRender::TRenderNode::GetTextureAsset()
{
	//	re-fetch mesh if we need to
	if ( GetTextureRef().IsValid() && !m_pTextureCache )
	{
		m_pTextureCache = TLAsset::GetAssetPtr<TLAsset::TTexture>( GetTextureRef() ).GetObjectPointer();
	}

	return m_pTextureCache;
}


//------------------------------------------------------------
//	pre-draw routine for a render object
//------------------------------------------------------------
Bool TLRender::TRenderNode::Draw(TRenderTarget* pRenderTarget,TRenderNode* pParent,TPtrArray<TRenderNode>& PostRenderList)
{
	//	if attach datum needs updating... do it
	if ( !m_AttachDatumValid )
		OnAttachDatumChanged();

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
	Bool ThisWorldBoundsChanged = FALSE;

	//	do self changes
	if ( InvLocal || InvWorld || InvPos )
	{
		Bool ThisLocalBoundsChanged = FALSE;
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
	}

	//	invalidate world bounds of children
	if ( HasChildren() )
	{
		//	if we have changed, or we force a child-invalidation
		//	AND the invalidation of children was requested, then do child invalidation
		//	ThisWorldBoundsChanged check is an optimisation so we don't invalidate children when nothign has changed
		if ( ( ThisWorldBoundsChanged || InvalidateFlags(ForceInvalidateChildren) ) && (InvalidateFlags(InvalidateChildWorldBounds) || InvalidateFlags(InvalidateChildWorldPos) ) )
		{
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

			TPtrArray<TLRender::TRenderNode>& NodeChildren = GetChildren();
			for ( u32 c=0;	c<NodeChildren.GetSize();	c++ )
			{
				TLRender::TRenderNode* pChild = NodeChildren[c];
				pChild->SetBoundsInvalid( ChildInvalidateFlags );
			}
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

	//	create children during init
	TPtrArray<TBinaryTree> InitChildDatas;
	if ( Message.GetChildren("Child", InitChildDatas ) )
		for ( u32 c=0;	c<InitChildDatas.GetSize();	c++ )
			CreateChildNode( *InitChildDatas[c] );
	
	//	do inherited init
	TLGraph::TGraphNode<TLRender::TRenderNode>::Initialise( Message );
}


//---------------------------------------------------------
//	create a child node from plain data
//---------------------------------------------------------
Bool TLRender::TRenderNode::CreateChildNode(TBinaryTree& ChildInitData)
{
	/*
	TTempString Debug_String("Creating child node from RenderNode ");
	this->GetNodeRef().GetString( Debug_String );
	TLDebug_Print( Debug_String );
	ChildInitData.Debug_PrintTree();
	*/

	//	import bits of optional data
	TRef Type;
	ChildInitData.ImportData("Type", Type);

	TRef Parent = this->GetNodeRef();
	if ( ChildInitData.ImportData("Parent", Parent) )
	{
		if ( Parent != this->GetNodeRef() )
		{
			TTempString Debug_String("Import \"child\" data for render node ");
			this->GetNodeRef().GetString( Debug_String );
			Debug_String.Append(", and child's parent is ");
			Parent.GetString( Debug_String );
			TLDebug_Break(Debug_String);
		}
	}

	TRef ChildRef;
	ChildInitData.ImportData("NodeRef", ChildRef);

	//	make up an initialise message and use the data provided
	TLMessaging::TMessage InitMessage(TLCore::InitialiseRef);
	InitMessage.ReferenceDataTree( ChildInitData );

	ChildRef = TLRender::g_pRendergraph->CreateNode( ChildRef, Type, Parent, &InitMessage );
	
	return ChildRef.IsValid();
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

	m_pMeshCache = NULL;
	m_pTextureCache = NULL;

	//	inherited cleanup
	TLGraph::TGraphNode<TLRender::TRenderNode>::Shutdown();
}


//---------------------------------------------------------
//	
//---------------------------------------------------------
void TLRender::TRenderNode::ProcessMessage(TLMessaging::TMessage& Message)
{
	//	catch the change of transform from our owner scene node, and then copy it
	//	"OnTransform"
	if ( Message.GetMessageRef() == TRef_Static(O,n,T,r,a) && Message.GetSenderRef() == GetOwnerSceneNodeRef() && GetOwnerSceneNodeRef().IsValid() )
	{
		u8 TransformChangedBits = m_Transform.ImportData( Message );
		OnTransformChanged(TransformChangedBits);
		return;
	}
	else if(Message.GetMessageRef() == TRef_Static(S,e,t,T,r) )
	{
		//	overwrite our transform
		u8 TransformChangedBits = m_Transform.ImportData( Message );
		OnTransformChanged(TransformChangedBits);

		return;
	}
	else if(Message.GetMessageRef() == TRef_Static(L,o,c,T,r))	//	local transform
	{
		//	read sent transform
		TLMaths::TTransform Transform;
		Transform.ImportData( Message );
		
		//	modify our existing transform by this transform
		//	gr: this takes Transform, localises the changes (eg. rotate and scale the translate) and then sets the values. 
		//	This is kinda okay for rotations and scales, but wrong for translations. This is like a Matrix multiply
		u8 TransformChangedBits = m_Transform.Transform_HasChanged( Transform );
		OnTransformChanged( TransformChangedBits );
		return;
	}
	else if(Message.GetMessageRef() == TRef_Static(D,o,T,r,a))
	{
		//	read sent transform
		TLMaths::TTransform Transform;
		Transform.ImportData( Message );
		
		//	modify our existing transform by this transform
		u8 TransformChangedBits = m_Transform.AddTransform_HasChanged( Transform );
		OnTransformChanged( TransformChangedBits );
		return;
	}

	//	do inherited init
	TLGraph::TGraphNode<TLRender::TRenderNode>::ProcessMessage( Message );
}


//---------------------------------------------------------
//	SetProperty message - made into virtual func as it's will be commonly used.
//---------------------------------------------------------
void TLRender::TRenderNode::SetProperty(TLMessaging::TMessage& Message)
{
	//	read new transform (acts like "SetTransform")
	u8 TransformChangedBits = m_Transform.ImportData( Message );
	if ( TransformChangedBits )
		OnTransformChanged(TransformChangedBits);

	//	line width
	Message.ImportData("LineWidth", m_LineWidth );

	//	point sprite size
	Message.ImportData("PointSize", m_PointSpriteSize );

	//	mesh
	if ( Message.ImportData("MeshRef", m_MeshRef ) )
	{
		//	start loading the asset in case we havent loaded it already
		if ( m_MeshRef.IsValid() )
			TLAsset::LoadAsset( m_MeshRef, TRef_Static4(M,e,s,h), FALSE );

		//	mesh ref changed
		OnMeshRefChanged();
	}

	//	texture
	if ( Message.ImportData("TextureRef", m_TextureRef ) )
	{
		//	start loading the asset in case we havent loaded it already
		if ( m_TextureRef.IsValid() )
			TLAsset::LoadAsset( m_TextureRef, TRef_Static(T,e,x,t,u), FALSE );

		//	texture ref changed
		OnTextureRefChanged();
	}

	//	enable state
	Bool Enabled = TRUE;
	if ( Message.ImportData("Enabled", Enabled ) )
	{
		//	set/unset flag
		SetEnabled( Enabled );
	}

	u32 RenderFlags = 0;
	if(Message.ImportData("RFlags", RenderFlags))
	{
		// Set the flags as a raw value.  Saves cycling through a load of 
		// flag indices but should generally only be used for exporting and re-import
		m_RenderFlags.SetData(RenderFlags);
	}
	else
	{
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
	}

	//	import colour
	if ( Message.ImportData("Colour", m_Colour ) )
		OnColourChanged();

	//	set attach datum
	TRef AttachDatum;
	if ( Message.ImportData("AttachDatum", AttachDatum ) )
		SetAttachDatum( AttachDatum );

	//	get list of datums to debug-render
	TPtrArray<TBinaryTree> DbgDatumDatas;
	if ( Message.GetChildren("DbgDatum", DbgDatumDatas ) )
	{
		for ( u32 c=0;	c<DbgDatumDatas.GetSize();	c++ )
		{
			TRef DatumRef;
			DbgDatumDatas[c]->ResetReadPos();
			if ( DbgDatumDatas[c]->Read(DatumRef) )
				m_Debug_RenderDatums.AddUnique( DatumRef );
		}
	}

	//	get list of datums to remove from the debug-render list
	DbgDatumDatas.Empty();
	if ( Message.GetChildren("RmDbgDatum", DbgDatumDatas ) )
	{
		for ( u32 c=0;	c<DbgDatumDatas.GetSize();	c++ )
		{
			TRef DatumRef;
			DbgDatumDatas[c]->ResetReadPos();
			if ( DbgDatumDatas[c]->Read(DatumRef) )
				m_Debug_RenderDatums.Remove( DatumRef );
		}
	}

	// Super SetProperty call
	TLGraph::TGraphNode<TLRender::TRenderNode>::SetProperty(Message);
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
		#ifdef _DEBUG
		if ( m_WorldTransform != SceneTransform )
		{
			TTempString Debug_String("World transform of render node ");
			this->GetRenderNodeRef().GetString( Debug_String );
			Debug_String.Append(" doesn't match it's scene transform... but render node thinks its up to date... Invalidation missing somewhere");
			TLDebug_Break(Debug_String);
		}
		#endif
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


void TLRender::TRenderNode::UpdateNodeData()
{
	TLRender::TRenderNode* pParent = GetParent();
	if(pParent)
	{
		GetNodeData().RemoveChild("Parent");
		GetNodeData().ExportData("Parent", pParent->GetNodeRef());
	}

	//	gr: i wonder if the type should be stored in itself, as (for editor-style reflection at least) it cannot be changed
	GetNodeData().RemoveChild("Type");
	GetNodeData().ExportData("Type", GetNodeTypeRef());

	GetNodeData().RemoveChild("MeshRef");
	GetNodeData().ExportData("MeshRef", m_MeshRef);

	// Export transform info
	GetNodeData().RemoveChild(TRef_Static(T,r,a,n,s));
	GetNodeData().RemoveChild(TRef_Static(S,c,a,l,e));
	GetNodeData().RemoveChild(TRef_Static(R,o,t,a,t));
	m_Transform.ExportData(GetNodeData());

	GetNodeData().RemoveChild("Texture");
	GetNodeData().ExportData("Texture", m_TextureRef);

	GetNodeData().RemoveChild("Colour");
	GetNodeData().ExportData("Colour", m_Colour);

	GetNodeData().RemoveChild("AttachDatum");

	TRef AttachDatum = GetAttachDatum();
	if(AttachDatum.IsValid())
		GetNodeData().ExportData("AttachDatum", AttachDatum);

	GetNodeData().RemoveChild("LineWidth");
	GetNodeData().ExportData("LineWidth", m_LineWidth );

	//	point sprite size
	GetNodeData().RemoveChild("PointSize");
	GetNodeData().ExportData("PointSize", m_PointSpriteSize );

	// Export renderflags as a single value rathe than individual bit's
	//NOTE: No need to do the 'enable' as that is stored in the render flags
	GetNodeData().RemoveChild("RFlags");
	GetNodeData().ExportData("RFlags", m_RenderFlags.GetData());
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
	const TLMaths::TTransform& ParentWorldTransform = pParent->GetChildWorldTransform( pRootNode, ForceCalculation );

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
		GetNodeData().RemoveChild("AttachDatum");
		return;
	}

	//	write changes to data
	Bool DatumChanged = GetNodeData().ReplaceData("AttachDatum", DatumRef );

	if ( DatumChanged )
	{
		m_AttachDatumValid = FALSE;

		//	try an immediate transform
		OnAttachDatumChanged();
	}
}


//---------------------------------------------------------
//	re-align ourselves with our attach datum if we can
//---------------------------------------------------------
void TLRender::TRenderNode::OnAttachDatumChanged()
{
	//	mark as invalid - todo change this so all callers invalidate it
	m_AttachDatumValid = FALSE;

	//	do an immediate transform if we can
	TLRender::TRenderNode* pParent = GetParent();
	if ( !pParent )
	{
		TLDebug_Break("Trying to set/place attach datum to a node with no parent...");
		m_AttachDatumValid = TRUE;
		return;
	}		

	//	get the datum ref
	TRef DatumRef;
	if ( !GetNodeData().ImportData("AttachDatum", DatumRef ) )
	{
		//	no attach datum set, mark postiion as valid
		m_AttachDatumValid = TRUE;
		return;
	}

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
			
		pDatum = pDatumParent->GetLocalDatum( DatumRef );
	}

	if ( !pDatum )
	{
		TTempString Debug_String("Failed to find datum ");
		DatumRef.GetString( Debug_String );
		Debug_String.Append(" to attach ");
		GetNodeRef().GetString( Debug_String );
		Debug_String.Append(" on to parent render node (");
		pParent->GetNodeRef().GetString( Debug_String );
		Debug_String.Append("). Will try again...");
		TLDebug_Print( Debug_String );
		return;
	}

	//	position to datum's center
	SetTranslate( pDatum->GetCenter() );

	//	position is now up to date
	m_AttachDatumValid = TRUE;

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
Bool TLRender::TRenderNode::GetWorldDatumPos(TRefRef DatumRef,float3& Position,Bool KeepShape,Bool ForceCalc)
{
	//	get local pos of datum first
	if ( !GetLocalDatumPos( DatumRef, Position ) )
		return FALSE;

	//	get the world transform (this will recalc it if out of date)
	const TLMaths::TTransform& WorldTransform = GetWorldTransform( NULL, ForceCalc );

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
TPtr<TLMaths::TShape> TLRender::TRenderNode::GetWorldDatum(TRefRef DatumRef,Bool KeepShape,Bool ForceCalc)
{
	//	get local pos of datum first
	const TLMaths::TShape* pLocalDatum = GetLocalDatum( DatumRef );				//	extract a datum from our mesh - unless a special ref is used to get bounds shapes
	if ( !pLocalDatum )
		return NULL;

	//	get the world transform (this will recalc it if out of date)
	const TLMaths::TTransform& WorldTransform = GetWorldTransform( NULL, ForceCalc );

	//	transform is out of date so cant use it
	if ( IsWorldTransformValid() != SyncTrue )
		return NULL;

	//	transform the datum by world transform into a new datum shape
	//	gr: in order to keep a bounds box as a box - for efficiency - we can keep the shape type
	TPtr<TLMaths::TShape> pNewShape = pLocalDatum->Transform( WorldTransform, TLPtr::GetNullPtr<TLMaths::TShape>(), KeepShape );
	return pNewShape;
}


//---------------------------------------------------------------
//	extract a datum from our mesh - unless a special ref is used to get bounds shapes
//---------------------------------------------------------------
const TLMaths::TShape* TLRender::TRenderNode::GetLocalDatum(TRefRef DatumRef)
{
	switch ( DatumRef.GetData() )
	{
		case TRef_InvalidValue:							return NULL;
		case TLRender_TRenderNode_DatumBoundsBox:		return &GetLocalBoundsBox();
		case TLRender_TRenderNode_DatumBoundsBox2D:		return &GetLocalBoundsBox2D();
		case TLRender_TRenderNode_DatumBoundsSphere:	return &GetLocalBoundsSphere();
		case TLRender_TRenderNode_DatumBoundsSphere2D:	return &GetLocalBoundsSphere2D();

		default:
		{
			//	get datum from mesh
			TLAsset::TMesh* pMesh = GetMeshAsset();
			if ( !pMesh )
				return NULL;
			
			return pMesh->GetDatum( DatumRef );
		}
	};
}


//---------------------------------------------------------------
//	get all datums in the mesh and render node (ie. includes bounds)
//---------------------------------------------------------------
void TLRender::TRenderNode::GetLocalDatums(TArray<const TLMaths::TShape*>& LocalDatums)
{
	LocalDatums.Add( &GetLocalBoundsBox() );
	LocalDatums.Add( &GetLocalBoundsBox2D() );
	LocalDatums.Add( &GetLocalBoundsSphere() );
	LocalDatums.Add( &GetLocalBoundsSphere2D() );
		
	//	add all the datums in the mesh
	TLAsset::TMesh* pMesh = GetMeshAsset();
	if ( pMesh )
	{
		const TPtrKeyArray<TRef,TLMaths::TShape>& MeshDatums = pMesh->GetDatums();
		for ( u32 i=0;	i<MeshDatums.GetSize();	i++ )
			LocalDatums.Add( MeshDatums.GetItemAt(i) );
	}
}


//---------------------------------------------------------------
//	get all datums in the mesh and render node (ie. includes bounds) in world space. Use very sparingly! (ie. debug only)
//---------------------------------------------------------------
void TLRender::TRenderNode::GetWorldDatums(TPtrArray<TLMaths::TShape>& WorldDatums,Bool KeepShape,Bool ForceCalc)
{
	//	get the world transform (this will recalc it if out of date)
	const TLMaths::TTransform& WorldTransform = GetWorldTransform( NULL, ForceCalc );

	//	transform is out of date so cant use it
	if ( IsWorldTransformValid() != SyncTrue )
		return;

	//	get all the local datums
	TArray<const TLMaths::TShape*> LocalDatums;
	GetLocalDatums( LocalDatums );

	for ( u32 i=0;	i<LocalDatums.GetSize();	i++ )
	{
		//	transform the datum by world transform into a new datum shape
		//	gr: in order to keep a bounds box as a box - for efficiency - we can keep the shape type
		TPtr<TLMaths::TShape> pNewShape = LocalDatums[i]->Transform( WorldTransform, TLPtr::GetNullPtr<TLMaths::TShape>(), KeepShape );
		if ( pNewShape )
			WorldDatums.Add( pNewShape );
	}
}


//---------------------------------------------------------------
//	calculate a local-relative position from a world position. this will be in THIS's space. 
//	so if this has a scale of 10, the relative position will be 1/10th of the world's offset. 
//	The end result, multiplied by this's world transform should produce the original world pos
//---------------------------------------------------------------
Bool TLRender::TRenderNode::GetLocalPos(float3& LocalPos,const float3& WorldPos)
{
	const TLMaths::TTransform& WorldTransform = GetWorldTransform();
//	TLMaths::TTransform Untransform = WorldTransform;
//Untransform.Invert();
	LocalPos = WorldPos;
	WorldTransform.Untransform( LocalPos );
//	Untransform.Untransform( LocalPos );
	return TRUE;
}


