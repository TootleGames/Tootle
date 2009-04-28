/*

	physics node graph - 
	todo:
		turn into a quad/oct/bsp tree via nodes to replace the collision zones

*/
#pragma once

#include <TootleCore/TLGraph.h>
#include "TLPhysics.h"
#include "TPhysicsNode.h"


//namespace Box2D
#include <box2d/include/box2d.h>


namespace TLPhysics
{
	class TPhysicsgraph;
	class TPhysicsNode;
	class TPhysicsNodeFactory;

	extern TPtr<TPhysicsgraph> g_pPhysicsgraph;

	class TPhysics_ContactFilter;		//	custom box2D contact filterer
};




//-----------------------------------------------------
//	custom box2D contact filterer
//-----------------------------------------------------
class TLPhysics::TPhysics_ContactFilter : public b2ContactFilter
{
	virtual bool ShouldCollide(b2Shape* shape1, b2Shape* shape2);
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
	
	void							SetRootCollisionZone(TPtr<TLMaths::TQuadTreeZone>& pZone);	//	set a new root collision zone
	TPtr<TLMaths::TQuadTreeZone>&	GetRootCollisionZone()										{	return m_pRootCollisionZone;	}
	
	FORCEINLINE TPtr<b2World>&		GetWorld()			{	return m_pWorld;	}				//	box2d's world

	// Test routines
	FORCEINLINE void		SetGravityX(float fValue)	{	if ( g_WorldUp.x == fValue )	return;		g_WorldUp.x = fValue;	CalcWorldUpNormal();	}
	FORCEINLINE void		SetGravityY(float fValue)	{	if ( g_WorldUp.y == fValue )	return;		g_WorldUp.y = fValue;	CalcWorldUpNormal();	}
	FORCEINLINE void		SetGravityZ(float fValue)	{	if ( g_WorldUp.z == fValue )	return;		g_WorldUp.z = fValue;	CalcWorldUpNormal();	}
	
protected:
	virtual void			OnNodeRemoving(TPtr<TLPhysics::TPhysicsNode>& pNode);
	virtual void			OnNodeAdded(TPtr<TLPhysics::TPhysicsNode>& pNode);

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

private:
	TPhysics_ContactFilter			m_ContactFilter;				//	instance of our custom box2d contact filter
};




//----------------------------------------------------------
//	Generic physics node factory
//----------------------------------------------------------
class TLPhysics::TPhysicsNodeFactory : public TClassFactory<TPhysicsNode,FALSE>
{
public:
	virtual TPhysicsNode*	CreateObject(TRefRef InstanceRef,TRefRef TypeRef);
};

