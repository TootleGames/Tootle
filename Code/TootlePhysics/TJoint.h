#pragma once

#include <TootleCore/TLTypes.h>
#include <TootleCore/TRef.h>

// Forward declarations
namespace TLPhysics
{
	class TJoint;
	class TPhysicsgraph;
}

class b2Joint;
class b2World;

//-----------------------------------------------------
//	class to join two nodes together in whatever way.
//	this class will expand in future for other joint types - maybe be overloaded
//-----------------------------------------------------
class TLPhysics::TJoint
{
public:
	TJoint();

	SyncBool	CreateJoint(b2World& World,TPhysicsgraph& PhysicsGraph);
	void		DestroyJoint(b2World& World);

public:
	TRef		m_NodeA;			//	physics nodes
	TRef		m_NodeB;			//	physics nodes
	float2		m_JointPosA;		//	position relative to the node
	float2		m_JointPosB;		//	position relative to the node
	Bool		m_CollisionBetweenNodes;	//	can explicitly disable collision between these two nodes via joint

private:
	b2Joint*		m_pJoint;		//	joint in world
};