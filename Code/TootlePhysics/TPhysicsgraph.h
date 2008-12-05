/*

	physics node graph - 
	todo:
		turn into a quad/oct/bsp tree via nodes to replace the collision zones

*/
#pragma once

#include <TootleCore/TLGraph.h>
#include "TPhysicsNode.h"
#include "TCollisionZone.h"


#define DO_COLLISIONS_BY_ZONE
//#define DO_COLLISIONS_BY_NODE
//#define DO_COLLISIONS_BY_NODEUPWARDS


namespace TLPhysics
{
	class TPhysicsgraph;
	class TIntersection;
	class TPhysicsNode;

	extern TPtr<TPhysicsgraph> g_pPhysicsgraph;
};


//-----------------------------------------------------
//	TPhysicsgraph class
//-----------------------------------------------------
class TLPhysics::TPhysicsgraph : public TLGraph::TGraph<TLPhysics::TPhysicsNode>
{
public:
	TPhysicsgraph(TRefRef refManagerID);

	virtual void			UpdateGraph(float TimeStep);
	
	void					SetRootCollisionZone(TPtr<TLPhysics::TCollisionZone>& pZone);	//	set a new root collision zone
	TPtr<TCollisionZone>&	GetRootCollisionZone()											{	return m_pRootCollisionZone;	}

	// Test routines
	FORCEINLINE void		SetGravityX(float fValue)	{	if ( g_WorldUp.x == fValue )	return;		g_WorldUp.x = fValue;	CalcWorldUpNormal();	}
	FORCEINLINE void		SetGravityY(float fValue)	{	if ( g_WorldUp.y == fValue )	return;		g_WorldUp.y = fValue;	CalcWorldUpNormal();	}
	FORCEINLINE void		SetGravityZ(float fValue)	{	if ( g_WorldUp.z == fValue )	return;		g_WorldUp.z = fValue;	CalcWorldUpNormal();	}
	
protected:
	virtual void			OnNodeRemoving(TPtr<TLPhysics::TPhysicsNode>& pNode);
	virtual void			OnNodeAdded(TPtr<TLPhysics::TPhysicsNode>& pNode);

	void					DoCollisionsByNode();							//	do all the object-object collison iterations
	void					DoCollisionsByNode(TPtr<TLPhysics::TPhysicsNode>& pNode);		//	do collision tests for this node
	void					DoCollisionsByNode(TLPhysics::TPhysicsNode* pNode,TLPhysics::TCollisionZone* pCollisionZone,TLPhysics::TCollisionZone* pNodeZone,TLPhysics::TCollisionZone* pPreviousParentZone,Bool TestChildZones,Bool TestNodeZone,u32& CollisionTestCounter);	//	do collision tests for this node

	void					DoCollisionsByNodeUpwards();
	void					DoCollisionsByNodeUpwards(TPtr<TLPhysics::TPhysicsNode>& pNode);		//	do collision tests for this node
	void					DoCollisionsByNodeUpwards(TLPhysics::TPhysicsNode* pNode,TLPhysics::TCollisionZone* pCollisionZone,TLPhysics::TCollisionZone* pNodeZone,TLPhysics::TCollisionZone* pPreviousParentZone,Bool TestChildZones,Bool TestNodeZone,u32& CollisionTestCounter);	//	do collision tests for this node

	void					DoCollisionsByZone(TLPhysics::TCollisionZone* pCollisionZone);
	void					DoCollisionsByZone(TLPhysics::TCollisionZone* pCollisionZone,TLPhysics::TPhysicsNode* pNode,Bool IsNodesZone,u32 FirstNode);

	void					DoCollision(TLPhysics::TPhysicsNode* pNodeA,TLPhysics::TPhysicsNode* pNodeB);
	void					DoStaticCollision(TLPhysics::TPhysicsNode* pNodeA,TLPhysics::TPhysicsNode* pStaticNode);

	void					CalcWorldUpNormal();					//	world up has changed, recalc the normal
	
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
	TPtr<TCollisionZone>	m_pRootCollisionZone;					//	collision zone tree
};

