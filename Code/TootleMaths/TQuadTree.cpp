#include "TQuadTree.h"



//	gr: currently disabled as there are some ptr issues
//#define ENABLE_CULL_EMPTY_ZONES


namespace TLMaths
{
	namespace TLQuadTree
	{
		FORCEINLINE TLArray::SortResult	SortNodes(const TPtr<TLMaths::TQuadTreeNode>& a,const TPtr<TLMaths::TQuadTreeNode>& b,const void* pTestVal);

		TZonePos		GetMirrorPosition_Vertical(TZonePos Position);
		TZonePos		GetMirrorPosition_Horizontal(TZonePos Position);

		TRef			g_NextQuadTreeNodeRef;	//	unique ref for tree nodes which is automaticcaly incremented
	}
}



//	node sorting
FORCEINLINE TLArray::SortResult	TLMaths::TLQuadTree::SortNodes(const TPtr<TLMaths::TQuadTreeNode>& a,const TPtr<TLMaths::TQuadTreeNode>& b,const void* pTestVal)
{
	//	cast test val
	const TPtr<TLMaths::TQuadTreeNode>* ppTestNode = (const TPtr<TLMaths::TQuadTreeNode>*)pTestVal;

	//	if pTestVal provided it's the ref
	TRefRef aRef = a->GetQuadTreeNodeRef();
	TRefRef bRef = pTestVal ? (*ppTestNode)->GetQuadTreeNodeRef() : b->GetQuadTreeNodeRef();
		
	//	== turns into 0 (is greater) or 1(equals)
	return aRef < bRef ? TLArray::IsLess : (TLArray::SortResult)(aRef==bRef);	
}





TLMaths::TQuadTreeNode::TQuadTreeNode() : 
//	m_IsZoneOutofDate		( TRUE ),
	m_QuadTreeNodeRef		( TLMaths::TLQuadTree::g_NextQuadTreeNodeRef.Increment() )
{
}

	
//-------------------------------------------------------------
//	if the node has moved, update it's zone. returns TRUE if zone changed
//-------------------------------------------------------------
void TLMaths::TQuadTreeNode::UpdateZone(TPtr<TLMaths::TQuadTreeNode> pThis,TPtr<TLMaths::TQuadTreeZone>& pRootZone)
{
	if ( !IsZoneOutOfDate() )
	{
		TLDebug_Print("Unneccesary zone update on node?");
	}

	//	gr: moved to start - assume we will change our zone as appropriately
	//	in our new zone (or Null zone)
	m_IsZoneOutofDate = FALSE;

//#define ENABLE_FAST_CHECK
//#define ENABLE_FAST_GOTO_SMALLER_ZONE


#ifdef ENABLE_FAST_CHECK
	//	are we still in our parents zone?
	TQuadTreeZone* pCurrentZone = pThis->GetZone();
	if ( pCurrentZone )
	{
		if ( pCurrentZone->IsNodeInZoneShape( pThis ) )
		{
			//	yes... see if we've crossed over to one of our parent's sibling zones
			if ( pCurrentZone->HasSiblingZones() )
			{
				Bool InSibling0 = pCurrentZone->GetSiblingZone(0)->IsNodeInZoneShape( pThis ) == SyncTrue;
				Bool InSibling1 = pCurrentZone->GetSiblingZone(1)->IsNodeInZoneShape( pThis ) == SyncTrue;
				Bool InSibling2 = pCurrentZone->GetSiblingZone(2)->IsNodeInZoneShape( pThis ) == SyncTrue;

				//	crossed over so we're now in multiple zones... need to move UP a level
				if ( InSibling0 || InSibling1 || InSibling2 )
				{
					//	make list of zones we're intersecting in of the current zone's parent
					TFixedArray<u32,4> ChildZoneList(0);
					ChildZoneList.Add( pCurrentZone->GetParentsChildIndex() );
					if ( InSibling0 )	ChildZoneList.Add( pCurrentZone->GetSiblingParentsChildIndex(0) );
					if ( InSibling1 )	ChildZoneList.Add( pCurrentZone->GetSiblingParentsChildIndex(1) );
					if ( InSibling2 )	ChildZoneList.Add( pCurrentZone->GetSiblingParentsChildIndex(2) );
	
					SetZone( pCurrentZone->GetParentZone(), pThis, &ChildZoneList );
					return;
				}
			}

			//	not in a sibling, see if we can go down to a smaller zone
#ifdef ENABLE_FAST_GOTO_SMALLER_ZONE
			pCurrentZone->AddNode( pThis, pThis->GetZone(), FALSE );
			return;
#endif
		}
		else
		{
			//	no longer in current zone
			//	keep going up the tree till we find a zone we're in... then go down
			//	...
		}
	}
#endif

	//	simple brute force mode
	TPtr<TQuadTreeZone> pParentZone = pRootZone;

	//	re-add to parent to evaluate if we now span multiple zones
	if ( pParentZone )
	{
		while ( !pParentZone->AddNode( pThis, pParentZone, TRUE ) )
		{
			//	no longer in parent zone, try parent of parent
			pParentZone = pParentZone->GetParentZone();
			if ( !pParentZone )
			{
				//	not in ANY zone any more
				SetZone( pParentZone, pThis, NULL );
				break;
			}
		}
	}

}

//-------------------------------------------------------------
//	
//-------------------------------------------------------------
void TLMaths::TQuadTreeNode::SetChildZones(const TFixedArray<u32,4>& InZones)
{
	if ( !InZones.GetSize() )
	{
		m_ChildZones.Empty();
		return;
	}

	if ( !m_pZone )
	{
		TLDebug_Break("Should be in a zone when trying to assign children");
		return;
	}

	//	add child zones
	m_ChildZones = InZones;
}



//-------------------------------------------------------------
//	attempt to add this node to this zone. checks with children 
//	first to see if it fits into just one child better. returns FALSE if not in this zone
//-------------------------------------------------------------
Bool TLMaths::TQuadTreeNode::SetZone(TPtr<TQuadTreeZone>& pZone,TPtr<TLMaths::TQuadTreeNode>& pThis,const TFixedArray<u32,4>* pChildZoneList)
{
	//	already in this zone
	if ( GetZone() == pZone )
	{
		//	just update child list
		if ( !pChildZoneList )
			SetChildZonesNone();
		else
			SetChildZones( *pChildZoneList );

		return TRUE;
	}

	//	remove from old zone
	TPtr<TQuadTreeZone> pOldZone = GetZone();
	if ( pOldZone )
	{
		pOldZone->DoRemoveNode( pThis );
	}

	//	add to this zone
	if ( pZone )
	{
		if ( pZone->GetNodes().Exists( pThis ) )
		{
			TLDebug_Break("Node shouldnt be in this list");
		}
		else
		{
			//	add node to collision zone
			pZone->DoAddNode( pThis );
		}
	}

	//	set new zone on node
	m_pZone = pZone;
	m_IsZoneOutofDate = FALSE;

	//	update child list
	if ( !pChildZoneList )
		SetChildZonesNone();
	else
		SetChildZones( *pChildZoneList );

	//	notify change of zone
	OnZoneChanged( pOldZone );

	return TRUE;
}



//-------------------------------------------------------------
//	do your object's test to see if it intersects at all with this 
//	zone's shape, default does shape/shape but you might want something more complex
//-------------------------------------------------------------
SyncBool TLMaths::TQuadTreeNode::IsInZone(const TQuadTreeZone& Zone)
{
	return IsInShape( Zone.GetShape() );
}


//-------------------------------------------------------------
//	do your object's test to see if it intersects at all with this 
//	zone's shape, default does shape/shape but you might want something more complex
//-------------------------------------------------------------
SyncBool TLMaths::TQuadTreeNode::IsInShape(const TLMaths::TBox2D& Shape)
{
	const TLMaths::TBox2D& NodeShape = GetZoneShape();

	if ( !NodeShape.IsValid() )
		return SyncFalse;

	return Shape.GetIntersection( NodeShape ) ? SyncTrue : SyncFalse;
}

	
//-------------------------------------------------------------
//	get the shape of this node
//-------------------------------------------------------------
const TLMaths::TBox2D& TLMaths::TQuadTreeNode::GetZoneShape()
{
	TLDebug_Break("GetZoneShape or IsInZone() must be overloaded for this type");
	static TLMaths::TBox2D g_DummyBox;
	g_DummyBox.SetInvalid();
	return g_DummyBox;
}











TLMaths::TQuadTreeZone::TQuadTreeZone(const TLMaths::TBox2D& ZoneShape,const TLMaths::TQuadTreeParams& ZoneParams) :
	m_Shape					( ZoneShape ),
	m_Nodes					( &TLMaths::TLQuadTree::SortNodes ),
	m_SiblingZoneIndexes	( 0 ),
	m_SiblingIndex			( -1 ),
	m_ZoneParams			( ZoneParams ),
	m_Active				( SyncTrue )
{
}


TLMaths::TQuadTreeZone::TQuadTreeZone(const TLMaths::TBox2D& ZoneShape,TPtr<TLMaths::TQuadTreeZone>& pParent) :
	m_pParent	( pParent ),
	m_Shape		( ZoneShape ),
	m_Nodes		( &TLMaths::TLQuadTree::SortNodes ),
	m_SiblingZoneIndexes	( 0 ),
	m_SiblingIndex			( -1 )
{
	if ( !pParent )
	{
		TLDebug_Break("Wrong constructor for Zone. If root use the other one, if not, parent missing.");
		return;
	}

	//	copy parent's params
	m_ZoneParams = pParent->m_ZoneParams;
}



//-------------------------------------------------------------
//	because of backwards referencing, we should shutdown before
//	NULL'ing otherwise nothing will get released until graphs/nodes etc are.
//-------------------------------------------------------------
void TLMaths::TQuadTreeZone::Shutdown()
{
	//	clear parent - might need to remove too...
	m_pParent = NULL;

	//	shutdown children
	for ( u32 i=0;	i<m_Children.GetSize();	i++ )
	{
		m_Children[i]->Shutdown();
	}

	//	now null everything
	m_Children.Empty();
	m_ChildrenWithNodes.Empty();
	m_ChildrenWithNonStaticNodes.Empty();

	m_Nodes.Empty();
	m_NonStaticNodes.Empty();
}


//-------------------------------------------------------------
//	test to see if this intersects into this zone
//-------------------------------------------------------------
SyncBool TLMaths::TQuadTreeZone::IsNodeInZoneShape(TLMaths::TQuadTreeNode* pNode)
{
	return pNode->IsInZone( *this );
}



//-------------------------------------------------------------
//	attempt to add this node to this zone. checks with children first 
//	to see if it fits into just one child better. returns FALSE if not in this zone
//-------------------------------------------------------------
Bool TLMaths::TQuadTreeZone::AddNode(TPtr<TLMaths::TQuadTreeNode>& pNode,TPtr<TLMaths::TQuadTreeZone>& pThis,Bool DoCheckInShape)
{
	//	if not in this shape, return
	if ( DoCheckInShape )
	{
		SyncBool IsInShape = IsNodeInZoneShape( pNode );
		if ( IsInShape == SyncWait )
		{
			TLDebug_Break("todo: handle this");
			return FALSE;
		}
		else if ( IsInShape == SyncFalse )
		{
			return FALSE;
		}
	}

	u32 NewSize = m_Nodes.GetSize() + (m_Nodes.Exists( pNode ) ? 0 : 1);

	//	fits in this zone, if we have X children already, we need to split
	//	gr: changed this from >= to >
	if ( NewSize > m_ZoneParams.m_MaxNodesPerZone && m_Children.GetSize() == 0 )
	{
		if ( Divide(pThis) )
		{
			//	if we process pNode in this code then we dont need to do it later
			Bool DoneNode = FALSE;

			//	re-evaluate existing nodes to see if they fit better in a child
			for ( s32 n=m_Nodes.GetSize()-1;	n>=0;	n-- )
			{
				TPtr<TQuadTreeNode> pChildNode = m_Nodes[n];
			//	TPtr<TQuadTreeNode>& pChildNode = m_Nodes[n];
				DoneNode |= (pNode == pChildNode);

				//	see if child node is now in one of the new zones
				TFixedArray<u32,4> InZones(0);
				GetInChildZones( pChildNode.GetObject(), InZones );

				//	node is not in any of these zones... error...
				if ( InZones.GetSize() == 0 )
				{
					//	gr: this can now occur if the node's shape cannot be determined at the moment
					//		we should mark the node to say zone is out of date, but only need to check DOWN the tree					pChildNode->SetChildZonesNone();
					continue;
				}

				//	in multiple zones, so stay in this zone, but assign the child zones
				if ( InZones.GetSize() > 1 )
				{
					pChildNode->SetChildZones( InZones );
					continue;
				}

				//	only in 1 child zone, so move out of this zone add into this new child zone
				TPtr<TQuadTreeZone>& pNewZone = m_Children[ InZones[0] ];
				pChildNode->SetChildZonesNone();
				pNewZone->AddNode( pChildNode, pNewZone, FALSE );
			}

			//	moved pNode around, don't need to do it again below
			if ( DoneNode )
				return TRUE;
		}
	}

	//	no child zones, so just add to this zone
	if ( !m_Children.GetSize() )
	{
		return pNode->SetZone( pThis, pNode, NULL );
	}

	//	loop through the child zones to see if it fits into 1 or multiple zones
	TFixedArray<u32,4> InZones(0);
	GetInChildZones( pNode.GetObject(), InZones );

	//	node is not in any of these zones... error... 
	//	or
	//	in multiple zones, so stay in this zone, but assign the child zones
	if ( InZones.GetSize() != 1 )
		return pNode->SetZone( pThis, pNode, &InZones );

	//	only in one child zone, add to this child zone (will go through same process, split child as neccessary etc)
	TPtr<TQuadTreeZone>& pNewZone = m_Children[ InZones[0] ];
	Bool AddedToChild = pNewZone->AddNode( pNode, pNewZone, FALSE );
	if ( !AddedToChild )
	{
		TLDebug_Break("Error: inside zone, inside child zone, but failed to add to child zone...");
		return pNode->SetZone( pThis, pNode, &InZones );
	}

	return TRUE;
}


//---------------------------------------------------------------------
//	return which child zone we're in, -1 if none
//---------------------------------------------------------------------
void TLMaths::TQuadTreeZone::GetInChildZones(TQuadTreeNode* pNode,TFixedArray<u32,4>& InZones)
{
	//	loop through the child zones to see if it fits into 1 or multiple zones
	for ( u32 c=0;	c<m_Children.GetSize();	c++ )
	{
		TPtr<TQuadTreeZone>& pChildZone = m_Children[c];
		SyncBool IsInShape = pChildZone->IsNodeInZoneShape( pNode );

		if ( IsInShape == SyncTrue )
		{
			//	add to list of zones we're in
			InZones.Add( c );
		}
		else if ( IsInShape == SyncWait )
		{
			//	cant tell with this node at the moment...
			//	todo: mark it to only check to see if it needs moving DOWN the tree
			pNode->SetZoneOutOfDate();
			break;
		}
	}
}


//-----------------------------------------------------------------
//	remove node from this zone's list
//-----------------------------------------------------------------
void TLMaths::TQuadTreeZone::DoRemoveNode(TPtr<TQuadTreeNode>& pNode)
{
#ifdef _DEBUG
	if ( !m_Nodes.Exists( pNode ) )
	{
		TLDebug_Break("Node missing from zone");
		return;
	}
#endif

	//	remove node from our list
	m_Nodes.Remove( pNode );
	m_NonStaticNodes.Remove( pNode );
	
	//	notify parent changed state of children
	TQuadTreeZone* pParentZone = GetParentZone().GetObject();
	if ( pParentZone )
		pParentZone->OnChildZoneNodesChanged();

}


//-----------------------------------------------------------------
//	add node to this zone's list
//-----------------------------------------------------------------
void TLMaths::TQuadTreeZone::DoAddNode(TPtr<TQuadTreeNode>& pNode)
{
#ifdef _DEBUG
	if ( m_Nodes.Exists( pNode ) )
	{
		TLDebug_Break("Node already in zone");
		return;
	}
#endif

	//	add to node array
	m_Nodes.Add( pNode );

	//	add to static list
	if ( !pNode->IsStatic() )
		m_NonStaticNodes.Add( pNode );
	
	//	clear list, just in case...
	pNode->SetChildZonesNone();

	//	notify parent changed state of children
	TQuadTreeZone* pParentZone = GetParentZone().GetObject();
	if ( pParentZone )
		pParentZone->OnChildZoneNodesChanged();
}


//-----------------------------------------------------------------
//	list of nodes in a child has changed
//-----------------------------------------------------------------
void TLMaths::TQuadTreeZone::OnChildZoneNodesChanged()
{
	//	evaluate which children have any nodes in them
	//	gr: tiny bit faster as it saves NULL'ing and re-assigning pointers

	//	pre-alloc array as we're not add()'ing
	m_ChildrenWithNodes.SetSize( 4 );
	m_ChildrenWithNonStaticNodes.SetSize( 4 );
	u32 WithNodesIndex = 0;
	u32 WithNonStaticNodesIndex = 0;
	for ( u32 c=0;	c<m_Children.GetSize();	c++ )
	{
		TPtr<TQuadTreeZone>& ChildZone = m_Children.ElementAt(c);
		if ( !ChildZone->HasAnyNodesTotal() )
			continue;
		
		m_ChildrenWithNodes[WithNodesIndex++] = ChildZone;
		
		if ( ChildZone->HasAnyNonStaticNodesTotal() )
			m_ChildrenWithNonStaticNodes[WithNonStaticNodesIndex++] = ChildZone;
	}
	
	//	cull array
	m_ChildrenWithNodes.SetSize( WithNodesIndex );
	m_ChildrenWithNonStaticNodes.SetSize( WithNonStaticNodesIndex );

#ifdef ENABLE_CULL_EMPTY_ZONES
	if ( m_ZoneParams.m_CullEmptyZones )
	{
		//	if we have ANY nodes below us in the tree, abort cull
		if ( !HasChildrenAnyNodes() )
		{
			//	children are empty, remove them!
			m_Children.Empty();

			m_ChildrenWithNodes.Empty();

			//	need to remove child lists in nodes for our node-is-intersecting-multiple-child-zones thing
			for ( u32 n=0;	n<m_Nodes.GetSize();	n++ )
			{
				m_Nodes[n]->SetChildZonesNone();
			}
		}
	}
#endif // ENABLE_CULL_EMPTY_ZONES

	//	update parent's children-with-nodes status too
	TQuadTreeZone* pParentZone = GetParentZone().GetObject();
	if ( pParentZone )
		pParentZone->OnChildZoneNodesChanged();
}



//-----------------------------------------------------------------
//	count number of nodes in children (recursive)
//-----------------------------------------------------------------
u32 TLMaths::TQuadTreeZone::GetNodeCountTotal()
{
	u32 NodeCount = 0;
		
	//	if we have any nodes, return
	NodeCount += m_Nodes.GetSize();

	//	see if any children have any nodes
	for ( u32 c=0;	c<m_Children.GetSize();	c++ )
	{
		NodeCount += m_Children[c]->GetNodeCountTotal();
	}

	return NodeCount;
}



//-----------------------------------------------------------------
//	count number of nodes in children (recursive)
//-----------------------------------------------------------------
u32 TLMaths::TQuadTreeZone::GetNonStaticNodeCountTotal()
{
	u32 NodeCount = 0;
		
	//	if we have any nodes, return
	NodeCount += m_NonStaticNodes.GetSize();

	//	see if any children have any nodes
	for ( u32 c=0;	c<m_Children.GetSize();	c++ )
	{
		NodeCount += m_Children[c]->GetNonStaticNodeCountTotal();
	}

	return NodeCount;
}


//-----------------------------------------------------------------
//	if we subdivide, or delete child zones etc, then call this func - will go up to the root then notify subscribers of the change
//-----------------------------------------------------------------
void TLMaths::TQuadTreeZone::OnZoneStructureChanged()
{
	//	go up to root...
	if ( GetParentZone() )
	{
		GetParentZone()->OnZoneStructureChanged();
		return;
	}

	//	we are the root, send out a changed message
	TLMessaging::TMessage ChangedMessage("OnChanged");
	PublishMessage( ChangedMessage );
}


//-----------------------------------------------------------------
//	if we subdivide, or delete child zones etc, then call this func - will go up to the root then notify subscribers of the change
//-----------------------------------------------------------------
void TLMaths::TQuadTreeZone::OnZoneActiveChanged(SyncBool Active)
{
	//	gr: for now, just notify structure change to rebuild debug render nodes
	OnZoneStructureChanged();
}


//-----------------------------------------------------------------
//	assign sibling
//-----------------------------------------------------------------
void TLMaths::TQuadTreeZone::SetSiblingZones(TPtrArray<TQuadTreeZone>& Siblings)
{
	if ( HasSiblingZones() )
	{
		TLDebug_Break("Siblings already assigned");
		return;
	}

	for ( u32 s=0;	s<Siblings.GetSize();	s++ )
	{
		TPtr<TQuadTreeZone>& pSibling = Siblings.ElementAt(s);

		//	if this new sibling is us, then store our index
		if ( pSibling.GetObject() == this )
		{
			m_SiblingIndex = s;
			continue;
		}

		//	add to list of siblings
		m_SiblingZones.Add( pSibling );
		m_SiblingZoneIndexes.Add( s );
	}
}


//---------------------------------------------------
//	subdivide zone
//---------------------------------------------------
Bool TLMaths::TQuadTreeZone::Divide(TPtr<TLMaths::TQuadTreeZone>& pThis)
{
	//	gr: note, this is 2D, Z size is same as before, need 8 boxes for 3D
	//	split shape
	const TLMaths::TBox2D& CollisionBox = m_Shape;
	float2 BoxHalfSize = CollisionBox.GetSize() * 0.5f;

	//	dont create children if child boxes will be too small
	if ( BoxHalfSize.x < m_ZoneParams.m_MinZoneSize || BoxHalfSize.y < m_ZoneParams.m_MinZoneSize )
		return FALSE;

	//	gr: this can all be sped up, but it's not a big deal
	const float2& BoxMin = CollisionBox.GetMin();
	const float2& BoxMax = CollisionBox.GetMax();

	float2 BoxMinTL( BoxMin.x,				BoxMin.y );
	float2 BoxMinTR( BoxMin.x + BoxHalfSize.x,	BoxMin.y );
	float2 BoxMinBL( BoxMin.x,				BoxMin.y + BoxHalfSize.y );
	float2 BoxMinBR( BoxMin.x + BoxHalfSize.x,	BoxMin.y + BoxHalfSize.y );

	TLMaths::TBox2D BoxTL( BoxMinTL, BoxMinTL + BoxHalfSize );
	TLMaths::TBox2D BoxTR( BoxMinTR, BoxMinTR + BoxHalfSize );
	TLMaths::TBox2D BoxBL( BoxMinBL, BoxMinBL + BoxHalfSize );
	TLMaths::TBox2D BoxBR( BoxMinBR, BoxMinBR + BoxHalfSize );

	//	gr: order should match the TZonePos_XXX order
	m_Children.Add( new TQuadTreeZone( BoxTL, pThis ) );
	m_Children.Add( new TQuadTreeZone( BoxTR, pThis ) );
	m_Children.Add( new TQuadTreeZone( BoxBL, pThis ) );
	m_Children.Add( new TQuadTreeZone( BoxBR, pThis ) );

	//	set sibling information
	m_Children[0]->SetSiblingZones( m_Children );
	m_Children[1]->SetSiblingZones( m_Children );
	m_Children[2]->SetSiblingZones( m_Children );
	m_Children[3]->SetSiblingZones( m_Children );

	//	notify change of zone structure
	OnZoneStructureChanged();

	return TRUE;
}


//---------------------------------------------------
//	recursively divide until we're at our minimum size leafs
//---------------------------------------------------
Bool TLMaths::TQuadTreeZone::DivideAll(TPtr<TLMaths::TQuadTreeZone>& pThis)
{
	//	divide this, if we cant (would be too small) then break out
	if ( !Divide( pThis ) )
		return FALSE;

	//	divided, devide children again
	for ( u32 c=0;	c<m_Children.GetSize();	c++ )
	{
		TPtr<TLMaths::TQuadTreeZone>& pChild = m_Children[c];
		pChild->DivideAll( pChild );
	}

	return TRUE;
}


//---------------------------------------------------
//	sets this zone and all it's child zones as active
//---------------------------------------------------
void TLMaths::TQuadTreeZone::SetActive(SyncBool Active,Bool SetChildren)
{
	//	gr: don't do children if no change?
	if ( m_Active != Active )
	{
		//	change state
		m_Active = Active;

		//	notify change to nodes if awake/sleep state changed
		for ( u32 n=0;	n<m_Nodes.GetSize();	n++ )
		{
			TLMaths::TQuadTreeNode& Node = *(m_Nodes[n]);
			if ( m_Active == SyncFalse )
				Node.OnZoneSleep();
			else
				Node.OnZoneWake( m_Active );
		}

		//	do any on-zone-activation changed notification
		OnZoneActiveChanged( m_Active );
	}

	if ( SetChildren )
	{
		for ( u32 c=0;	c<m_Children.GetSize();	c++ )
		{
			TPtr<TLMaths::TQuadTreeZone>& pChild = m_Children[c];

			pChild->SetActive( Active, SetChildren );
		}
	}
}


//---------------------------------------------------
//	recursive call to FindNeighbours to sort out all the neighbours in the tree
//---------------------------------------------------
void TLMaths::TQuadTreeZone::FindNeighboursAll(TPtr<TLMaths::TQuadTreeZone>& pThis)
{
	//	find neighbours...
	FindNeighbours( pThis );

	//	do same for children
	for ( u32 c=0;	c<m_Children.GetSize();	c++ )
	{
		TPtr<TLMaths::TQuadTreeZone>& pChildZone = m_Children[c];
		pChildZone->FindNeighboursAll( pChildZone );
	}
}


//---------------------------------------------------
//	calculate neighbours of ourselves
//---------------------------------------------------
void TLMaths::TQuadTreeZone::FindNeighbours(TPtr<TLMaths::TQuadTreeZone>& pThis)
{
	//	do we have all our neighbour zones calculated already?
	if ( m_NeighbourZones.GetSize() >= TQUADTREEZONE_MAX_NEIGHBOUR_ZONES )
		return;

	//	add neighbours
	m_NeighbourZones.AddUnique( FindNeighbourZone_NorthWest() );
	m_NeighbourZones.AddUnique( FindNeighbourZone_North() );
	m_NeighbourZones.AddUnique( FindNeighbourZone_NorthEast() );
	m_NeighbourZones.AddUnique( FindNeighbourZone_East() );
	m_NeighbourZones.AddUnique( FindNeighbourZone_SouthEast() );
	m_NeighbourZones.AddUnique( FindNeighbourZone_South() );
	m_NeighbourZones.AddUnique( FindNeighbourZone_SouthWest() );
	m_NeighbourZones.AddUnique( FindNeighbourZone_West() );

	//	remove null entries
	m_NeighbourZones.RemoveNull();

	//	too many neighbours!
	if ( m_NeighbourZones.GetSize() > TQUADTREEZONE_MAX_NEIGHBOUR_ZONES )
	{
		TLDebug_Break("Calculated too many neighbours for zone!");
		m_NeighbourZones.Empty();
		return;
	}
}


TPtr<TLMaths::TQuadTreeZone>& TLMaths::TQuadTreeZone::GetParentsNeighbourChild(TPtr<TLMaths::TQuadTreeZone>* ppParentNeighbour,TLMaths::TLQuadTree::TZonePos ParentNeighbourChildPositon)
{
	//	parent is on an edge so nothing at the position we want
	if ( !ppParentNeighbour )
		return TLPtr::GetNullPtr<TLMaths::TQuadTreeZone>();

	TLMaths::TQuadTreeZone* pParentNeighbour = *ppParentNeighbour;
	if ( !pParentNeighbour )
		return TLPtr::GetNullPtr<TLMaths::TQuadTreeZone>();

	//	now find that child
	return pParentNeighbour->GetChildFromZonePosition( ParentNeighbourChildPositon );
}


TLMaths::TLQuadTree::TZonePos TLMaths::TLQuadTree::GetMirrorPosition_Vertical(TLMaths::TLQuadTree::TZonePos Position)
{
	if ( Position == TLMaths::TLQuadTree::ZonePos_TopLeft )		return TLMaths::TLQuadTree::ZonePos_BottomLeft;
	if ( Position == TLMaths::TLQuadTree::ZonePos_TopRight )	return TLMaths::TLQuadTree::ZonePos_BottomRight;
	if ( Position == TLMaths::TLQuadTree::ZonePos_BottomLeft )	return TLMaths::TLQuadTree::ZonePos_TopLeft;
	if ( Position == TLMaths::TLQuadTree::ZonePos_BottomRight )	return TLMaths::TLQuadTree::ZonePos_TopRight;

	return TLMaths::TLQuadTree::ZonePos_Invalid;
}

TLMaths::TLQuadTree::TZonePos TLMaths::TLQuadTree::GetMirrorPosition_Horizontal(TLMaths::TLQuadTree::TZonePos Position)
{
	if ( Position == TLMaths::TLQuadTree::ZonePos_TopLeft )		return TLMaths::TLQuadTree::ZonePos_TopRight;
	if ( Position == TLMaths::TLQuadTree::ZonePos_TopRight )	return TLMaths::TLQuadTree::ZonePos_TopLeft;
	if ( Position == TLMaths::TLQuadTree::ZonePos_BottomLeft )	return TLMaths::TLQuadTree::ZonePos_BottomRight;
	if ( Position == TLMaths::TLQuadTree::ZonePos_BottomRight )	return TLMaths::TLQuadTree::ZonePos_BottomLeft;

	return TLMaths::TLQuadTree::ZonePos_Invalid;
}



TPtr<TLMaths::TQuadTreeZone>& TLMaths::TQuadTreeZone::FindNeighbourZone_West()
{
	//	is a sibling
	TLQuadTree::TZonePos MirrorPositon = GetMirrorPosition_Horizontal( GetZonePos() );
	if ( GetZonePos() == TLMaths::TLQuadTree::ZonePos_TopRight || GetZonePos() == TLMaths::TLQuadTree::ZonePos_BottomRight )
		return GetSiblingFromZonePosition( MirrorPositon );

	//	looking for a node outside of the parent, so go into parent...
	TQuadTreeZone* pParent = GetParentZone();
	if ( !pParent )
		return TLPtr::GetNullPtr<TLMaths::TQuadTreeZone>();

	//	now work out where the neighbour will be 
	TPtr<TQuadTreeZone>* pParentNeighbour = &pParent->FindNeighbourZone_West();
	return GetParentsNeighbourChild( pParentNeighbour, MirrorPositon );
}



TPtr<TLMaths::TQuadTreeZone>& TLMaths::TQuadTreeZone::FindNeighbourZone_North()
{
	//	is a sibling
	TLQuadTree::TZonePos MirrorPositon = GetMirrorPosition_Vertical( GetZonePos() );
	if ( GetZonePos() == TLMaths::TLQuadTree::ZonePos_BottomLeft || GetZonePos() == TLMaths::TLQuadTree::ZonePos_BottomRight )
		return GetSiblingFromZonePosition( MirrorPositon );

	//	looking for a node outside of the parent, so go into parent...
	TQuadTreeZone* pParent = GetParentZone();
	if ( !pParent )
		return TLPtr::GetNullPtr<TLMaths::TQuadTreeZone>();

	//	now work out where the neighbour will be 
	TPtr<TQuadTreeZone>* pParentNeighbour = &pParent->FindNeighbourZone_North();
	return GetParentsNeighbourChild( pParentNeighbour, MirrorPositon );
}




TPtr<TLMaths::TQuadTreeZone>& TLMaths::TQuadTreeZone::FindNeighbourZone_East()
{
	//	is a sibling
	TLQuadTree::TZonePos MirrorPositon = GetMirrorPosition_Horizontal( GetZonePos() );
	if ( GetZonePos() == TLMaths::TLQuadTree::ZonePos_TopLeft || GetZonePos() == TLMaths::TLQuadTree::ZonePos_BottomLeft )
		return GetSiblingFromZonePosition( MirrorPositon );

	//	looking for a node outside of the parent, so go into parent...
	TQuadTreeZone* pParent = GetParentZone();
	if ( !pParent )
		return TLPtr::GetNullPtr<TLMaths::TQuadTreeZone>();

	//	now work out where the neighbour will be 
	TPtr<TQuadTreeZone>* pParentNeighbour = &pParent->FindNeighbourZone_East();
	return GetParentsNeighbourChild( pParentNeighbour, MirrorPositon );
}



TPtr<TLMaths::TQuadTreeZone>& TLMaths::TQuadTreeZone::FindNeighbourZone_South()
{
	//	is a sibling
	TLQuadTree::TZonePos MirrorPositon = GetMirrorPosition_Vertical( GetZonePos() );
	if ( GetZonePos() == TLMaths::TLQuadTree::ZonePos_TopLeft || GetZonePos() == TLMaths::TLQuadTree::ZonePos_TopRight )
		return GetSiblingFromZonePosition( MirrorPositon );

	//	looking for a node outside of the parent, so go into parent...
	TQuadTreeZone* pParent = GetParentZone();
	if ( !pParent )
		return TLPtr::GetNullPtr<TLMaths::TQuadTreeZone>();

	//	now work out where the neighbour will be 
	TPtr<TQuadTreeZone>* pParentNeighbour = &pParent->FindNeighbourZone_South();
	return GetParentsNeighbourChild( pParentNeighbour, MirrorPositon );
}




TPtr<TLMaths::TQuadTreeZone>& TLMaths::TQuadTreeZone::FindNeighbourZone_SouthWest()
{
	//	is a sibling
	if ( GetZonePos() == TLMaths::TLQuadTree::ZonePos_TopRight )
		return GetSiblingFromZonePosition( TLMaths::TLQuadTree::ZonePos_BottomLeft );

	//	looking for a node outside of the parent, so go into parent...
	TQuadTreeZone* pParent = GetParentZone();
	if ( !pParent )
		return TLPtr::GetNullPtr<TLMaths::TQuadTreeZone>();

	//	now work out where the neighbour will be 
	TPtr<TQuadTreeZone>* pParentNeighbour = NULL;
	TLQuadTree::TZonePos ParentNeighbourChildPositon = TLMaths::TLQuadTree::ZonePos_Invalid;

	switch ( GetZonePos() )
	{
	case TLMaths::TLQuadTree::ZonePos_TopLeft:
		//	get parent's WEST neighbour
		pParentNeighbour = &pParent->FindNeighbourZone_West();
		ParentNeighbourChildPositon = TLMaths::TLQuadTree::ZonePos_BottomRight;
		break;

	case TLMaths::TLQuadTree::ZonePos_BottomLeft:
		//	get parent's SOUTH WEST neighbour
		pParentNeighbour = &pParent->FindNeighbourZone_SouthWest();
		ParentNeighbourChildPositon = TLMaths::TLQuadTree::ZonePos_TopRight;
		break;

	case TLMaths::TLQuadTree::ZonePos_BottomRight:
		//	get parent's SOUTH neighbour
		pParentNeighbour = &pParent->FindNeighbourZone_South();
		ParentNeighbourChildPositon = TLMaths::TLQuadTree::ZonePos_TopLeft;
		break;
	}

	return GetParentsNeighbourChild( pParentNeighbour, ParentNeighbourChildPositon );
}





TPtr<TLMaths::TQuadTreeZone>& TLMaths::TQuadTreeZone::FindNeighbourZone_NorthWest()
{
	//	is a sibling
	if ( GetZonePos() == TLMaths::TLQuadTree::ZonePos_BottomRight )
		return GetSiblingFromZonePosition( TLMaths::TLQuadTree::ZonePos_TopLeft );

	//	looking for a node outside of the parent, so go into parent...
	TQuadTreeZone* pParent = GetParentZone();
	if ( !pParent )
		return TLPtr::GetNullPtr<TLMaths::TQuadTreeZone>();

	//	now work out where the neighbour will be 
	TPtr<TQuadTreeZone>* pParentNeighbour = NULL;
	TLQuadTree::TZonePos ParentNeighbourChildPositon = TLQuadTree::ZonePos_Invalid;

	switch ( GetZonePos() )
	{
	case TLMaths::TLQuadTree::ZonePos_TopLeft:
		//	get parent's neighbour
		pParentNeighbour = &pParent->FindNeighbourZone_NorthWest();
		ParentNeighbourChildPositon = TLMaths::TLQuadTree::ZonePos_BottomRight;
		break;

	case TLMaths::TLQuadTree::ZonePos_BottomLeft:
		//	get parent's neighbour
		pParentNeighbour = &pParent->FindNeighbourZone_West();
		ParentNeighbourChildPositon = TLMaths::TLQuadTree::ZonePos_TopRight;
		break;

	case TLMaths::TLQuadTree::ZonePos_TopRight:
		//	get parent's neighbour
		pParentNeighbour = &pParent->FindNeighbourZone_North();
		ParentNeighbourChildPositon = TLMaths::TLQuadTree::ZonePos_BottomLeft;
		break;
	}

	return GetParentsNeighbourChild( pParentNeighbour, ParentNeighbourChildPositon );
}




TPtr<TLMaths::TQuadTreeZone>& TLMaths::TQuadTreeZone::FindNeighbourZone_NorthEast()
{
	//	is a sibling
	if ( GetZonePos() == TLMaths::TLQuadTree::ZonePos_BottomLeft )
		return GetSiblingFromZonePosition( TLMaths::TLQuadTree::ZonePos_TopRight );

	//	looking for a node outside of the parent, so go into parent...
	TQuadTreeZone* pParent = GetParentZone();
	if ( !pParent )
		return TLPtr::GetNullPtr<TLMaths::TQuadTreeZone>();

	//	now work out where the neighbour will be 
	TPtr<TQuadTreeZone>* pParentNeighbour = NULL;
	TLQuadTree::TZonePos ParentNeighbourChildPositon = TLQuadTree::ZonePos_Invalid;

	switch ( GetZonePos() )
	{
	case TLMaths::TLQuadTree::ZonePos_TopLeft:
		//	get parent's neighbour
		pParentNeighbour = &pParent->FindNeighbourZone_North();
		ParentNeighbourChildPositon = TLMaths::TLQuadTree::ZonePos_BottomRight;
		break;

	case TLMaths::TLQuadTree::ZonePos_BottomRight:
		//	get parent's neighbour
		pParentNeighbour = &pParent->FindNeighbourZone_East();
		ParentNeighbourChildPositon = TLMaths::TLQuadTree::ZonePos_TopLeft;
		break;

	case TLMaths::TLQuadTree::ZonePos_TopRight:
		//	get parent's neighbour
		pParentNeighbour = &pParent->FindNeighbourZone_NorthEast();
		ParentNeighbourChildPositon = TLMaths::TLQuadTree::ZonePos_BottomLeft;
		break;
	}

	return GetParentsNeighbourChild( pParentNeighbour, ParentNeighbourChildPositon );
}


TPtr<TLMaths::TQuadTreeZone>& TLMaths::TQuadTreeZone::FindNeighbourZone_SouthEast()
{
	//	is a sibling
	if ( GetZonePos() == TLMaths::TLQuadTree::ZonePos_TopLeft )
		return GetSiblingFromZonePosition( TLMaths::TLQuadTree::ZonePos_BottomRight );

	//	looking for a node outside of the parent, so go into parent...
	TQuadTreeZone* pParent = GetParentZone();
	if ( !pParent )
		return TLPtr::GetNullPtr<TLMaths::TQuadTreeZone>();

	//	now work out where the neighbour will be 
	TPtr<TQuadTreeZone>* pParentNeighbour = NULL;
	TLQuadTree::TZonePos ParentNeighbourChildPositon = TLQuadTree::ZonePos_Invalid;

	switch ( GetZonePos() )
	{
	case TLMaths::TLQuadTree::ZonePos_BottomLeft:
		//	get parent's neighbour
		pParentNeighbour = &pParent->FindNeighbourZone_South();
		ParentNeighbourChildPositon = TLMaths::TLQuadTree::ZonePos_TopRight;
		break;

	case TLMaths::TLQuadTree::ZonePos_BottomRight:
		//	get parent's neighbour
		pParentNeighbour = &pParent->FindNeighbourZone_SouthEast();
		ParentNeighbourChildPositon = TLMaths::TLQuadTree::ZonePos_TopLeft;
		break;

	case TLMaths::TLQuadTree::ZonePos_TopRight:
		//	get parent's neighbour
		pParentNeighbour = &pParent->FindNeighbourZone_East();
		ParentNeighbourChildPositon = TLMaths::TLQuadTree::ZonePos_BottomLeft;
		break;
	}

	return GetParentsNeighbourChild( pParentNeighbour, ParentNeighbourChildPositon );
}


//----------------------------------------------
//	find the sibling with this sibling index (ie. where in the parent they are placed)
//----------------------------------------------
TPtr<TLMaths::TQuadTreeZone>& TLMaths::TQuadTreeZone::GetSiblingFromZonePosition(TLMaths::TLQuadTree::TZonePos Position)
{
	for ( u32 s=0;	s<m_SiblingZones.GetSize();	s++ )
	{
		TPtr<TLMaths::TQuadTreeZone>& pSiblingZone = m_SiblingZones[s];
		if ( pSiblingZone->GetZonePos() == Position )
			return pSiblingZone;
	}

	return TLPtr::GetNullPtr<TLMaths::TQuadTreeZone>();
}

	
//----------------------------------------------
//	search the tree to find the existing zone at this position - todo: expand to use shape
//----------------------------------------------
const TLMaths::TQuadTreeZone* TLMaths::TQuadTreeZone::GetZoneAt(const float2& Position) const
{
	//	not in this shape - fail
	if ( !GetShape().GetIntersection( Position ) )
		return NULL;

	const TLMaths::TQuadTreeZone* pInChildZone = NULL;

	//	in this shape, are we in a child (or multiple children)
	for ( u32 c=0;	c<m_Children.GetSize();	c++ )
	{
		//	do the test with the child, if we get NULL then it's not in this child
		const TLMaths::TQuadTreeZone* pChildZoneResult = m_Children[c]->GetZoneAt( Position );
		if ( !pChildZoneResult )
			continue;

		//	is in this child... (or a childs child)
		
		//	have we intersected multiple children? if so, just return this
		if ( pInChildZone )
			return this;

		//	nope, first time, store the result
		pInChildZone = pChildZoneResult;
	}

	//	are we in a child?
	if ( pInChildZone )
		return pInChildZone;

	//	no, just in this
	return this;
}


//----------------------------------------------
//	get a list of all leaf zones that this shape intersects
//----------------------------------------------
void TLMaths::TQuadTreeZone::GetIntersectingLeafZones(const TLMaths::TLine2D& Shape,TArray<const TLMaths::TQuadTreeZone*>& IntersectZones)
{
	//	check shape is in my shape
	if ( !GetShape().GetIntersection( Shape ) )
		return;

	//	am i a leaf?
	if ( !m_Children.GetSize() )
	{
		//	i am! add me to list
		IntersectZones.Add( this );
		return;
	}

	//	no.. go into children
	for ( u32 c=0;	c<m_Children.GetSize();	c++ )
	{
		m_Children[c]->GetIntersectingLeafZones( Shape, IntersectZones );
	}
}


