#include "TJoint.h"
#include "TPhysicsNode.h"
#include "TPhysicsgraph.h"

#include <box2d/include/box2d.h>


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

	b2Vec2 WorldAnchor1 = pBodyA->GetWorldPoint( b2Vec2( m_JointPosA.x, m_JointPosA.y ) );
	b2Vec2 WorldAnchor2 = pBodyB->GetWorldPoint( b2Vec2( m_JointPosB.x, m_JointPosB.y ) );
	JointDef.Initialize( pBodyA, pBodyB, WorldAnchor1, WorldAnchor2 );

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
