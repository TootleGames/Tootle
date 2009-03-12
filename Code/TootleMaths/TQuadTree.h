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



#include <TootleCore/TLTypes.h>
#include <TootleCore/TPtrArray.h>
#include "TBox.h"
#include <TootleCore/TPublisher.h>


namespace TLMaths
{
	class TQuadTreeZone;	//	A zone in the quad tree
	class TQuadTreeNode;	//	member of a zone
}


//---------------------------------------------------------------
//	gr: consider templating this to get rid of the need for virtuals
//---------------------------------------------------------------
class TLMaths::TQuadTreeNode
{
	friend class TLMaths::TQuadTreeZone;
public:
	TQuadTreeNode();
	virtual ~TQuadTreeNode()		{}

	virtual Bool					IsStatic()									{	return FALSE;	}	//	if static we can assume this node won't be traversing zones. Only really used for filtering whne accessing the zones, i.e. by the physics for specific traversal
	virtual SyncBool				IsInZone(const TLMaths::TQuadTreeZone& Zone);	//	do your object's test to see if it intersects at all with this zone's shape, default does shape/shape but you might want something more complex
	virtual SyncBool				IsInShape(const TLMaths::TBox2D& Shape);		//	do your object's test to see if it intersects at all with this zone's shape, default does shape/shape but you might want something more complex
	virtual const TLMaths::TBox2D&	GetZoneShape();									//	get the shape of this node
	FORCEINLINE TRefRef				GetQuadTreeNodeRef() const					{	return m_QuadTreeNodeRef;	}

	FORCEINLINE TPtr<TQuadTreeZone>&		GetZone()								{	return m_pZone;	}
	FORCEINLINE const TPtr<TQuadTreeZone>&	GetZone() const							{	return m_pZone;	}
	FORCEINLINE TFixedArray<u32,4>&			GetChildZones()							{	return m_ChildZones;	}
	FORCEINLINE const TFixedArray<u32,4>&	GetChildZones() const					{	return m_ChildZones;	}

	FORCEINLINE Bool				IsZoneOutOfDate() const							{	return m_IsZoneOutofDate;	}
	FORCEINLINE void				SetZoneOutOfDate(Bool OutOfDate=TRUE)			{	m_IsZoneOutofDate = OutOfDate;	}
	void							UpdateZone(TPtr<TLMaths::TQuadTreeNode> pThis,TPtr<TLMaths::TQuadTreeZone>& pRootZone);	//	if the node has moved, update it's zone. returns TRUE if zone changed

	FORCEINLINE Bool				operator==(TRefRef NodeRef) const				{	return (GetQuadTreeNodeRef() == NodeRef);	}

protected:
	Bool									SetZone(TPtr<TQuadTreeZone>& pZone,TPtr<TQuadTreeNode>& pThis,const TFixedArray<u32,4>* pChildZoneList);
	FORCEINLINE void						SetChildZonesNone()									{	m_ChildZones.Empty();	}
	void									SetChildZones(const TFixedArray<u32,4>& InZones);	//	

protected:
	TPtr<TQuadTreeZone>		m_pZone;			//	collision zone we're in
	TFixedArray<u32,4>		m_ChildZones;		//	zones we're intersecting (of m_pZone), these are direct children of m_pZone. if this is empty we're not intersecting any, otherwise MUST be intersecting more than 1 child zone
	Bool					m_IsZoneOutofDate;	//	flag to say zone needs update
	TRef					m_QuadTreeNodeRef;	//	
};



//---------------------------------------------------------------
//	Quad tree zone
//---------------------------------------------------------------
class TLMaths::TQuadTreeZone : public TLMessaging::TPublisher
{
	friend class TLMaths::TQuadTreeNode;
public:
	TQuadTreeZone(const TLMaths::TBox2D& ZoneShape,TPtr<TQuadTreeZone>& pParent);
	~TQuadTreeZone()			{	Shutdown();	}

	void						Shutdown();				//	because of backwards referencing, we should shutdown before NULL'ing otherwise nothing will get released until graphs/nodes etc are.

	Bool						AddNode(TPtr<TQuadTreeNode>& pNode,TPtr<TQuadTreeZone>& pThis,Bool DoCheckInShape);	//	attempt to add this node to this zone. checks with children first to see if it fits into just one child better. returns FALSE if not in this zone
	FORCEINLINE Bool			IsBelowZone(const TQuadTreeZone* pZone) const		{	return (this == pZone) ? TRUE : (!m_pParent.IsValid()) ? FALSE : m_pParent->IsBelowZone( pZone );	}
	FORCEINLINE Bool			IsBelowZone(const TPtr<TQuadTreeZone>& pZone) const	{	return IsBelowZone( pZone.GetObject() );	}
	FORCEINLINE Bool			IsInZone(const TPtr<TQuadTreeNode>& pNode)			{	return m_Nodes.Exists( pNode.GetObject() );	}	//	test to see if node exists in this zone
	FORCEINLINE Bool			IsInZone(const TQuadTreeNode& Node)					{	return m_Nodes.Exists( &Node );	}	//	test to see if node exists in this zone
	TPtrArray<TQuadTreeNode>&	GetNodes()											{	return m_Nodes;	}
	TPtrArray<TQuadTreeNode>&	GetNonStaticNodes()									{	return m_NonStaticNodes;	}
	TPtrArray<TQuadTreeZone>&	GetChildZones()										{	return m_Children;	}
	const TPtrArray<TQuadTreeZone>&	GetChildZones() const							{	return m_Children;	}
	Bool						HasChildrenAnyNodes()								{	return m_ChildrenWithNodes.GetSize() > 0;	}
	Bool						HasChildrenAnyNonStaticNodes()						{	return m_ChildrenWithNonStaticNodes.GetSize() > 0;	}
	TPtrArray<TQuadTreeZone>&	GetChildZonesWithNodes()							{	return m_ChildrenWithNodes;	}
	TPtrArray<TQuadTreeZone>&	GetChildZonesWithNonStaticNodes()					{	return m_ChildrenWithNonStaticNodes;	}
	TPtr<TQuadTreeZone>&		GetParentZone()										{	return m_pParent;	}
	const TPtr<TQuadTreeZone>&	GetParentZone() const								{	return m_pParent;	}
	u32							GetNodeCountTotal();
	u32							GetNonStaticNodeCountTotal();

	Bool						HasAnyNodes()										{	return (m_Nodes.GetSize() > 0);	}
	Bool						HasAnyNodesTotal()									{	return (m_Nodes.GetSize() > 0) || HasChildrenAnyNodes();	}
	Bool						HasAnyNonStaticNodes()								{	return (m_NonStaticNodes.GetSize() > 0);	}
	Bool						HasAnyNonStaticNodesTotal()							{	return (m_NonStaticNodes.GetSize() > 0) || HasChildrenAnyNonStaticNodes();	}

	SyncBool					IsNodeInZoneShape(TQuadTreeNode* pNode);			//	test to see if this intersects into this zone
	FORCEINLINE SyncBool		IsNodeInZoneShape(TPtr<TQuadTreeNode>& pNode)		{	return IsNodeInZoneShape( pNode.GetObject() );	}
	FORCEINLINE const TLMaths::TBox2D&	GetShape() const							{	return m_Shape;	}

	void						SetSiblingZones(TPtrArray<TQuadTreeZone>& Siblings);	//	assign sibling
	TPtr<TQuadTreeZone>&		GetSiblingZone(u32 SiblingIndex)					{	return m_SiblingZones[SiblingIndex];	}
	u32							GetSiblingParentsChildIndex(u32 SiblingIndex) const	{	return m_SiblingZones[SiblingIndex];	}
	Bool						HasSiblingZones() const								{	return (m_SiblingIndex!=-1);	}	//	our has parent, or m_SiblingZones.GetSIze
	u32							GetParentsChildIndex() const						{	return m_SiblingIndex;	}

	Bool						DivideAll(TPtr<TLMaths::TQuadTreeZone>& pThis);		//	recursively divide until we're at our minimum size leafs

protected:
	void						GetInChildZones(TQuadTreeNode* pNode,TFixedArray<u32,4>& InZones);			//	return which child zone we're in, -1 if none
	void						OnChildZoneNodesChanged();							//	list of nodes in a child has changed

	void						DoAddNode(TPtr<TQuadTreeNode>& pNode);				//	add node to this zone's list
	void						DoRemoveNode(TPtr<TQuadTreeNode>& pNode);			//	remove node from this zone's list

	void						OnZoneStructureChanged();							//	if we subdivide, or delete child zones etc, then call this func - will go up to the root then notify subscribers of the change

private:
	Bool						Divide(TPtr<TLMaths::TQuadTreeZone>& pThis);		//	subdivide ourselves into 4 children

protected:
	TLMaths::TBox2D				m_Shape;				//	shape of the zone
	TPtrArray<TQuadTreeNode>	m_Nodes;				//	nodes in this zone (includes static and non static)
	TPtrArray<TQuadTreeNode>	m_NonStaticNodes;		//	non-static nodes in this zone
	TPtrArray<TQuadTreeZone>	m_Children;				//	child zones
	TPtr<TQuadTreeZone>			m_pParent;				//	parent zone
	TPtrArray<TQuadTreeZone>	m_ChildrenWithNodes;			//	child zones which DO have nodes in them
	TPtrArray<TQuadTreeZone>	m_ChildrenWithNonStaticNodes;	//	child zones which DO have nodes in them that ARE NOT static
	//TPtrArray<TQuadTreeZone>	m_InactiveChildren;		//	former child zones - zones moved to here when we cull them, when we want to divide again we can re-use these
	
	TPtrArray<TQuadTreeZone>	m_SiblingZones;			//	zones next to us (also children of our parent)
	TFixedArray<u32,3>			m_SiblingZoneIndexes;	//	m_SiblingZoneIndexes[SiblingIndex] == Parent's Child[Index]
	s32							m_SiblingIndex;			//	our index in our parent's children. -1 when no parent
};


