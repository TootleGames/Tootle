#include "TCollisionZone.h"
#include "TPhysicsNode.h"
#include "TPhysicsGraph.h"


#define MAX_NODES_PER_ZONE	1
#define MIN_ZONE_SIZE		4.f	//	box width or height must be at least this big (best would be a bit bigger than smallest collision object)

//	gr: faster with culling empty zones
//	gr: some issue after DoAddNode where we delete the children too early, whilst we're adding a node to it...
//	#define CULL_EMPTY_ZONES	//	delete child zones if theyre all empty



TLPhysics::TCollisionZone::TCollisionZone(const TLMaths::TBox2D& ZoneShape,TPtr<TCollisionZone>& pParent) :
	m_pParent				( pParent ),
	m_CollisionShape		( ZoneShape )
{
}


//-------------------------------------------------------------
//	test to see if this intersects into this zone
//-------------------------------------------------------------
Bool TLPhysics::TCollisionZone::IsNodeInZoneShape(TLPhysics::TPhysicsNode* pNode,Bool TestAlreadyInZone)
{
	TLPhysics::g_pPhysicsgraph->m_Debug_InZoneTests++;

	//	get node's collision shape in world space
	if ( !pNode->HasCollision() )
		return FALSE;

	if ( TestAlreadyInZone && pNode->GetCollisionZone().GetObject() == this )
	{
		TLDebug_Break("We already know this node is in this shape...");
		return TRUE;
	}

	TLPhysics::TCollisionShape* pNodeCollisionShape = pNode->CalcWorldCollisionShape();
	if ( !pNodeCollisionShape )
		return FALSE;

	//	test this node and this zone intersect
	if ( !m_CollisionShape.HasIntersection( pNodeCollisionShape ) )
	{
		TLPhysics::g_pPhysicsgraph->m_Debug_InZoneTestsFailed++;
		return FALSE;
	}

	return TRUE;
}



//-------------------------------------------------------------
//	attempt to add this node to this zone. checks with children first 
//	to see if it fits into just one child better. returns FALSE if not in this zone
//-------------------------------------------------------------
Bool TLPhysics::TCollisionZone::AddNode(TPtr<TLPhysics::TPhysicsNode>& pNode,TPtr<TCollisionZone>& pThis,Bool DoCheckInShape)
{
	//	if not in this shape, return
	if ( DoCheckInShape && !IsNodeInZoneShape( pNode, FALSE ) )
		return FALSE;

	u32 NewSize = m_Nodes.GetSize() + (m_Nodes.Exists( pNode ) ? 0 : 1);

	//	fits in this zone, if we have X children already, we need to split
	if ( NewSize >= MAX_NODES_PER_ZONE && m_Children.GetSize() == 0 )
	{
		//	split zone
		const TLMaths::TBox2D& CollisionBox = m_CollisionShape.GetBox();
		const float2& BoxMin = CollisionBox.GetMin();
		const float2& BoxMax = CollisionBox.GetMax();

		//	gr: note, this is 2D, Z size is same as before, need 8 boxes for 3D
		float2 BoxHalf = BoxMax - BoxMin;
		BoxHalf *= 0.5f;

		//	dont create children if child boxes will be too small
		if ( BoxHalf.x > MIN_ZONE_SIZE && BoxHalf.y > MIN_ZONE_SIZE )
		{
			float2 BoxMinTL( BoxMin.x,				BoxMin.y );
			float2 BoxMinTR( BoxMin.x + BoxHalf.x,	BoxMin.y );
			float2 BoxMinBL( BoxMin.x,				BoxMin.y + BoxHalf.y );
			float2 BoxMinBR( BoxMin.x + BoxHalf.x,	BoxMin.y + BoxHalf.y );

			TLMaths::TBox2D BoxTL( BoxMinTL, BoxMinTL + BoxHalf );
			TLMaths::TBox2D BoxTR( BoxMinTR, BoxMinTR + BoxHalf );
			TLMaths::TBox2D BoxBL( BoxMinBL, BoxMinBL + BoxHalf );
			TLMaths::TBox2D BoxBR( BoxMinBR, BoxMinBR + BoxHalf );

			m_Children.Add( new TCollisionZone( BoxTL, pThis ) );
			m_Children.Add( new TCollisionZone( BoxTR, pThis ) );
			m_Children.Add( new TCollisionZone( BoxBL, pThis ) );
			m_Children.Add( new TCollisionZone( BoxBR, pThis ) );

			//	if we process pNode in this code then we dont need to do it later
			Bool DoneNode = FALSE;

			//	re-evaluate existing nodes to see if they fit better in a child
			for ( s32 n=m_Nodes.GetSize()-1;	n>=0;	n-- )
			{
				TPtr<TLPhysics::TPhysicsNode>& pChildNode = m_Nodes[n];
				DoneNode |= (pNode == pChildNode);

				//	see if child node is now in one of the new zones
				TFixedArray<u32,4> InZones(0);
				GetInChildZones( pChildNode.GetObject(), InZones );

				//	node is not in any of these zones... error...
				if ( InZones.GetSize() == 0 )
				{
					pChildNode->SetChildZonesNone();
					continue;
				}

				//	in multiple zones, so stay in this zone, but assign the child zones
				if ( InZones.GetSize() > 1 )
				{
					pChildNode->SetChildZones( InZones );
					continue;
				}

				//	only in 1 child zone, so move out of this zone add into this new child zone
				TPtr<TCollisionZone>& pNewZone = m_Children[ InZones[0] ];
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
		return pNode->SetCollisionZone( pThis, pNode, NULL );
	}

	//	loop through the child zones to see if it fits into 1 or multiple zones
	TFixedArray<u32,4> InZones(0);
	GetInChildZones( pNode.GetObject(), InZones );

	//	node is not in any of these zones... error... 
	//	or
	//	in multiple zones, so stay in this zone, but assign the child zones
	if ( InZones.GetSize() != 1 )
		return pNode->SetCollisionZone( pThis, pNode, &InZones );

	//	only in one child zone, add to this child zone (will go through same process, split child as neccessary etc)
	TPtr<TCollisionZone>& pNewZone = m_Children[ InZones[0] ];
	Bool AddedToChild = pNewZone->AddNode( pNode, pNewZone, FALSE );
	if ( !AddedToChild )
	{
		TLDebug_Break("Error: inside zone, inside child zone, but failed to add to child zone...");
		return pNode->SetCollisionZone( pThis, pNode, &InZones );
	}

	return TRUE;
}


//---------------------------------------------------------------------
//	return which child zone we're in, -1 if none
//---------------------------------------------------------------------
void TLPhysics::TCollisionZone::GetInChildZones(TPhysicsNode* pNode,TFixedArray<u32,4>& InZones)
{
	//	loop through the child zones to see if it fits into 1 or multiple zones
	for ( u32 c=0;	c<m_Children.GetSize();	c++ )
	{
		TPtr<TCollisionZone>& pChildZone = m_Children[c];
		if ( pChildZone->IsNodeInZoneShape( pNode, FALSE ) )
		{
			//	add to list of zones we're in
			InZones.Add( c );
		}
	}
}


//-----------------------------------------------------------------
//	remove node from this zone's list
//-----------------------------------------------------------------
void TLPhysics::TCollisionZone::DoRemoveNode(TPtr<TPhysicsNode>& pNode)
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
	TCollisionZone* pParentZone = GetParentZone().GetObject();
	if ( pParentZone )
		pParentZone->OnChildZoneNodesChanged();

}


//-----------------------------------------------------------------
//	add node to this zone's list
//-----------------------------------------------------------------
void TLPhysics::TCollisionZone::DoAddNode(TPtr<TPhysicsNode>& pNode)
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
	TCollisionZone* pParentZone = GetParentZone().GetObject();
	if ( pParentZone )
		pParentZone->OnChildZoneNodesChanged();
}


//-----------------------------------------------------------------
//	list of nodes in a child has changed
//-----------------------------------------------------------------
void TLPhysics::TCollisionZone::OnChildZoneNodesChanged()
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
		TPtr<TCollisionZone>& ChildZone = m_Children.ElementAt(c);
		if ( !ChildZone->HasAnyNodesTotal() )
			continue;
		
		m_ChildrenWithNodes[WithNodesIndex++] = ChildZone;
		
		if ( ChildZone->HasAnyNonStaticNodesTotal() )
			m_ChildrenWithNonStaticNodes[WithNonStaticNodesIndex++] = ChildZone;
	}
	
	//	cull array
	m_ChildrenWithNodes.SetSize( WithNodesIndex );
	m_ChildrenWithNonStaticNodes.SetSize( WithNonStaticNodesIndex );
/*
	m_ChildrenWithNodes.Empty();
	m_ChildrenWithNonStaticNodes.Empty();
	for ( u32 c=0;	c<m_Children.GetSize();	c++ )
	{
		if ( m_Children[c]->HasAnyNodesTotal() )
			m_ChildrenWithNodes.Add( m_Children[c] );

		if ( m_Children[c]->HasAnyNonStaticNodesTotal() )
			m_ChildrenWithNonStaticNodes.Add( m_Children[c] );
	}
*/

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
	TCollisionZone* pParentZone = GetParentZone().GetObject();
	if ( pParentZone )
		pParentZone->OnChildZoneNodesChanged();
}



//-----------------------------------------------------------------
//	count number of nodes in children (recursive)
//-----------------------------------------------------------------
u32 TLPhysics::TCollisionZone::GetNodeCountTotal()
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
u32 TLPhysics::TCollisionZone::GetNonStaticNodeCountTotal()
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




