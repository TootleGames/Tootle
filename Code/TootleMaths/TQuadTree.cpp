#include "TQuadTree.h"


#define MAX_NODES_PER_ZONE	2
#define MIN_ZONE_SIZE		6.0f	//	box width or height must be at least this big (best would be a bit bigger than smallest collision object)

//	gr: faster with culling empty zones
//	gr: some issue after DoAddNode where we delete the children too early, whilst we're adding a node to it...
//	#define CULL_EMPTY_ZONES	//	delete child zones if theyre all empty


namespace TLMaths
{
	namespace TLQuadTree
	{
		FORCEINLINE TLArray::SortResult	SortNodes(const TPtr<TLMaths::TQuadTreeNode>& a,const TPtr<TLMaths::TQuadTreeNode>& b,const void* pTestVal);

		TRef		g_NextQuadTreeNodeRef;	//	unique ref for tree nodes which is automaticcaly incremented
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
	TPtr<TQuadTreeZone>& pOldZone = GetZone();
	if ( pOldZone == pZone )
	{
		//	just update child list
		if ( !pChildZoneList )
			SetChildZonesNone();
		else
			SetChildZones( *pChildZoneList );

		return TRUE;
	}

	//	remove from old zone
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














TLMaths::TQuadTreeZone::TQuadTreeZone(const TLMaths::TBox2D& ZoneShape,TPtr<TLMaths::TQuadTreeZone>& pParent) :
	m_pParent	( pParent ),
	m_Shape		( ZoneShape ),
	m_Nodes		( &TLMaths::TLQuadTree::SortNodes ),
	m_SiblingZoneIndexes	( 0 ),
	m_SiblingIndex			( -1 )

{
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
	if ( NewSize > MAX_NODES_PER_ZONE && m_Children.GetSize() == 0 )
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

#ifdef CULL_EMPTY_ZONES
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
#endif

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
	//	split shape
	const TLMaths::TBox2D& CollisionBox = m_Shape;
	const float2& BoxMin = CollisionBox.GetMin();
	const float2& BoxMax = CollisionBox.GetMax();

	//	gr: note, this is 2D, Z size is same as before, need 8 boxes for 3D
	float2 BoxHalf = BoxMax - BoxMin;
	BoxHalf *= 0.5f;

	//	dont create children if child boxes will be too small
	if ( BoxHalf.x < MIN_ZONE_SIZE || BoxHalf.y < MIN_ZONE_SIZE )
		return FALSE;

	float2 BoxMinTL( BoxMin.x,				BoxMin.y );
	float2 BoxMinTR( BoxMin.x + BoxHalf.x,	BoxMin.y );
	float2 BoxMinBL( BoxMin.x,				BoxMin.y + BoxHalf.y );
	float2 BoxMinBR( BoxMin.x + BoxHalf.x,	BoxMin.y + BoxHalf.y );

	TLMaths::TBox2D BoxTL( BoxMinTL, BoxMinTL + BoxHalf );
	TLMaths::TBox2D BoxTR( BoxMinTR, BoxMinTR + BoxHalf );
	TLMaths::TBox2D BoxBL( BoxMinBL, BoxMinBL + BoxHalf );
	TLMaths::TBox2D BoxBR( BoxMinBR, BoxMinBR + BoxHalf );

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

