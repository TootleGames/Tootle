#include "TPhysicsgraph.h"
#include "TPhysicsNode.h"
#include "TPhysicsNodeSphere.h"

#include <TootleCore/TLTime.h>


#define MAX_PHYSICS_TIMESTEP	0.3f	//	max step is 20/60 ish...


#define BOX2D_ITERATIONS		10	//	constraint/collision iterations



//	collision method to use
#ifndef USE_BOX2D
	#define DO_COLLISIONS_BY_ZONE
	//#define DO_COLLISIONS_BY_NODE
	//#define DO_COLLISIONS_BY_NODEUPWARDS
#endif

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






//-----------------------------------------------------
//	custom box2D contact filterer
//-----------------------------------------------------
bool TLPhysics::TPhysics_ContactFilter::ShouldCollide(b2Shape* shape1, b2Shape* shape2)
{
	const b2FilterData& filter1 = shape1->GetFilterData();
	const b2FilterData& filter2 = shape2->GetFilterData();

	//	increment collision test counter
	TLCounter::Debug_Increment( TRef_Static(C,o,T,s,t) );

	//	either is in a negative group... no collision
	if ( filter1.groupIndex < 0 || filter2.groupIndex < 0 )
		return FALSE;

	if (filter1.groupIndex == filter2.groupIndex && filter1.groupIndex != 0)
	{
		//	always collide when in same non-zero group (default box2d behaviour)
		return TRUE;
	}

	bool collide = (filter1.maskBits & filter2.categoryBits) != 0 && (filter1.categoryBits & filter2.maskBits) != 0;
	return collide;
}




TLPhysics::TJoint::TJoint() :
	m_CollisionBetweenNodes	( FALSE ),
	m_pJoint				( NULL )
{
}


//----------------------------------------------------------
//	create joint in box2d world - returns WAIT if a node is missing
//----------------------------------------------------------
SyncBool TLPhysics::TJoint::CreateJoint(b2World& World,TPhysicsgraph& PhysicsGraph)
{
	//	get the bodies of the nodes
	TPhysicsNode* pNodeA = PhysicsGraph.FindNode( m_NodeA );
	TPhysicsNode* pNodeB = PhysicsGraph.FindNode( m_NodeB );
	b2Body* pBodyA = pNodeA ? pNodeA->GetBody() : NULL;
	b2Body* pBodyB = pNodeB ? pNodeB->GetBody() : NULL;

	//	missing body/node[s]
	if ( !pBodyA || !pBodyB )
	{
		//	gr: probably waiting for node to be initialised with a shape
		//TLDebug_Break("Missing nodes/bodies when creating a joint");
		return SyncWait;
	}

	//	init definition
	b2DistanceJointDef JointDef;
	JointDef.collideConnected = m_CollisionBetweenNodes;
	JointDef.InitializeLocal( pBodyA, pBodyB, b2Vec2( m_JointPosA.x, m_JointPosA.y ), b2Vec2( m_JointPosB.x, m_JointPosB.y ) );

//	JointDef.dampingRatio = 1.0f;
//	JointDef.frequencyHz = 0.9f;
	JointDef.dampingRatio = 0.0f;
	JointDef.frequencyHz = 0.0f;

	//	instance joint
	m_pJoint = World.CreateJoint( &JointDef );
	
	//	failed to create joint
	if ( !m_pJoint )
		return SyncFalse;

	return SyncTrue;
}


//----------------------------------------------------------
//	remove joint from box2d world
//----------------------------------------------------------
void TLPhysics::TJoint::DestroyJoint(b2World& World)
{
	if ( m_pJoint )
	{
		World.DestroyJoint( m_pJoint );
		m_pJoint = NULL;
	}
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
	TLTime::TScopeTimer Timer( TRef_Static(P,h,y,s,c) );

	TLMaths::Limit( fTimeStep, 0.f, MAX_PHYSICS_TIMESTEP );
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

	// Process all queued messages first
	ProcessMessageQueue();
	
	//	create queued joints
	for ( s32 j=m_NodeJointQueue.GetLastIndex();	j>=0;	j-- )
	{
		SyncBool Result = CreateJoint( m_NodeJointQueue[j] );

		//	keep the joint in the queue if it returns wait
		if ( Result != SyncWait )
			m_NodeJointQueue.RemoveAt( j );
	}

	//	no root? nothing to do
	TPtr<TLPhysics::TPhysicsNode>& pRootNode = GetRootNode();
	if ( pRootNode )
	{
		//	do update
		{
			TLTime::TScopeTimer Timer( TRef_Static(p,h,u,p,d) );
			pRootNode->UpdateAll( fTimeStep );
		}

#ifdef USE_BOX2D
		{
			TLTime::TScopeTimer Timer( TRef_Static(C,o,l,l,i) );

			//	box2d prefers fixed timesteps... lets see how our variable rate goes...
			float timeStep = fTimeStep;			//	1.0f / 60.0f;
			s32 iterations = BOX2D_ITERATIONS;	//	10

			if ( m_pWorld )
			{
				m_pWorld->Step( timeStep, iterations );
			}
		}
#endif

		//	do collisions
#ifdef DO_COLLISIONS_BY_ZONE
		{
			TLTime::TScopeTimer Timer( TRef_Static(C,o,l,l,i) );
			if ( m_pRootCollisionZone )
				DoCollisionsByZone( m_pRootCollisionZone.GetObject() );
		}
#endif

#ifdef DO_COLLISIONS_BY_NODE
		DoCollisionsByNode();
#endif

#ifdef DO_COLLISIONS_BY_NODEUPWARDS
		DoCollisionsByNodeUpwards();
#endif

		//	do post update
		{
			TLTime::TScopeTimer Timer( TRef_Static(p,p,o,s,t) );
			pRootNode->PostUpdateAll( fTimeStep, *this, pRootNode );
		}
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
		TPhysicsNode& Node = *ZoneNodes[n].GetObject<TPhysicsNode>();
		if ( !Node.IsEnabled() )
			continue;
		//if ( pNode->IsStatic() )
		//	continue;

		//	check node can do shape tests atm (could be temporary)
		if ( !Node.HasZoneShape() )
			continue;

		//	do collisions for each node in this zone
		DoCollisionsByZone( pCollisionZone, Node, TRUE, n+1 );
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
void TLPhysics::TPhysicsgraph::DoCollisionsByZone(TLMaths::TQuadTreeZone* pCollisionZone,TPhysicsNode& Node,Bool IsNodesZone,u32 FirstNode)
{
	Bool NodeIsStatic = Node.IsStatic();

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
		TPhysicsNode& OtherNode = *ZoneNodes[o].GetObject<TPhysicsNode>();

		//	skip test against disabled node
		if ( !OtherNode.IsEnabled() )
			continue;
	
		if ( IsNodesZone && &Node==&OtherNode )
			continue;

		Bool OtherNodeIsStatic = OtherNode.IsStatic();
	
		//	do node collisions
		if ( !NodeIsStatic && OtherNodeIsStatic )
		{
			DoStaticCollision( Node, OtherNode );
		}
		else if ( NodeIsStatic && !OtherNodeIsStatic )
		{
			DoStaticCollision( OtherNode, Node );
		}
		else if ( NodeIsStatic && OtherNodeIsStatic )
		{
			//	no collision test!
		}
		else
		{
			DoCollision( Node, OtherNode );
		}
	}

	//	collide with nodes in child zones - this is using the list of zones we know that this zone overlaps (will be at least 2)
	TPtrArray<TLMaths::TQuadTreeZone>& ZoneChildZones = pCollisionZone->GetChildZones();
	TPtrArray<TLMaths::TQuadTreeZone>& ZoneChildZonesWithNodes = NodeIsStatic ? pCollisionZone->GetChildZonesWithNonStaticNodes() : pCollisionZone->GetChildZonesWithNodes();
	if ( IsNodesZone && ZoneChildZones.GetSize() )
	{
		TFixedArray<u32,4>& NodeChildZones = Node.GetChildZones();
		for ( u32 c=0;	c<NodeChildZones.GetSize();	c++ )
		{
			TLMaths::TQuadTreeZone* pNodeChildZone = ZoneChildZones[NodeChildZones[c]].GetObject();
			
			//	if we're a static node, and this child zone only has static nodes, dont do the collision tests
			if ( NodeIsStatic && !pNodeChildZone->HasAnyNonStaticNodesTotal() )
				continue;

			//	if there are no nodes at all, skip the check
			if ( !pNodeChildZone->HasAnyNodesTotal() )
				continue;

			if ( !pNodeChildZone->IsNodeInZoneShape( Node/*, FALSE*/ ) )
			{
			//	TLDebug_Break("Should be intersecting this zone");
				continue;
			}

			DoCollisionsByZone( pNodeChildZone, Node, FALSE, 0 );
		}
	}
	else if ( !IsNodesZone && ZoneChildZonesWithNodes.GetSize() )
	{
		//	don't know what child zones our node intersects, so check all the child zones with any nodes in them
		for ( u32 c=0;	c<ZoneChildZonesWithNodes.GetSize();	c++ )
		{
			TLMaths::TQuadTreeZone* pNodeChildZone = ZoneChildZonesWithNodes[c].GetObject();

			//	check our node intersects with this child zone
			if ( pNodeChildZone->IsNodeInZoneShape( Node/*, TRUE*/ ) )
				DoCollisionsByZone( pNodeChildZone, Node, FALSE, 0 );
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
	TLPhysics::TPhysicsNode& Node = *pNode;
	if ( TestNodeZone && pNodeZone == pCollisionZone )
		TestNodeZone = FALSE;

	m_Debug_ZonesTested++;

	//	collide with all the nodes in this zone
	TPtrArray<TLMaths::TQuadTreeNode>& ZoneNodes = pCollisionZone->GetNodes();
	for ( u32 n=0;	n<ZoneNodes.GetSize();	n++ )
	{
		TPhysicsNode& OtherNode = *(ZoneNodes[n].GetObject<TPhysicsNode>());

		//	dont collide with self!
		if ( Node.GetQuadTreeNodeRef() == OtherNode.GetQuadTreeNodeRef() )
			continue;
		
		//	to save some collision time, check to see if this OtherNode intersects our node's zone 
		//	this is to skip collision checks with a node in a parent(or parent parent) zone that might be far away
		if ( TestNodeZone )
		{
#ifdef ENABLE_TEST_PARENTZONENODE_INPREVIOUSPARENTZONE
			if ( pPreviousParentZone )
			{
				TFixedArray<u32,4>& OtherNodeChildZones = OtherNode.GetChildZones();
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
			if ( !pNodeZone->IsNodeInZoneShape( OtherNode/*, TRUE*/ ) )
				continue;
#endif
		}

		//	do node collisions
		if ( OtherNode.IsStatic() )
			DoStaticCollision( Node, OtherNode );
		else
			DoCollision( Node, OtherNode );
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
			if ( !pChildCollisionZone->IsNodeInZoneShape( *pNode/*, TRUE*/ ) )
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
	TLPhysics::TPhysicsNode& Node = *pNode;
	Bool NodeIsStatic = Node.IsStatic();

	if ( TestNodeZone && pNodeZone == pCollisionZone )
		TestNodeZone = FALSE;

	m_Debug_ZonesTested++;

	//	collide with all the nodes in this zone
	TPtrArray<TLMaths::TQuadTreeNode>& ZoneNodes = NodeIsStatic ? pCollisionZone->GetNonStaticNodes() : pCollisionZone->GetNodes();
	for ( u32 n=0;	n<ZoneNodes.GetSize();	n++ )
	{
		TLPhysics::TPhysicsNode& OtherNode = *(ZoneNodes[n].GetObject<TLPhysics::TPhysicsNode>());

		//	dont collide with self!
		if ( Node.GetNodeRef() == OtherNode.GetNodeRef() )
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
		Bool OtherNodeStatic = OtherNode.IsStatic();
		if ( NodeIsStatic && !OtherNodeStatic )
		{
			DoStaticCollision( OtherNode, Node );
		}
		else if ( !NodeIsStatic && OtherNodeStatic )
		{
			DoStaticCollision( Node, OtherNode );
		}
		else if ( NodeIsStatic && OtherNodeStatic )
		{
			TLDebug_Break("shouldnt occur?");
		}
		else
		{
			DoCollision( Node, OtherNode );
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

	TPtr<TLMaths::TShape>& pShapePtrA = pNodeA->GetCollisionShape();
	TPtr<TLMaths::TShape>& pShapePtrB = pNodeB->GetCollisionShape();
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
void TLPhysics::TPhysicsgraph::DoCollision(TLPhysics::TPhysicsNode& NodeA,TLPhysics::TPhysicsNode& NodeB)
{
	m_Debug_CollisionTestCount++;

	//	check params
//	if ( !Debug_CheckCollisionObjects( pNodeA, pNodeB ) )
//		return;

	//	get world-collision shapes
	TLMaths::TShape* pNodeAWorldCollisionShape = NodeA.CalcWorldCollisionShape();
	if ( !pNodeAWorldCollisionShape )
		return;

	TLMaths::TShape* pNodeBWorldCollisionShape = NodeB.CalcWorldCollisionShape();
	if ( !pNodeBWorldCollisionShape )
		return;

	//	calc accumulated movement if required
	if ( !NodeA.IsAccumulatedMovementValid() )
		NodeA.CalcAccumulatedMovement();

	if ( !NodeB.IsAccumulatedMovementValid() )
		NodeB.CalcAccumulatedMovement();

	//	reset intersection data
	NodeA.m_Temp_Intersection.Reset();
	NodeB.m_Temp_Intersection.Reset();

	//	do intersection test
	if ( !pNodeAWorldCollisionShape->GetIntersection( *pNodeBWorldCollisionShape, NodeA.m_Temp_Intersection, NodeB.m_Temp_Intersection ) )
		return;
	
	TLDebug_CheckFloat( NodeA.m_Temp_Intersection.m_Intersection );
	TLDebug_CheckFloat( NodeB.m_Temp_Intersection.m_Intersection );

	//	actual intersection
	m_Debug_CollisionIntersections++;
	
	//	re-act to collision
	if ( !NodeA.GetPhysicsFlags().IsSet( TLPhysics::TPhysicsNode::Flag_Static ) )
	{
		if ( NodeA.OnCollision( NodeB ) )
		{
			NodeA.AddCollisionInfo( NodeB, NodeA.m_Temp_Intersection );
		}
	}
	else
	{
		NodeA.AddCollisionInfo( NodeB, NodeA.m_Temp_Intersection );
	}


	if ( !NodeB.GetPhysicsFlags().IsSet( TLPhysics::TPhysicsNode::Flag_Static ) )
	{
		if ( NodeB.OnCollision( NodeA ) )
		{
			NodeB.AddCollisionInfo( NodeA, NodeB.m_Temp_Intersection );
		}
	}
	else
	{
		NodeB.AddCollisionInfo( NodeA, NodeB.m_Temp_Intersection );
	}

	
}

//----------------------------------------------------------
//
//----------------------------------------------------------
void TLPhysics::TPhysicsgraph::DoStaticCollision(TLPhysics::TPhysicsNode& NodeA,TLPhysics::TPhysicsNode& StaticNode)
{
	TLCounter::Increment( TRef_Static(S,C,o,l,t) );

	DoCollision( NodeA, StaticNode );
}


//----------------------------------------------------------
//
//----------------------------------------------------------
void TLPhysics::TPhysicsgraph::OnNodeRemoving(TPtr<TLPhysics::TPhysicsNode>& pNode)
{
	//	remove node from zones
	TPtr<TLMaths::TQuadTreeNode> pQuadTreeNode = pNode;
	pNode->SetZone( TLPtr::GetNullPtr<TLMaths::TQuadTreeZone>(), pQuadTreeNode, NULL );

	//	inherited OnRemoving()
	TLGraph::TGraph<TLPhysics::TPhysicsNode>::OnNodeRemoving( pNode );
}


//----------------------------------------------------------
//
//----------------------------------------------------------
void TLPhysics::TPhysicsgraph::OnNodeAdded(TPtr<TLPhysics::TPhysicsNode>& pNode)
{
	//	inherited OnAdded()
	//	gr: do the inherited OnNodeAdded first, this means the Initialise() will be called BEFORE we
	//		try to add it to the zone. ie. initialise collision shape from Init before using shape tests in zone code
	//		if this is an issue for something, then just manually call ProcessMessageQueue and then move this back
	//		to the end of OnNodeAdded
	TLGraph::TGraph<TLPhysics::TPhysicsNode>::OnNodeAdded( pNode );

#ifdef _DEBUG
	TTempString DebugString("Added node to physics graph: ");
	pNode->GetNodeRef().GetString( DebugString );
	TLDebug_Print( DebugString );
#endif

	//	create box2d body
	if ( m_pWorld )
	{
		pNode->CreateBody( *m_pWorld );
	}

	//	initialise zone
	if ( m_pRootCollisionZone )
	{
		TPtr<TLMaths::TQuadTreeNode> pQuadTreeNode = pNode;
		if ( pQuadTreeNode->HasZoneShape() )
		{
			m_pRootCollisionZone->AddNode( pQuadTreeNode, m_pRootCollisionZone, TRUE );
		}
	}
}


//----------------------------------------------------------
//	set a new root collision zone
//----------------------------------------------------------
void TLPhysics::TPhysicsgraph::SetRootCollisionZone(TPtr<TLMaths::TQuadTreeZone>& pZone,Bool AllowSleep)
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

	//	create box2d world[bounds]
	if ( m_pRootCollisionZone )
	{
		const TLMaths::TBox2D& WorldShape = m_pRootCollisionZone->GetShape();
		b2AABB WorldBox;
		WorldBox.lowerBound.Set( WorldShape.GetLeft(), WorldShape.GetTop() );
		WorldBox.upperBound.Set( WorldShape.GetRight(), WorldShape.GetBottom() );

		//	gravity is I think is still meters/sec
		//float2 Gravity = g_WorldUpNormal.xy() * TLPhysics::g_GravityMetresSec;
		
		//	gr: gravity is applied by node, not by system
		float2 Gravity( 0.f, 0.f );

		//	create box2d world with the shape of our zone
		//	gr: note our world up is opposite to box2d...
		m_pWorld = new b2World( WorldBox, b2Vec2( Gravity.x, -Gravity.y ), AllowSleep );

		//	set contact filter
		m_pWorld->SetContactFilter( &m_ContactFilter );
	}
	else
	{
		//	delete old world
		m_pWorld = NULL;
	}
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
	if ( NormalLenSq < TLMaths_NearZero )
		return;
	
	//	valid values, normalise
	NewNormal.Normalise( TLMaths::Sqrtf(NormalLenSq), 1.f );
	g_WorldUpNormal.x = NewNormal.x;
	g_WorldUpNormal.y = NewNormal.y;
	g_WorldUpNormal.z = 0.f;
}


//-------------------------------------------------
//	create joint. if Wait then we're waiting for a node to be created still
//-------------------------------------------------
SyncBool TLPhysics::TPhysicsgraph::CreateJoint(const TJoint& Joint)
{
	//	must have a world
	if ( !m_pWorld )
	{
		TLDebug_Break("world expected when adding a joint");
		return SyncFalse;
	}

	//	make sure joint doesnt already exist
	for ( s32 j=m_NodeJoints.GetLastIndex();	j>=0;	j-- )
	{
		TJoint& ExistingJoint = m_NodeJoints[j];

		//	check for matching joint
		if ( ( ExistingJoint.m_NodeA == Joint.m_NodeA && ExistingJoint.m_NodeB == Joint.m_NodeB ) ||
			 ( ExistingJoint.m_NodeA == Joint.m_NodeB && ExistingJoint.m_NodeB == Joint.m_NodeA ) )
		{
			return SyncFalse;
		}
	}


	//	make new joint 
	TJoint NewJoint = Joint;

	//	create joint in world
	SyncBool CreateResult = NewJoint.CreateJoint( *m_pWorld, *this );
	if ( CreateResult != SyncTrue )
		return CreateResult;

	//	add joint to list
	m_NodeJoints.Add( NewJoint );

	return SyncTrue;
}


//-------------------------------------------------
//	remove joint between these two nodes
//-------------------------------------------------
void TLPhysics::TPhysicsgraph::RemoveJoint(TRefRef NodeA,TRefRef NodeB)
{
	if ( !m_pWorld )
	{
		TLDebug_Break("World expected when removing joints. Joints must be invalid if world was deleted");
		return;
	}

	for ( s32 j=m_NodeJoints.GetLastIndex();	j>=0;	j-- )
	{
		TJoint& Joint = m_NodeJoints[j];

		//	joint includes these nodes together
		if ( ( Joint.m_NodeA == NodeA && Joint.m_NodeB == NodeB ) ||
			 ( Joint.m_NodeA == NodeB && Joint.m_NodeB == NodeA ) )
		{
			//	destroy node
			Joint.DestroyJoint( *m_pWorld );

			//	remove joint from list
			m_NodeJoints.RemoveAt( j );

			//	gr: can only have 1 link between 2 nodes so won't be any more matches
			return;
		}
	}
}


//-------------------------------------------------
//	remove all joints involving this node
//-------------------------------------------------
void TLPhysics::TPhysicsgraph::RemoveJoint(TRefRef NodeA)
{
	if ( !m_pWorld )
	{
		TLDebug_Break("World expected when removing joints. Joints must be invalid if world was deleted");
		return;
	}

	for ( s32 j=m_NodeJoints.GetLastIndex();	j>=0;	j-- )
	{
		TJoint& Joint = m_NodeJoints[j];

		//	joint involves this node
		if ( Joint.m_NodeA == NodeA || Joint.m_NodeB == NodeA )
		{
			//	destroy node
			Joint.DestroyJoint( *m_pWorld );

			//	remove joint from list
			m_NodeJoints.RemoveAt( j );
		}
	}

}





TLPhysics::TPhysicsNode* TLPhysics::TPhysicsNodeFactory::CreateObject(TRefRef InstanceRef,TRefRef TypeRef)
{
	if ( TypeRef == "Sphere" )
		return new TLPhysics::TPhysicsNodeSphere(InstanceRef,TypeRef);

	return NULL;
}

