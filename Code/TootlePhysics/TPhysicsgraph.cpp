#include "TPhysicsgraph.h"
#include "TCollisionShape.h"
#include "TPhysicsNode.h"
#include "TPhysicsNodeSphere.h"

#include <TootleCore/TLTime.h>

//	no collide once, 3 iterastions - 5.X but stable
//	collide once, 4 iterations, 4.4, quite stable
//	collide once, 3 iterasions, 3.4 quite stable
//	collide once, 2 iterasions, 3.0 very erratic, but quite stable
//	collide once, 1 iterasions, 2.0 very erratic, unstable

//#define COLLIDE_ONCE
#define COLLISION_ITERATIONS	1
//#define ENABLE_NOZONE_COLLISION
//#define ENABLE_COLLISIONTEST_TRACE


#define ENABLE_TEST_PARENTZONENODE_INMYZONE
#define ENABLE_TEST_PARENTZONENODE_INPREVIOUSPARENTZONE
/*
NONE:
Collision tests: 2899 (300 static) 443 intersections(15.28%) 1463 zone tests(63.57%)

ENABLE_TEST_PARENTZONENODE_INMYZONE:
Collision tests: 1963 (144 static) 415 intersections(21.14%) 3435 zone tests(57.09%)

ENABLE_TEST_PARENTZONENODE_INPREVIOUSPARENTZONE:
Collision tests: 2618 (207 static) 435 intersections(16.62%) 1510 zone tests(62.38%)

BOTH:
Collision tests: 1763 (136 static) 441 intersections(25.01%) 3066 zone tests(54.96%)
*/

//#define PREDIVIDE_COLLISION_ZONES

namespace TLPhysics
{
	TPtr<TPhysicsgraph> g_pPhysicsgraph;

	Bool		Debug_CheckCollisionObjects(TLPhysics::TPhysicsNode* pNodeA,TLPhysics::TPhysicsNode* pNodeB);
}







TLPhysics::TPhysicsgraph::TPhysicsgraph(TRefRef refManagerID) :
	TLGraph::TGraph<TLPhysics::TPhysicsNode>	( refManagerID ),
	m_Debug_CollisionTestCount					( 0 ),
	m_Debug_StaticCollisionTestCount			( 0 ),
	m_Debug_CollisionTestDupeSavedCount			( 0 ),
	m_Debug_CollisionIntersections				( 0 ),
	m_Debug_InZoneTests							( 0 ),
	m_Debug_InZoneTestsFailed					( 0 )
{
}


SyncBool TLPhysics::TPhysicsgraph::Initialise()
{
	if ( TLGraph::TGraph<TLPhysics::TPhysicsNode>::Initialise() == SyncFalse )
		return SyncFalse;

	//	create generic render node factory
	TPtr<TClassFactory<TLPhysics::TPhysicsNode,FALSE> > pFactory = new TPhysicsNodeFactory();
	AddFactory(pFactory);

	return SyncTrue;
}
	
void TLPhysics::TPhysicsgraph::UpdateGraph(float fTimeStep)
{
	//	reset collision test count
	m_Debug_CollisionTestCount = 0;
	m_Debug_StaticCollisionTestCount = 0;
	m_Debug_CollisionTestDupeSavedCount = 0;
	m_Debug_CollisionIntersections = 0;
	m_Debug_InZoneTests = 0;
	m_Debug_InZoneTestsFailed = 0;
	m_Debug_CollisionTestsUpwards = 0;
	m_Debug_CollisionTestsDownwards = 0;
	m_Debug_ZonesTested = 0;

	// DB - This needs removing.  Essentially the physcis is still working per-frame
	// rather than in seconds.  
	// Also, the collision code doesn't support any kind of time at 
	// all so will need further changes to get the collisions working correctly.
	float fFrameStep = fTimeStep * TLTime::GetUpdatesPerSecond();

	// DB - The physics will assume a max of one frame step with the following.
	//	gr: capping timestep for testing
	if ( fFrameStep > 1.f )
		fFrameStep = 1.f;

	// Process all queued messages first
	ProcessMessageQueue();
	
	//	no root? nothing to do
	TPtr<TLPhysics::TPhysicsNode>& pRootNode = GetRootNode();
	if ( pRootNode )
	{
		//	do update
		pRootNode->UpdateAll( fFrameStep );

		//	do collisions
#ifdef DO_COLLISIONS_BY_ZONE
		if ( m_pRootCollisionZone )
			DoCollisionsByZone( m_pRootCollisionZone.GetObject() );
#endif

#ifdef DO_COLLISIONS_BY_NODE
		DoCollisionsByNode();
#endif

#ifdef DO_COLLISIONS_BY_NODEUPWARDS
		DoCollisionsByNodeUpwards();
#endif

		//	do post update
		pRootNode->PostUpdateAll( fFrameStep, this, pRootNode );
	}
	
	// Update the graph structure with any changes
	UpdateGraphStructure();

	//	debug collision count
#ifdef _DEBUG
#ifdef ENABLE_COLLISIONTEST_TRACE
	float ZoneTestSuccess = m_Debug_InZoneTests == 0 ? 0.f : (float)m_Debug_InZoneTestsFailed / (float)m_Debug_InZoneTests;
	float IntersectionRate = m_Debug_CollisionTestCount == 0 ? 0.f : (float)m_Debug_CollisionIntersections / (float)m_Debug_CollisionTestCount;
	float CollisionRateUp = m_Debug_CollisionTestCount == 0 ? 0.f : (float)m_Debug_CollisionTestsUpwards / (float)m_Debug_CollisionTestCount;
	float CollisionRateDown = m_Debug_CollisionTestCount == 0 ? 0.f : (float)m_Debug_CollisionTestsDownwards / (float)m_Debug_CollisionTestCount;
	//TLDebug_Print( TString("Collision tests: %d (%d static, %.2f up, %.2f down) %d intersections(%.2f%%) %d zone shape tests(%.2f%%) %d zone loops", m_Debug_CollisionTestCount, m_Debug_StaticCollisionTestCount, CollisionRateUp, CollisionRateDown, m_Debug_CollisionIntersections, IntersectionRate*100.f, m_Debug_InZoneTests, ZoneTestSuccess*100.f, m_Debug_ZonesTested ) );
#endif
#endif
}


//----------------------------------------------------------
//	
//----------------------------------------------------------
void TLPhysics::TPhysicsgraph::DoCollisionsByZone(TLMaths::TQuadTreeZone* pCollisionZone)
{
	TPtrArray<TLMaths::TQuadTreeNode>& ZoneNodes = pCollisionZone->GetNodes();
	for ( u32 n=0;	n<ZoneNodes.GetSize();	n++ )
	{
		TLMaths::TQuadTreeNode* pNode = ZoneNodes[n].GetObject();
		//if ( pNode->IsStatic() )
		//	continue;

		//	do collisions for each node in this zone
		DoCollisionsByZone( pCollisionZone, pNode, TRUE, n+1 );
	}

	//	now do child zones
	TPtrArray<TLMaths::TQuadTreeZone>& ZoneChildZonesWithNodes = pCollisionZone->GetChildZonesWithNodes();
	for ( u32 c=0;	c<ZoneChildZonesWithNodes.GetSize();	c++ )
	{
		DoCollisionsByZone( ZoneChildZonesWithNodes[c].GetObject() );
	}

}


//----------------------------------------------------------
//	do collisons with each node in this zone, then go to zone's child zones
//----------------------------------------------------------
void TLPhysics::TPhysicsgraph::DoCollisionsByZone(TLMaths::TQuadTreeZone* pCollisionZone,TLMaths::TQuadTreeNode* pNode,Bool IsNodesZone,u32 FirstNode)
{
	Bool NodeIsStatic = pNode->IsStatic();

	Bool TestStaticNodes = (!NodeIsStatic);

	//	version 1 - checks less nodes in the parent zone (for the list in the zone) but doesnt skip static nodes
	if ( FirstNode != 0 )
	{
		TestStaticNodes = TRUE;
	}
/*
	//	version 2 - duplicates nodes tests in the parent zone (for the list in the zone) but skips static-static node tests
	if ( NodeIsStatic )
	{
		TestStaticNodes = FALSE;
		FirstNode = 0;
	}
*/
	m_Debug_ZonesTested++;

	//	collide with nodes in this zone
	TPtrArray<TLMaths::TQuadTreeNode>& ZoneNodes = TestStaticNodes ? pCollisionZone->GetNodes() : pCollisionZone->GetNonStaticNodes();
	for ( u32 o=FirstNode;	o<ZoneNodes.GetSize();	o++ )
	{
		TLMaths::TQuadTreeNode* pOtherNode = ZoneNodes[o].GetObject();
	
		if ( IsNodesZone && pNode==pOtherNode )
			continue;

		TLPhysics::TPhysicsNode* pPhysicsNode = static_cast<TLPhysics::TPhysicsZoneNode*>( pNode )->GetPhysicsNode();
		TLPhysics::TPhysicsNode* pOtherPhysicsNode = static_cast<TLPhysics::TPhysicsZoneNode*>( pOtherNode )->GetPhysicsNode();
		if ( !pOtherPhysicsNode )
			continue;

		Bool OtherNodeIsStatic = pOtherPhysicsNode->IsStatic();
	
		//	do node collisions
		if ( !NodeIsStatic && OtherNodeIsStatic )
		{
			DoStaticCollision( pPhysicsNode, pOtherPhysicsNode );
		}
		else if ( NodeIsStatic && !OtherNodeIsStatic )
		{
			DoStaticCollision( pOtherPhysicsNode, pPhysicsNode );
		}
		else if ( NodeIsStatic && OtherNodeIsStatic )
		{
			//	no collision test!
		}
		else
		{
			DoCollision( pPhysicsNode, pOtherPhysicsNode );
		}
	}

	//	collide with nodes in child zones - this is using the list of zones we know that this zone overlaps (will be at least 2)
	TPtrArray<TLMaths::TQuadTreeZone>& ZoneChildZones = pCollisionZone->GetChildZones();
	TPtrArray<TLMaths::TQuadTreeZone>& ZoneChildZonesWithNodes = NodeIsStatic ? pCollisionZone->GetChildZonesWithNonStaticNodes() : pCollisionZone->GetChildZonesWithNodes();
	if ( IsNodesZone && ZoneChildZones.GetSize() )
	{
		TFixedArray<u32,4>& NodeChildZones = pNode->GetChildZones();
		for ( u32 c=0;	c<NodeChildZones.GetSize();	c++ )
		{
			TLMaths::TQuadTreeZone* pNodeChildZone = ZoneChildZones[NodeChildZones[c]].GetObject();
			
			//	if we're a static node, and this child zone only has static nodes, dont do the collision tests
			if ( NodeIsStatic && !pNodeChildZone->HasAnyNonStaticNodesTotal() )
				continue;

			//	if there are no nodes at all, skip the check
			if ( !pNodeChildZone->HasAnyNodesTotal() )
				continue;

			if ( !pNodeChildZone->IsNodeInZoneShape( pNode/*, FALSE*/ ) )
			{
			//	TLDebug_Break("Should be intersecting this zone");
				continue;
			}

			DoCollisionsByZone( pNodeChildZone, pNode, FALSE, 0 );
		}
	}
	else if ( !IsNodesZone && ZoneChildZonesWithNodes.GetSize() )
	{
		//	don't know what child zones our node intersects, so check all the child zones with any nodes in them
		for ( u32 c=0;	c<ZoneChildZonesWithNodes.GetSize();	c++ )
		{
			TLMaths::TQuadTreeZone* pNodeChildZone = ZoneChildZonesWithNodes[c].GetObject();

			//	check our node intersects with this child zone
			if ( pNodeChildZone->IsNodeInZoneShape( pNode/*, TRUE*/ ) )
				DoCollisionsByZone( pNodeChildZone, pNode, FALSE, 0 );
		}
	}
}


//----------------------------------------------------------
//	do all the object-object collison iterations
//----------------------------------------------------------
void TLPhysics::TPhysicsgraph::DoCollisionsByNode()
{
	//	loop through every node and do a collision
	//	todo; reduce to only a list of nodes that have moved?
	for ( u32 n=0;	n<GetNodeList().GetSize();	n++ )
	{
		TPtr<TPhysicsNode>& pNode = GetNodeList().ElementAt(n);

		if ( pNode->IsStatic() )
			continue;

		if ( !pNode->HasCollision() )
			continue;

		DoCollisionsByNode( pNode );
	}

}


//----------------------------------------------------------
//	do collision tests for this node
//----------------------------------------------------------
void TLPhysics::TPhysicsgraph::DoCollisionsByNode(TPtr<TLPhysics::TPhysicsNode>& pNode)
{
	//	get our zone
	TPhysicsNode* pNodeObject = pNode.GetObject();
	TLMaths::TQuadTreeZone* pNodeCollisionZone = pNodeObject->GetZone().GetObject();

	//	no zone? dont know what to test with...
	if ( !pNodeCollisionZone )
		return;

	//	collide with all the nodes in our zone
	u32 DummyCounter = 0;
	DoCollisionsByNode( pNodeObject, pNodeCollisionZone, pNodeCollisionZone, NULL, FALSE, FALSE, DummyCounter );

	//	collide with the multiple child zones we intersect (saving here is say only doing 2/4 child-zone checks)
	TFixedArray<u32,4>& ChildCollisionZones = pNodeObject->GetChildZones();
	if ( ChildCollisionZones.GetSize() && pNodeCollisionZone->HasChildrenAnyNodes() )
	{
		TPtrArray<TLMaths::TQuadTreeZone>& NodeZoneChildZones = pNodeCollisionZone->GetChildZones();

		//	gr: when culling, for some reason NodeZoneChildZones becomes empty, but the child list on our node... doesnt...
		for ( u32 c=0;	c<ChildCollisionZones.GetSize()/* && NodeZoneChildZones.GetSize()*/;	c++ )
		{
			u32 ChildZoneIndex = ChildCollisionZones[c];
			TLMaths::TQuadTreeZone* pChildCollisionZone = NodeZoneChildZones[ChildZoneIndex].GetObject();

			//	if child zone has no nodes, dont check
			if ( !pChildCollisionZone->HasAnyNodesTotal() )
				continue;
			
			DoCollisionsByNode( pNodeObject, pChildCollisionZone, pNodeCollisionZone,  NULL, TRUE, FALSE, m_Debug_CollisionTestsDownwards );
		}
	}

	//	now collide with nodes in the parent zone (nodes in the parent zone are overlapping
	TLMaths::TQuadTreeZone* pPrevParentCollisionZone = pNodeCollisionZone;
	TLMaths::TQuadTreeZone* pParentCollisionZone = pNodeCollisionZone->GetParentZone().GetObject();
	while ( pParentCollisionZone )
	{
		DoCollisionsByNode( pNodeObject, pParentCollisionZone, pNodeCollisionZone, pPrevParentCollisionZone, FALSE, TRUE, m_Debug_CollisionTestsUpwards );
		pPrevParentCollisionZone = pParentCollisionZone;
		pParentCollisionZone = pParentCollisionZone->GetParentZone().GetObject();
	}

}

//----------------------------------------------------------
//	
//----------------------------------------------------------
void TLPhysics::TPhysicsgraph::DoCollisionsByNode(TLPhysics::TPhysicsNode* pNode,TLMaths::TQuadTreeZone* pCollisionZone,TLMaths::TQuadTreeZone* pNodeZone,TLMaths::TQuadTreeZone* pPreviousParentZone,Bool TestChildZones,Bool TestNodeZone,u32& CollisionTestCounter)
{
	if ( TestNodeZone && pNodeZone == pCollisionZone )
		TestNodeZone = FALSE;

	m_Debug_ZonesTested++;

	//	collide with all the nodes in this zone
	TPtrArray<TLMaths::TQuadTreeNode>& ZoneNodes = pCollisionZone->GetNodes();
	for ( u32 n=0;	n<ZoneNodes.GetSize();	n++ )
	{
		TPtr<TLMaths::TQuadTreeNode>& pOtherNodePtr = ZoneNodes[n];
		TLMaths::TQuadTreeNode* pOtherNode = pOtherNodePtr.GetObject();

		//	dont collide with self!
		if ( pNode->GetQuadTreeNodeRef() == pOtherNode->GetQuadTreeNodeRef() )
			continue;
		
		//	to save some collision time, check to see if this OtherNode intersects our node's zone 
		//	this is to skip collision checks with a node in a parent(or parent parent) zone that might be far away
		if ( TestNodeZone )
		{
#ifdef ENABLE_TEST_PARENTZONENODE_INPREVIOUSPARENTZONE
			if ( pPreviousParentZone )
			{
				TFixedArray<u32,4>& OtherNodeChildZones = pOtherNode->GetChildZones();
				if ( OtherNodeChildZones.GetSize() )
				{
					Bool IsInPrevParentZone = FALSE;
					for ( u32 c=0;	c<OtherNodeChildZones.GetSize();	c++ )
					{
						TPtr<TLMaths::TQuadTreeZone>& pOtherNodeChildZone = pCollisionZone->GetChildZones().ElementAt( OtherNodeChildZones[c] );
						if ( pOtherNodeChildZone.GetObject() == pPreviousParentZone )
						{
							IsInPrevParentZone = TRUE;
							break;
						}
					}
					
					if ( !IsInPrevParentZone )
						continue;
				}
			}
#endif
#ifdef ENABLE_TEST_PARENTZONENODE_INMYZONE
			if ( !pNodeZone->IsNodeInZoneShape( pOtherNode/*, TRUE*/ ) )
				continue;
#endif
		}

		TLPhysics::TPhysicsNode* pOtherPhysicsNode = static_cast<TLPhysics::TPhysicsZoneNode*>( pOtherNode )->GetPhysicsNode();

		//	do node collisions
		if ( pOtherPhysicsNode->IsStatic() )
			DoStaticCollision( pNode, pOtherPhysicsNode );
		else
			DoCollision( pNode, pOtherPhysicsNode );
		CollisionTestCounter++;
	}

	//	if we know the children have no nodes, skip the check
	if ( !pCollisionZone->HasChildrenAnyNodes() )
		TestChildZones = FALSE;

	//	now collide with nodes in the child zones... if there are some then we must be overlapping
	//	at least 2 child zones, other we would be IN that child zone
	if ( TestChildZones )
	{
		for ( u32 c=0;	c<pCollisionZone->GetChildZonesWithNodes().GetSize();	c++ )
		{
			TPtr<TLMaths::TQuadTreeZone>& pChildCollisionZone = pCollisionZone->GetChildZones().ElementAt(c);

			//	test against this zone to see if we're intersecting it
			if ( !pChildCollisionZone->IsNodeInZoneShape( &pNode->GetZoneNode()/*, TRUE*/ ) )
				continue;

			//	do collisions with nodes in child zone
			DoCollisionsByNode( pNode, pChildCollisionZone.GetObject(), pNodeZone, NULL, TestChildZones, TestNodeZone, CollisionTestCounter );
		}
	}
}




//----------------------------------------------------------
//	do all the object-object collison iterations
//----------------------------------------------------------
void TLPhysics::TPhysicsgraph::DoCollisionsByNodeUpwards()
{
	//	loop through every node and do a collision
	//	todo; reduce to only a list of nodes that have moved?
	for ( u32 n=0;	n<GetNodeList().GetSize();	n++ )
	{
		TPtr<TPhysicsNode>& pNode = GetNodeList().ElementAt(n);

		//	need to do static collisions when going up the tree
		//if ( pNode->IsStatic() )
		//	continue;

		if ( !pNode->HasCollision() )
			continue;

		DoCollisionsByNodeUpwards( pNode );
	}

}


//----------------------------------------------------------
//	do collision tests for this node
//----------------------------------------------------------
void TLPhysics::TPhysicsgraph::DoCollisionsByNodeUpwards(TPtr<TLPhysics::TPhysicsNode>& pNode)
{
	//	get our zone
	TPhysicsNode* pNodeObject = pNode.GetObject();
	TLMaths::TQuadTreeZone* pNodeCollisionZone = pNodeObject->GetZone().GetObject();

	//	no zone? dont know what to test with...
	if ( !pNodeCollisionZone )
		return;

	//	collide with all the nodes in our zone
	u32 DummyCounter = 0;
	DoCollisionsByNodeUpwards( pNodeObject, pNodeCollisionZone, pNodeCollisionZone, NULL, FALSE, FALSE, DummyCounter );

	//	now collide with nodes in the parent zone (nodes in the parent zone are overlapping
	TLMaths::TQuadTreeZone* pPrevParentCollisionZone = pNodeCollisionZone;
	TLMaths::TQuadTreeZone* pParentCollisionZone = pNodeCollisionZone->GetParentZone().GetObject();
	while ( pParentCollisionZone )
	{
		DoCollisionsByNodeUpwards( pNodeObject, pParentCollisionZone, pNodeCollisionZone, pPrevParentCollisionZone, FALSE, TRUE, m_Debug_CollisionTestsUpwards );
		pPrevParentCollisionZone = pParentCollisionZone;
		pParentCollisionZone = pParentCollisionZone->GetParentZone().GetObject();
	}

}

//----------------------------------------------------------
//	
//----------------------------------------------------------
void TLPhysics::TPhysicsgraph::DoCollisionsByNodeUpwards(TLPhysics::TPhysicsNode* pNode,TLMaths::TQuadTreeZone* pCollisionZone,TLMaths::TQuadTreeZone* pNodeZone,TLMaths::TQuadTreeZone* pPreviousParentZone,Bool TestChildZones,Bool TestNodeZone,u32& CollisionTestCounter)
{
	Bool NodeIsStatic = pNode->IsStatic();

	if ( TestNodeZone && pNodeZone == pCollisionZone )
		TestNodeZone = FALSE;

	m_Debug_ZonesTested++;

	//	collide with all the nodes in this zone
	TPtrArray<TLMaths::TQuadTreeNode>& ZoneNodes = NodeIsStatic ? pCollisionZone->GetNonStaticNodes() : pCollisionZone->GetNodes();
	for ( u32 n=0;	n<ZoneNodes.GetSize();	n++ )
	{
		TPtr<TLMaths::TQuadTreeNode>& pOtherNodePtr = ZoneNodes[n];
		TLPhysics::TPhysicsNode* pOtherNode = pOtherNodePtr.GetObject<TLPhysics::TPhysicsNode>();

		//	dont collide with self!
		if ( pNode->GetNodeRef() == pOtherNode->GetNodeRef() )
			continue;
		
		/*
		//	to save some collision time, check to see if this OtherNode intersects our node's zone 
		//	this is to skip collision checks with a node in a parent(or parent parent) zone that might be far away
		if ( TestNodeZone )
		{
#ifdef ENABLE_TEST_PARENTZONENODE_INPREVIOUSPARENTZONE
			if ( pPreviousParentZone )
			{
				TFixedArray<u32,4>& OtherNodeChildZones = pOtherNode->m_ChildCollisionZones;
				if ( OtherNodeChildZones.GetSize() )
				{
					Bool IsInPrevParentZone = FALSE;
					for ( u32 c=0;	c<OtherNodeChildZones.GetSize();	c++ )
					{
						TPtr<TCollisionZone>& pOtherNodeChildZone = pCollisionZone->GetChildZones().ElementAt( OtherNodeChildZones[c] );
						if ( pOtherNodeChildZone.GetObject() == pPreviousParentZone )
						{
							IsInPrevParentZone = TRUE;
							break;
						}
					}
					
					if ( !IsInPrevParentZone )
						continue;
				}
			}
#endif
#ifdef ENABLE_TEST_PARENTZONENODE_INMYZONE
			if ( !pNodeZone->IsNodeInZoneShape( pOtherNode ) )
				continue;
#endif
		}
*/
		//	do node collisions
		Bool OtherNodeStatic = pOtherNode->IsStatic();
		if ( NodeIsStatic && !OtherNodeStatic )
		{
			DoStaticCollision( pOtherNode, pNode );
		}
		else if ( !NodeIsStatic && OtherNodeStatic )
		{
			DoStaticCollision( pNode, pOtherNode );
		}
		else if ( NodeIsStatic && OtherNodeStatic )
		{
			TLDebug_Break("shouldnt occur?");
		}
		else
		{
			DoCollision( pNode, pOtherNode );
		}
		CollisionTestCounter++;
	}
}



//----------------------------------------------------------
//	check all the collision objects exist as they should
//----------------------------------------------------------
Bool TLPhysics::Debug_CheckCollisionObjects(TLPhysics::TPhysicsNode* pNodeA,TLPhysics::TPhysicsNode* pNodeB)
{
	if ( !pNodeA || !pNodeB )
	{
		TLDebug_Break("Physics nodes expected");
		return FALSE;
	}

	TPtr<TCollisionShape>& pShapePtrA = pNodeA->GetCollisionShape();
	TPtr<TCollisionShape>& pShapePtrB = pNodeB->GetCollisionShape();
	if ( !pShapePtrA.IsValid() || !pShapePtrB.IsValid() )
	{
		TLDebug_Break("Physics nodes collision objects expected");
		return FALSE;
	}

	return TRUE;
}


//----------------------------------------------------------
//	
//----------------------------------------------------------
void TLPhysics::TPhysicsgraph::DoCollision(TLPhysics::TPhysicsNode* pNodeA,TLPhysics::TPhysicsNode* pNodeB)
{
	m_Debug_CollisionTestCount++;

	//	check params
//	if ( !Debug_CheckCollisionObjects( pNodeA, pNodeB ) )
//		return;

	//	get world-collision shapes
	TCollisionShape* pNodeAWorldCollisionShape = pNodeA->CalcWorldCollisionShape();
	if ( !pNodeAWorldCollisionShape )
		return;

	TCollisionShape* pNodeBWorldCollisionShape = pNodeB->CalcWorldCollisionShape();
	if ( !pNodeBWorldCollisionShape )
		return;

	//	calc accumulated movement if required
	if ( !pNodeA->IsAccumulatedMovementValid() )
		pNodeA->CalcAccumulatedMovement();

	if ( !pNodeB->IsAccumulatedMovementValid() )
		pNodeB->CalcAccumulatedMovement();

	//	reset intersection data
	pNodeA->m_Temp_Intersection.Reset();
	pNodeB->m_Temp_Intersection.Reset();

	//	do intersection test
	if ( !pNodeAWorldCollisionShape->GetIntersection( pNodeBWorldCollisionShape, pNodeA->m_Temp_Intersection, pNodeB->m_Temp_Intersection ) )
		return;
	
	TLDebug_CheckFloat( pNodeA->m_Temp_Intersection.m_Intersection );
	TLDebug_CheckFloat( pNodeB->m_Temp_Intersection.m_Intersection );

	//	actual intersection
	m_Debug_CollisionIntersections++;
	
	//	re-act to collision
	if ( !pNodeA->GetPhysicsFlags().IsSet( TLPhysics::TPhysicsNode::Flag_Static ) )
		pNodeA->OnCollision( pNodeB );

	if ( !pNodeB->GetPhysicsFlags().IsSet( TLPhysics::TPhysicsNode::Flag_Static ) )
		pNodeB->OnCollision( pNodeA );
	
}

//----------------------------------------------------------
//
//----------------------------------------------------------
void TLPhysics::TPhysicsgraph::DoStaticCollision(TLPhysics::TPhysicsNode* pNodeA,TLPhysics::TPhysicsNode* pStaticNode)
{
	m_Debug_StaticCollisionTestCount++;
	pNodeA->m_Debug_StaticCollisions++;

	DoCollision( pNodeA, pStaticNode );
}


//----------------------------------------------------------
//
//----------------------------------------------------------
void TLPhysics::TPhysicsgraph::OnNodeRemoving(TPtr<TLPhysics::TPhysicsNode>& pNode)
{
	//	remove node from zones
	//TPtr<TLMaths::TQuadTreeZone> pNullZone;
	//TPtr<TLMaths::TQuadTreeNode> pQuadTreeNode = pNode;
	//pNode->GetZoneNode().SetZone( pNullZone, pQuadTreeNode, NULL );
	pNode->SetZoneNone();

	//	inherited OnRemoving()
	TLGraph::TGraph<TLPhysics::TPhysicsNode>::OnNodeRemoving( pNode );
}


//----------------------------------------------------------
//
//----------------------------------------------------------
void TLPhysics::TPhysicsgraph::OnNodeAdded(TPtr<TLPhysics::TPhysicsNode>& pNode)
{
	TTempString DebugString("Added node to physics graph: ");
	pNode->GetNodeRef().GetString( DebugString );
	TLDebug_Print( DebugString );

	//	initialise zone
	if ( m_pRootCollisionZone )
	{
		m_pRootCollisionZone->AddNode( pNode->GetZoneNodePtr(), m_pRootCollisionZone, TRUE );
	}

	//	inherited OnAdded()
	TLGraph::TGraph<TLPhysics::TPhysicsNode>::OnNodeAdded( pNode );
}


//----------------------------------------------------------
//	set a new root collision zone
//----------------------------------------------------------
void TLPhysics::TPhysicsgraph::SetRootCollisionZone(TPtr<TLMaths::TQuadTreeZone>& pZone)
{
	//	clean up old zone tree
	if ( m_pRootCollisionZone && pZone )
	{
		TLDebug_Break("todo: clean up old collision tree");
		m_pRootCollisionZone = NULL;
	}

	//	set new root zone
	m_pRootCollisionZone = pZone;

	//	pre-divide tree
#ifdef PREDIVIDE_COLLISION_ZONES
	if ( m_pRootCollisionZone )
	{
		m_pRootCollisionZone->DivideAll( m_pRootCollisionZone );
	}
#endif

}


//-------------------------------------------------
//	world up has changed, recalc the normal
//-------------------------------------------------
void TLPhysics::TPhysicsgraph::CalcWorldUpNormal()
{
	//	just for now, we use a float2
	float2 NewNormal( g_WorldUp.x, g_WorldUp.y );
	float NormalLenSq = NewNormal.LengthSq();
	
	//	new values are too small, dont change
	if ( NormalLenSq < TLMaths::g_NearZero )
		return;
	
	//	valid values, normalise
	NewNormal.Normalise( TLMaths::Sqrtf(NormalLenSq), 1.f );
	g_WorldUpNormal.x = NewNormal.x;
	g_WorldUpNormal.y = NewNormal.y;
	g_WorldUpNormal.z = 0.f;
}







TLPhysics::TPhysicsNode* TLPhysics::TPhysicsNodeFactory::CreateObject(TRefRef InstanceRef,TRefRef TypeRef)
{
	if ( TypeRef == "Sphere" )
		return new TLPhysics::TPhysicsNodeSphere(InstanceRef,TypeRef);

	return NULL;
}

