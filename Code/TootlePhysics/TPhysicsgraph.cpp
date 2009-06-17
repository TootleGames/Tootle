#include "TPhysicsgraph.h"
#include "TPhysicsNode.h"
#include "TPhysicsNodeSphere.h"

#include <TootleCore/TLTime.h>


#define MAX_PHYSICS_TIMESTEP	0.3f	//	max step is 20/60 ish...


#define BOX2D_ITERATIONS		10	//	constraint/collision iterations



#define COLLISION_ITERATIONS	1

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

		//	do box2d world step
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
void TLPhysics::TPhysicsgraph::OnNodeAdded(TPtr<TLPhysics::TPhysicsNode>& pNode,Bool SendAddedMessage)
{
	//	inherited OnAdded()
	//	gr: do the inherited OnNodeAdded first, this means the Initialise() will be called BEFORE we
	//		try to add it to the zone. ie. initialise collision shape from Init before using shape tests in zone code
	//		if this is an issue for something, then just manually call ProcessMessageQueue and then move this back
	//		to the end of OnNodeAdded
	//	gr: DONT send the OnNodeAdded message here, we send it after we've created our body
	TLGraph::TGraph<TLPhysics::TPhysicsNode>::OnNodeAdded( pNode, FALSE );

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

	// Send the added node notificaiton
	if ( SendAddedMessage )
	{
		TLMessaging::TMessage Message("NodeAdded", GetGraphRef());
		Message.Write(pNode->GetNodeRef());

		PublishMessage(Message);
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
		m_pWorld->SetContactListener( &m_ContactListener );
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




//-------------------------------------------------
//	
//-------------------------------------------------
TLPhysics::TPhysicsNode* TLPhysics::TPhysicsNodeFactory::CreateObject(TRefRef InstanceRef,TRefRef TypeRef)
{
	if ( TypeRef == "Sphere" )
		return new TLPhysics::TPhysicsNodeSphere(InstanceRef,TypeRef);

	return NULL;
}



//-------------------------------------------------
// handle add point - pre-solver.	gr: new collision
//-------------------------------------------------
void TLPhysics::TPhysics_ContactListener::Add(const b2ContactPoint* point)
{
	//	get physics node for shape 1
	TPhysicsNode* pNodeA = (TPhysicsNode*)point->shape1->GetBody()->GetUserData();
	TPhysicsNode* pNodeB = (TPhysicsNode*)point->shape2->GetBody()->GetUserData();

	if ( pNodeA )
	{
		TLPhysics::TCollisionInfo* pCollisionInfo = pNodeA->OnCollision();
		if ( pCollisionInfo )
		{
			pCollisionInfo->m_Intersection = float3( point->position.x, point->position.y, 0.f );
			pCollisionInfo->m_OtherNode = pNodeB->GetNodeRef();
			pCollisionInfo->m_OtherNodeOwner = pNodeB->GetOwnerSceneNodeRef();
			pCollisionInfo->m_OtherNodeStatic = pNodeB->IsStatic();
		}
	}

	if ( pNodeB )
	{
		TLPhysics::TCollisionInfo* pCollisionInfo = pNodeB->OnCollision();
		if ( pCollisionInfo )
		{
			pCollisionInfo->m_Intersection = float3( point->position.x, point->position.y, 0.f );
			pCollisionInfo->m_OtherNode = pNodeA->GetNodeRef();
			pCollisionInfo->m_OtherNodeOwner = pNodeA->GetOwnerSceneNodeRef();
			pCollisionInfo->m_OtherNodeStatic = pNodeA->IsStatic();
		}
	}



}
