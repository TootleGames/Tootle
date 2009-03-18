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


#define TQUADTREEZONE_MAX_NEIGHBOUR_ZONES		8	//	whilst neighbour system is just the grid neighbours, there is a max of 8


namespace TLMaths
{
	class TQuadTreeZone;		//	A zone in the quad tree
	class TQuadTreeNode;		//	member of a zone
	class TQuadTreeParams;		//	various configurationy things wrapped up in a class to make it easier to pass around

	namespace TLQuadTree
	{
		//	sibling index to position in our parent. Not a strict ordering system, it just works out this way when we Divide()
		enum TZonePos
		{
			ZonePos_Invalid = -1,
			ZonePos_TopLeft = 0,
			ZonePos_TopRight = 1,
			ZonePos_BottomLeft = 2,
			ZonePos_BottomRight = 3,
		};
	};

}


//---------------------------------------------------------------
//	
//---------------------------------------------------------------
class TLMaths::TQuadTreeParams
{
public:
	TQuadTreeParams(u32 MaxNodesPerZone=2,float MinZoneSize=6.f,Bool CullEmptyZones=FALSE) : 
		m_MaxNodesPerZone	( MaxNodesPerZone ),
		m_MinZoneSize		( MinZoneSize ),
		m_CullEmptyZones	( CullEmptyZones )
	{
	};

public:
	u32			m_MaxNodesPerZone;	//	if we have [or could have] more than this number of nodes in a zone we divide it
	float		m_MinZoneSize;		//	zone cannot be smaller than this upon divide
	Bool		m_CullEmptyZones;	//	delete zones if empty
};



//---------------------------------------------------------------
//	gr: consider templating this to get rid of the need for virtuals
//---------------------------------------------------------------
class TLMaths::TQuadTreeNode
{
	friend class TLMaths::TQuadTreeZone;
public:
	TQuadTreeNode();
	virtual ~TQuadTreeNode()		{}

	virtual Bool					IsStatic()										{	return FALSE;	}	//	if static we can assume this node won't be traversing zones. Only really used for filtering whne accessing the zones, i.e. by the physics for specific traversal
	virtual SyncBool				IsInZone(const TLMaths::TQuadTreeZone& Zone);	//	do your object's test to see if it intersects at all with this zone's shape, default does shape/shape but you might want something more complex
	virtual SyncBool				IsInShape(const TLMaths::TBox2D& Shape);		//	do your object's test to see if it intersects at all with this zone's shape, default does shape/shape but you might want something more complex
	virtual const TLMaths::TBox2D&	GetZoneShape();									//	get the shape of this node
	FORCEINLINE TRefRef				GetQuadTreeNodeRef() const						{	return m_QuadTreeNodeRef;	}
	virtual void					OnZoneWake(SyncBool ZoneActive)					{	}	//	notifcation when zone is set to active (from non-active)
	virtual void					OnZoneSleep()									{	}	//	notifcation when zone is set to non-active (from active)

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
	virtual void							OnZoneChanged(TPtr<TQuadTreeZone>& pOldZone)	{	}	//	zone has changed

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
	TQuadTreeZone(const TLMaths::TBox2D& ZoneShape,const TLMaths::TQuadTreeParams& ZoneParams);
	~TQuadTreeZone()			{	Shutdown();	}

	void						Shutdown();				//	because of backwards referencing, we should shutdown before NULL'ing otherwise nothing will get released until graphs/nodes etc are.

	Bool						AddNode(TPtr<TQuadTreeNode>& pNode,TPtr<TQuadTreeZone>& pThis,Bool DoCheckInShape);	//	attempt to add this node to this zone. checks with children first to see if it fits into just one child better. returns FALSE if not in this zone
	FORCEINLINE Bool			IsBelowZone(const TQuadTreeZone* pZone) const		{	return (this == pZone) ? TRUE : (!m_pParent.IsValid()) ? FALSE : m_pParent->IsBelowZone( pZone );	}
	FORCEINLINE Bool			IsBelowZone(const TPtr<TQuadTreeZone>& pZone) const	{	return IsBelowZone( pZone.GetObject() );	}
	FORCEINLINE Bool			IsInZone(const TPtr<TQuadTreeNode>& pNode)			{	return m_Nodes.Exists( pNode.GetObject() );	}	//	test to see if node exists in this zone
	FORCEINLINE Bool			IsInZone(const TQuadTreeNode& Node)					{	return m_Nodes.Exists( &Node );	}	//	test to see if node exists in this zone
	const TQuadTreeZone*		GetZoneAt(const float2& Position) const;			//	search the tree to find the existing zone at this position - todo: expand to use shape
	void						GetIntersectingLeafZones(const TLMaths::TLine2D& Shape,TArray<const TLMaths::TQuadTreeZone*>& IntersectZones);	//	get a list of all leaf zones that this shape intersects

	TPtr<TQuadTreeZone>&		GetParentZone()										{	return m_pParent;	}
	const TPtr<TQuadTreeZone>&	GetParentZone() const								{	return m_pParent;	}

	TPtrArray<TQuadTreeNode>&	GetNodes()											{	return m_Nodes;	}
	TPtrArray<TQuadTreeNode>&	GetNonStaticNodes()									{	return m_NonStaticNodes;	}
	u32							GetNodeCountTotal();
	u32							GetNonStaticNodeCountTotal();
	TLQuadTree::TZonePos		GetZonePos() const									{	return (TLQuadTree::TZonePos)m_SiblingIndex;	}

	TPtrArray<TQuadTreeZone>&	GetChildZones()										{	return m_Children;	}
	const TPtrArray<TQuadTreeZone>&	GetChildZones() const							{	return m_Children;	}
	Bool						HasChildrenAnyNodes()								{	return m_ChildrenWithNodes.GetSize() > 0;	}
	Bool						HasChildrenAnyNonStaticNodes()						{	return m_ChildrenWithNonStaticNodes.GetSize() > 0;	}
	TPtrArray<TQuadTreeZone>&	GetChildZonesWithNodes()							{	return m_ChildrenWithNodes;	}
	TPtrArray<TQuadTreeZone>&	GetChildZonesWithNonStaticNodes()					{	return m_ChildrenWithNonStaticNodes;	}
	TPtr<TQuadTreeZone>&		GetChildFromZonePosition(TLQuadTree::TZonePos Positon)		{	return (m_Children.GetSize() > 0) ? m_Children[Positon] : TLPtr::GetNullPtr<TQuadTreeZone>();	}

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
	TPtr<TQuadTreeZone>&		GetSiblingFromZonePosition(TLQuadTree::TZonePos Position);			//	find the sibling with this sibling index (ie. where in the parent they are placed)

	TPtrArray<TQuadTreeZone>&	GetNeighbourZones()									{	return m_NeighbourZones;	}
	void						FindNeighboursAll(TPtr<TLMaths::TQuadTreeZone>& pThis);	//	recursive call to FindNeighbours to sort out all the neighbours in the tree
	void						FindNeighbours(TPtr<TLMaths::TQuadTreeZone>& pThis);	//	calculate neighbours of ourselves

	Bool						DivideAll(TPtr<TLMaths::TQuadTreeZone>& pThis);		//	recursively divide until we're at our minimum size leafs

	FORCEINLINE SyncBool		IsActive() const									{	return m_Active;	}
	void						SetActive(SyncBool Active,Bool SetChildren);		//	sets this zone and all it's child zones as active

protected:
	void						GetInChildZones(TQuadTreeNode* pNode,TFixedArray<u32,4>& InZones);			//	return which child zone we're in, -1 if none
	void						OnChildZoneNodesChanged();							//	list of nodes in a child has changed

	void						DoAddNode(TPtr<TQuadTreeNode>& pNode);				//	add node to this zone's list
	void						DoRemoveNode(TPtr<TQuadTreeNode>& pNode);			//	remove node from this zone's list

	void						OnZoneStructureChanged();							//	if we subdivide, or delete child zones etc, then call this func - will go up to the root then notify subscribers of the change
	void						OnZoneActiveChanged(SyncBool Active);					//	zone has gone to sleep/woken up

	TPtr<TQuadTreeZone>&		FindNeighbourZone_West();
	TPtr<TQuadTreeZone>&		FindNeighbourZone_North();
	TPtr<TQuadTreeZone>&		FindNeighbourZone_East();
	TPtr<TQuadTreeZone>&		FindNeighbourZone_South();
	TPtr<TQuadTreeZone>&		FindNeighbourZone_SouthWest();
	TPtr<TQuadTreeZone>&		FindNeighbourZone_NorthWest();
	TPtr<TQuadTreeZone>&		FindNeighbourZone_NorthEast();
	TPtr<TQuadTreeZone>&		FindNeighbourZone_SouthEast();

private:
	Bool						Divide(TPtr<TLMaths::TQuadTreeZone>& pThis);		//	subdivide ourselves into 4 children

	TPtr<TQuadTreeZone>&		GetParentsNeighbourChild(TPtr<TQuadTreeZone>* pParentNeighbour,TLQuadTree::TZonePos ParentNeighbourChildPositon);

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

	TPtrArray<TQuadTreeZone>	m_NeighbourZones;		//	zones that surround us at the same depth, but may have different parents. Max of 8. In future these would be "zones that might be visible through portals"

	TQuadTreeParams				m_ZoneParams;			//	zone parameters - copied from parent
	
	SyncBool					m_Active;				//	currently just used for the scene graph as this type doesnt get overloaded. when wait is used it means we're active, but not THE active zone (a neighbour)
};


