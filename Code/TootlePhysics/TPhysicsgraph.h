/*

	physics node graph - 
	todo:
		turn into a quad/oct/bsp tree via nodes to replace the collision zones

*/
#pragma once

#include <TootleCore/TLGraph.h>
#include "TLPhysics.h"
#include "TPhysicsNode.h"
#include <TootleMaths/TQuadTree.h>


namespace TLPhysics
{
	class TPhysicsgraph;
	class TPhysicsNode;
	class TPhysicsNodeFactory;
	class TJoint;					//	joint between two physics nodes.

	extern TPtr<TPhysicsgraph> g_pPhysicsgraph;
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
class TLPhysics::TPhysicsgraph : public TLGraph::TGraph<TLPhysics::TPhysicsNode>, public b2ContactListener, public b2ContactFilter
{
public:
	TPhysicsgraph() :
		TLGraph::TGraph<TLPhysics::TPhysicsNode>	( "Physics" )
	{
	}

	virtual SyncBool		Initialise();
	virtual void			UpdateGraph(float TimeStep);
	
	void					SetRootCollisionZone(TPtr<TLMaths::TQuadTreeZone>& pZone,Bool AllowSleep=TRUE);	//	set a new root collision zone. Allow sleep to speed up idle objects, BUT without gravity, joints don't update/constrain properly... looking for a good soluition to this

	void					GetNodesInShape(const TLMaths::TShapePolygon2D& Shape,TArray<TLPhysics::TPhysicsNode*>& NearPhysicsNodes);
	void					GetNodesInShape(const TLMaths::TSphere2D& Shape,TArray<TLPhysics::TPhysicsNode*>& NearPhysicsNodes,Bool StrictSphere);	//	get all the nodes in this shape - for spheres optionally do strict sphere checks - box2D uses Boxes for its query so it can return objects outside the sphere. this does an extra loop to make sure distance is within the radius
	void					GetNodesInShape(const TLMaths::TBox2D& Shape,TArray<TLPhysics::TPhysicsNode*>& NearPhysicsNodes);						//	get all the nodes in this shape

	FORCEINLINE TPtr<b2World>&		GetWorld()						{	return m_pWorld;	}				//	box2d's world
	FORCEINLINE void				AddJoint(const TJoint& Joint)	{	m_NodeJointQueue.Add( Joint );	};	//	add a joint to be created on next update
	FORCEINLINE void				RefilterShape(b2Fixture* pShape)	{	m_RefilterQueue.Add( pShape );	}	//	add to list of shapes that need refiltering

	// Test routines
	FORCEINLINE void		SetGravityX(float fValue)		{	if ( g_WorldUp.x == fValue )	return;		g_WorldUp.x = fValue;	CalcWorldUpNormal();	}
	FORCEINLINE void		SetGravityY(float fValue)		{	if ( g_WorldUp.y == fValue )	return;		g_WorldUp.y = fValue;	CalcWorldUpNormal();	}
	FORCEINLINE void		SetGravityZ(float fValue)		{	if ( g_WorldUp.z == fValue )	return;		g_WorldUp.z = fValue;	CalcWorldUpNormal();	}
	
protected:
	virtual void			OnNodeRemoving(TPtr<TLPhysics::TPhysicsNode>& pNode);
	virtual void			OnNodeAdded(TPtr<TLPhysics::TPhysicsNode>& pNode,Bool SendAddedMessage);

	void					CalcWorldUpNormal();					//	world up has changed, recalc the normal
	
	SyncBool				CreateJoint(const TJoint& Joint);		//	create joint. if Wait then we're waiting for a node to be created still
	void					RemoveJoint(TRefRef NodeA,TRefRef NodeB);	//	remove joint between these two nodes
	void					RemoveJoint(TRefRef NodeA);				//	remove all joints involving this node

private:
	//	box2d listener functions
	virtual void			BeginContact(b2Contact* contact);		//	new collision with shape
	virtual void			EndContact(b2Contact* contact);			//	no longer colliding with shape
	virtual bool			ShouldCollide(b2Fixture* fixtureA, b2Fixture* fixtureB);

protected:
	TPtr<b2World>					m_pWorld;						//	box2d's world
	TArray<TJoint>					m_NodeJoints;					//	list of joints created
	TArray<TJoint>					m_NodeJointQueue;				//	list of joints that are to be created in the next update
	TArray<b2Fixture*>				m_RefilterQueue;				//	queue of box2d shapes that need refiltering
};




//----------------------------------------------------------
//	Generic physics node factory
//----------------------------------------------------------
class TLPhysics::TPhysicsNodeFactory : public TClassFactory<TPhysicsNode,FALSE>
{
public:
	virtual TPhysicsNode*	CreateObject(TRefRef InstanceRef,TRefRef TypeRef);
};

