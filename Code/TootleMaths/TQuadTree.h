/*------------------------------------------------------

	This class is a spacial tree used to divide objects(nodes)
	into a specific zone

	Can be used to divide up Physics nodes into collision areas
	or divide render nodes into visible/non visible zones

	Currently it's a QuadTree, but the class could be easily altered
	to become a 3D division system like an OctTree (same division but an 
	extra dimension so the divison into 4 becomes 2 layers of divison)
	or a BSP (non-uniform division)

	The shape/division limitation is essentially the Shape

-------------------------------------------------------*/
#pragma once

/*

#include <TootleCore/TLTypes.h>
#include <TootleCore/TPtrArray.h>
#include "TCollisionShape.h"


namespace TLMaths
{
	class TQuadTreeZone;	//	A zone in the quad tree
	class TQuadTreeNode;	//	member of a zone
}

namespace TLPhysics
{
	class TCollisionBox;
	class TCollisionZone;
	class TPhysicsNode;
}


//---------------------------------------------------------------
//	gr: consider templating this to get rid of the need for virtuals
//---------------------------------------------------------------
class TLMaths::TQuadTreeNode
{
public:
	virtual Bool			IsStatic() const			{	return FALSE;	}	//	if static we can assume this node won't be traversing zones

protected:
	TPtr<TQuadTreeZone>		m_pZone;						//	collision zone we're in
	TFixedArray<u32,4>		m_IntersectingChildZones;		//	zone we're intersecting, these are direct children of m_pZone. if this is empty we're not intersecting any, otherwise MUST be intersecting more than 1 child zone
};



//---------------------------------------------------------------
//	Quad tree zone
//---------------------------------------------------------------
class TLMaths::TQuadTreeZone
{
	friend class TLMaths::TQuadTreeNode;
public:
	TCollisionZone(const TLMaths::TBox2D& ZoneShape,TPtr<TCollisionZone>& pParent);

	Bool						AddNode(TPtr<TPhysicsNode>& pNode,TPtr<TCollisionZone>& pThis,Bool DoCheckInShape);	//	attempt to add this node to this zone. checks with children first to see if it fits into just one child better. returns FALSE if not in this zone
	FORCEINLINE Bool			IsInZone(const TPtr<TPhysicsNode>& pNode)			{	return m_Nodes.Exists( pNode );	}	//	test to see if node exists in this zone
	TPtrArray<TQuadTreeNode>&	GetNodes()											{	return m_Nodes;	}
	TPtrArray<TQuadTreeNode>&	GetNonStaticNodes()									{	return m_NonStaticNodes;	}
	TPtrArray<TQuadTreeZone>&	GetChildZones()										{	return m_Children;	}
	Bool						HasChildrenAnyNodes()								{	return m_ChildrenWithNodes.GetSize() > 0;	}
	Bool						HasChildrenAnyNonStaticNodes()						{	return m_ChildrenWithNonStaticNodes.GetSize() > 0;	}
	TPtrArray<TQuadTreeZone>&	GetChildZonesWithNodes()							{	return m_ChildrenWithNodes;	}
	TPtrArray<TQuadTreeZone>&	GetChildZonesWithNonStaticNodes()					{	return m_ChildrenWithNonStaticNodes;	}
	TPtr<TQuadTreeZone>&		GetParentZone()										{	return m_pParent;	}
	u32							GetNodeCountTotal();
	u32							GetNonStaticNodeCountTotal();

	Bool						HasAnyNodes()										{	return (m_Nodes.GetSize() > 0);	}
	Bool						HasAnyNodesTotal()									{	return (m_Nodes.GetSize() > 0) || HasChildrenAnyNodes();	}
	Bool						HasAnyNonStaticNodes()								{	return (m_NonStaticNodes.GetSize() > 0);	}
	Bool						HasAnyNonStaticNodesTotal()							{	return (m_NonStaticNodes.GetSize() > 0) || HasChildrenAnyNonStaticNodes();	}

	Bool						IsNodeInZoneShape(TLPhysics::TPhysicsNode* pNode,Bool TestAlreadyInZone);			//	test to see if this intersects into this zone
	FORCEINLINE Bool			IsNodeInZoneShape(TPtr<TLPhysics::TPhysicsNode>& pNode,Bool TestAlreadyInZone)	{	return IsNodeInZoneShape( pNode.GetObject(), TestAlreadyInZone );	}
	FORCEINLINE TCollisionBox2D&	GetCollisionShape()									{	return m_CollisionShape;	}

protected:
	void						GetInChildZones(TPhysicsNode* pNode,TFixedArray<u32,4>& InZones);			//	return which child zone we're in, -1 if none
	void						OnChildZoneNodesChanged();							//	list of nodes in a child has changed

	void						DoAddNode(TPtr<TPhysicsNode>& pNode);				//	add node to this zone's list
	void						DoRemoveNode(TPtr<TPhysicsNode>& pNode);			//	remove node from this zone's list

protected:
	TCollisionBox2D				m_CollisionShape;		//	shape of the zone
	TPtrArray<TQuadTreeNode>	m_Nodes;				//	nodes in this zone
	TPtrArray<TQuadTreeNode>	m_NonStaticNodes;		//	non-nodes in this zone
	TPtrArray<TQuadTreeZone>	m_Children;				//	child zones
	TPtr<TQuadTreeZone>			m_pParent;				//	parent zone
	TPtrArray<TQuadTreeZone>	m_ChildrenWithNodes;			//	child zones which DO have nodes in them
	TPtrArray<TQuadTreeZone>	m_ChildrenWithNonStaticNodes;	//	child zones which DO have nodes in them that ARE NOT static
};

*/
