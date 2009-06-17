/*

	physics node graph - 
	todo:
		turn into a quad/oct/bsp tree via nodes to replace the collision zones

*/
#pragma once

#include <TootleCore/TLGraph.h>
#include "TLPhysics.h"
#include "TPhysicsNode.h"
#include <box2d/include/box2d.h>


namespace TLPhysics
{
	class TPhysicsgraph;
	class TPhysicsNode;
	class TPhysicsNodeFactory;
	class TJoint;					//	joint between two physics nodes.

	extern TPtr<TPhysicsgraph> g_pPhysicsgraph;

	class TPhysics_ContactFilter;		//	custom box2D contact filterer
	class TPhysics_ContactListener;		//	custom box2D contact listener
};




//-----------------------------------------------------
//	custom box2D contact filterer
//-----------------------------------------------------
class TLPhysics::TPhysics_ContactFilter : public b2ContactFilter
{
	virtual bool ShouldCollide(b2Shape* shape1, b2Shape* shape2);
};


class TLPhysics::TPhysics_ContactListener : public b2ContactListener
{
public:
	virtual void	Add(const b2ContactPoint* point);		// handle add point - pre-solver.	gr: new collision
	virtual void	Persist(const b2ContactPoint* point)	{}	// handle persist point	 - pre-solver.	gr: collision still exists
	virtual void	Remove(const b2ContactPoint* point)		{}	// handle remove point - pre-solver.	gr: no more collision
	virtual void	Result(const b2ContactResult* point)	{}	// handle results - post solver. can occur multiple times
};


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


//-----------------------------------------------------
//	TPhysicsgraph class
//-----------------------------------------------------
class TLPhysics::TPhysicsgraph : public TLGraph::TGraph<TLPhysics::TPhysicsNode>
{
public:
	TPhysicsgraph(TRefRef refManagerID);

	virtual SyncBool		Initialise();
	virtual void			UpdateGraph(float TimeStep);
	
	void							SetRootCollisionZone(TPtr<TLMaths::TQuadTreeZone>& pZone,Bool AllowSleep=TRUE);	//	set a new root collision zone. Allow sleep to speed up idle objects, BUT without gravity, joints don't update/constrain properly... looking for a good soluition to this
	TPtr<TLMaths::TQuadTreeZone>&	GetRootCollisionZone()										{	return m_pRootCollisionZone;	}
	
	FORCEINLINE TPtr<b2World>&		GetWorld()						{	return m_pWorld;	}				//	box2d's world
	FORCEINLINE void				AddJoint(const TJoint& Joint)	{	m_NodeJointQueue.Add( Joint );	};	//	add a joint to be created on next update

	// Test routines
	FORCEINLINE void		SetGravityX(float fValue)		{	if ( g_WorldUp.x == fValue )	return;		g_WorldUp.x = fValue;	CalcWorldUpNormal();	}
	FORCEINLINE void		SetGravityY(float fValue)		{	if ( g_WorldUp.y == fValue )	return;		g_WorldUp.y = fValue;	CalcWorldUpNormal();	}
	FORCEINLINE void		SetGravityZ(float fValue)		{	if ( g_WorldUp.z == fValue )	return;		g_WorldUp.z = fValue;	CalcWorldUpNormal();	}
	
protected:
	virtual void			OnNodeRemoving(TPtr<TLPhysics::TPhysicsNode>& pNode);
	virtual void			OnNodeAdded(TPtr<TLPhysics::TPhysicsNode>& pNode,Bool SendAddedMessage);

	void					DoCollisionsByNode();							//	do all the object-object collison iterations
	void					DoCollisionsByNode(TPtr<TLPhysics::TPhysicsNode>& pNode);		//	do collision tests for this node
	void					DoCollisionsByNode(TLPhysics::TPhysicsNode* pNode,TLMaths::TQuadTreeZone* pCollisionZone,TLMaths::TQuadTreeZone* pNodeZone,TLMaths::TQuadTreeZone* pPreviousParentZone,Bool TestChildZones,Bool TestNodeZone,u32& CollisionTestCounter);	//	do collision tests for this node

	void					DoCollisionsByNodeUpwards();
	void					DoCollisionsByNodeUpwards(TPtr<TLPhysics::TPhysicsNode>& pNode);		//	do collision tests for this node
	void					DoCollisionsByNodeUpwards(TLPhysics::TPhysicsNode* pNode,TLMaths::TQuadTreeZone* pCollisionZone,TLMaths::TQuadTreeZone* pNodeZone,TLMaths::TQuadTreeZone* pPreviousParentZone,Bool TestChildZones,Bool TestNodeZone,u32& CollisionTestCounter);	//	do collision tests for this node

	void					DoCollisionsByZone(TLMaths::TQuadTreeZone* pCollisionZone);
	void					DoCollisionsByZone(TLMaths::TQuadTreeZone* pCollisionZone,TPhysicsNode& Node,Bool IsNodesZone,u32 FirstNode);

	void					DoCollision(TLPhysics::TPhysicsNode& NodeA,TLPhysics::TPhysicsNode& NodeB);
	void					DoStaticCollision(TLPhysics::TPhysicsNode& NodeA,TLPhysics::TPhysicsNode& StaticNode);

	void					CalcWorldUpNormal();					//	world up has changed, recalc the normal
	
	SyncBool				CreateJoint(const TJoint& Joint);		//	create joint. if Wait then we're waiting for a node to be created still
	void					RemoveJoint(TRefRef NodeA,TRefRef NodeB);	//	remove joint between these two nodes
	void					RemoveJoint(TRefRef NodeA);				//	remove all joints involving this node

public:
	u32						m_Debug_CollisionTestCount;				//	collision test count
	u32						m_Debug_StaticCollisionTestCount;		//	collision test count
	u32						m_Debug_CollisionTestDupeSavedCount;	//	collision test count
	u32						m_Debug_CollisionIntersections;			//	number of actual intersections
	u32						m_Debug_InZoneTests;
	u32						m_Debug_InZoneTestsFailed;
	u32						m_Debug_CollisionTestsUpwards;			//	number of collision tests we do after going UP the tree
	u32						m_Debug_CollisionTestsDownwards;		//	number of collision tests we do after going DOWN the zone tree
	u32						m_Debug_ZonesTested;					//	total number of zones that were looped through for collisions

protected:
	TPtr<TLMaths::TQuadTreeZone>	m_pRootCollisionZone;			//	collision zone tree
	TPtr<b2World>					m_pWorld;						//	box2d's world
	TArray<TJoint>					m_NodeJoints;					//	list of joints created
	TArray<TJoint>					m_NodeJointQueue;				//	list of joints that are to be created in the next update

private:
	TPhysics_ContactFilter			m_ContactFilter;				//	instance of our custom box2d contact filter
	TPhysics_ContactListener		m_ContactListener;				//	instance of our custom box2d contact listener
};




//----------------------------------------------------------
//	Generic physics node factory
//----------------------------------------------------------
class TLPhysics::TPhysicsNodeFactory : public TClassFactory<TPhysicsNode,FALSE>
{
public:
	virtual TPhysicsNode*	CreateObject(TRefRef InstanceRef,TRefRef TypeRef);
};

