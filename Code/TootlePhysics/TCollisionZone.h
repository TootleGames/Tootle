/*------------------------------------------------------

	collision zone is like an area (collision shape) that 
	contains X collision nodes 
	nodes belong to multiple zones (to allow overlap)
	this is to reduce collision checks between objects that are
	nowhere near each other

-------------------------------------------------------*/
#pragma once


#include <TootleCore/TLTypes.h>
#include <TootleCore/TPtrArray.h>
#include "TCollisionShape.h"


namespace TLMaths
{
	class TBox;
}

namespace TLPhysics
{
	class TCollisionBox;
	class TCollisionZone;
	class TPhysicsNode;
}




class TLPhysics::TCollisionZone
{
	friend class TPhysicsNode;
public:
	TCollisionZone(const TLMaths::TBox2D& ZoneShape,TPtr<TCollisionZone>& pParent);

	Bool						AddNode(TPtr<TPhysicsNode>& pNode,TPtr<TCollisionZone>& pThis,Bool DoCheckInShape);	//	attempt to add this node to this zone. checks with children first to see if it fits into just one child better. returns FALSE if not in this zone
	FORCEINLINE Bool			IsInZone(const TPtr<TPhysicsNode>& pNode)			{	return m_Nodes.Exists( pNode );	}	//	test to see if node exists in this zone
	TPtrArray<TPhysicsNode>&	GetNodes()											{	return m_Nodes;	}
	TPtrArray<TPhysicsNode>&	GetNonStaticNodes()									{	return m_NonStaticNodes;	}
	TPtrArray<TCollisionZone>&	GetChildZones()										{	return m_Children;	}
	Bool						HasChildrenAnyNodes()								{	return m_ChildrenWithNodes.GetSize() > 0;	}
	Bool						HasChildrenAnyNonStaticNodes()						{	return m_ChildrenWithNonStaticNodes.GetSize() > 0;	}
	TPtrArray<TCollisionZone>&	GetChildZonesWithNodes()							{	return m_ChildrenWithNodes;	}
	TPtrArray<TCollisionZone>&	GetChildZonesWithNonStaticNodes()					{	return m_ChildrenWithNonStaticNodes;	}
	TPtr<TCollisionZone>&		GetParentZone()										{	return m_pParent;	}
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
	TPtrArray<TPhysicsNode>		m_Nodes;				//	nodes in this zone
	TPtrArray<TPhysicsNode>		m_NonStaticNodes;		//	non-nodes in this zone
	TPtrArray<TCollisionZone>	m_Children;				//	child zones
	TPtr<TCollisionZone>		m_pParent;				//	parent zone
	TPtrArray<TCollisionZone>	m_ChildrenWithNodes;			//	child zones which DO have nodes in them
	TPtrArray<TCollisionZone>	m_ChildrenWithNonStaticNodes;	//	child zones which DO have nodes in them that ARE NOT static
};


