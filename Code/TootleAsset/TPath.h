/*------------------------------------------------------

	Path asset - currently just a PathNetwork type

-------------------------------------------------------*/
#pragma once

#include "TLAsset.h"
#include "TAsset.h"
#include <TootleMaths/TBox.h>
#include <TootleMaths/TSphere.h>
#include <TootleMaths/TLine.h>


namespace TLAsset
{
	class TPathNetwork;
};

namespace TLPath
{
	class TPathNode;			//	node in a path
	class TPathNodeLink;		//	node -> node link (contained in first node)
	class TPathLink;			//	node <-> node link (contained in path)

	namespace TDirection
	{
		enum Type
		{
			Any			= 0,
			Forward		= 1,
			Backward	= 2
		};
	}
};




//--------------------------------------------------
//	link to another node - always contained within a node
//--------------------------------------------------
class TLPath::TPathNodeLink
{
public:
	TPathNodeLink();
	TPathNodeLink(TRefRef LinkNodeRef,TDirection::Type Direction);

	FORCEINLINE TRefRef				GetLinkNodeRef() const		{	return m_LinkNodeRef;	}
	FORCEINLINE TDirection::Type 	GetDirection() const		{	return (TDirection::Type)m_Direction;	}

	FORCEINLINE Bool				operator==(TRefRef NodeRef) const			{	return GetLinkNodeRef() == NodeRef;	}
	FORCEINLINE Bool				operator==(const TPathNodeLink& Link) const		{	return GetLinkNodeRef() == Link.GetLinkNodeRef();	}

protected:
	TRef					m_LinkNodeRef;		//	
	u8						m_Direction;		//	stored as a u8 so we can just Write/Read this class as a Data type
};

TLCore_DeclareIsDataType( TLPath::TPathNodeLink );


//--------------------------------------------------
//	node/node link contained within a path
//	currently doesnt include much link info, but could expand to cache length, direction etc
//--------------------------------------------------
class TLPath::TPathLink
{
public:
	TPathLink();
	TPathLink(TRefRef NodeA,TRefRef NodeB);
	TPathLink(const TPathNode& NodeA,const TPathNode& NodeB);

	FORCEINLINE Bool				IsValid() const				{	return (m_Nodes.GetSize()==2) && GetNodeA().IsValid() && GetNodeB().IsValid();	}
	FORCEINLINE Bool				IsCacheValid() const		{	return m_CacheValid;	}

	FORCEINLINE TRefRef				GetNodeA() const			{	return m_Nodes[0];	}
	FORCEINLINE TRefRef				GetNodeB() const			{	return m_Nodes[1];	}
	FORCEINLINE const TArray<TRef>&	GetNodes() const			{	return m_Nodes;	}

	FORCEINLINE Bool				HasNode(TRefRef NodeRef) const	{	return m_Nodes.Exists( NodeRef );	}

	const TLMaths::TLine2D&			GetLinkLine() const			{	TLDebug_Assert( IsCacheValid(), "Accessing invalid pathlink data");	return m_LinkLine;	}
	const TLMaths::TSphere2D&		GetLinkBoundsSphere() const	{	TLDebug_Assert( IsCacheValid(), "Accessing invalid pathlink data");	return m_LinkBoundsSphere;	}
	float							GetLinkDistance() const		{	TLDebug_Assert( IsCacheValid(), "Accessing invalid pathlink data");	return m_LinkBoundsSphere.GetRadius() * 2.f;	}
	const float2&					GetLinkMidpoint() const		{	TLDebug_Assert( IsCacheValid(), "Accessing invalid pathlink data");	return m_LinkBoundsSphere.GetPos();	}

	void							OnNodePosChanged(const TPathNode& NodeA,const TPathNode& NodeB);	//	if either node position changes recalc cache info (line/sphere/etc)

	void							GetTransformOnLink(TLMaths::TTransform& Transform,float AlongLine);
	void							GetTransformOnLink(TLMaths::TTransform& Transform,float AlongLine,const float2& Translation);

	FORCEINLINE Bool				operator<(const TPathLink& Link) const;		//	for sorting
	FORCEINLINE Bool				operator==(const TPathLink& Link) const;	//	link comparisons must have A/B matching, but can be B/A

protected:
	TFixedArray<TRef,2>				m_Nodes;
	Bool							m_CacheValid;			//	is the cached line/sphere data valid?
	TLMaths::TLine2D				m_LinkLine;				//	line that links the two nodes
	TLMaths::TSphere2D				m_LinkBoundsSphere;		//	sphere bounds of the two links... this also serves as the distance (twice the radius) and the center point between the two lines (if thats useful..)
};

TLCore_DeclareIsDataType( TLPath::TPathLink );


//--------------------------------------------------
//	node on a path/path network
//--------------------------------------------------
class TLPath::TPathNode : public TBinaryTree
{
	friend class TLAsset::TPathNetwork;
public:
	TPathNode(TRefRef NodeRef=TRef(),const float2& NodePos=float2());

	TRefRef						GetNodeRef() const							{	return TBinaryTree::GetDataRef();	}
	FORCEINLINE Bool			IsJunction() const							{	return (m_Links.GetSize() >= 3);	}
	const float2&				GetPosition() const							{	return m_Position;	}
	FORCEINLINE void			SetPosition(TLAsset::TPathNetwork& PathNetwork,const float2& NewPos);

	TBinaryTree&				GetData()									{	return *this;	}
	const TBinaryTree&			GetData() const								{	return *this;	}

	TRefRef						GetLinkRef(u32 Index) const					{	return m_Links[Index].GetLinkNodeRef();	}
	const TPathNodeLink&		GetLinkAt(u32 Index) const					{	return m_Links[Index];	}
	const TPathNodeLink*		GetLink(TRefRef LinkNodeRef) const			{	return m_Links.FindConst( LinkNodeRef );	}
	TArray<TPathNodeLink>&		GetLinks()									{	return m_Links;	}
	const TArray<TPathNodeLink>&	GetLinks() const							{	return m_Links;	}

	FORCEINLINE Bool			operator==(TRefRef NodeRef) const			{	return GetNodeRef() == NodeRef;	}
	FORCEINLINE Bool			operator==(const TPathNode& Node) const		{	return GetNodeRef() == Node.GetNodeRef();	}

protected:
	Bool						ImportData(TBinaryTree& Data);
	Bool						ExportData(TBinaryTree& Data);
	
	FORCEINLINE void			SetPosition(const float2& NewPos)			{	m_Position = NewPos;	}	//	

	//	only accessible through the owner as it links both ways
	FORCEINLINE Bool			AddLink(TRefRef NodeRef,TDirection::Type Direction)	{	return m_Links.AddUnique( TPathNodeLink( NodeRef, Direction ) ) != -1;	}
	FORCEINLINE Bool			AddLink(TPathNode& Node,TDirection::Type Direction)	{	return m_Links.AddUnique( TPathNodeLink( Node.GetNodeRef(), Direction ) ) != -1;	}
	FORCEINLINE Bool			RemoveLink(TRefRef NodeRef)							{	return m_Links.Remove( NodeRef );	}
	FORCEINLINE Bool			RemoveLink(TPathNode& Node)							{	return m_Links.Remove( Node.GetNodeRef() );	}

private:
	FORCEINLINE void			SetNodeRef(TRefRef NodeRef)					{	return TBinaryTree::SetDataRef( NodeRef );	}

protected:
	float2						m_Position;			//	node position
	THeapArray<TPathNodeLink>	m_Links;			//	links to other nodes
};




//--------------------------------------------------
//	network of nodes for a path
//--------------------------------------------------
class TLAsset::TPathNetwork : public TLAsset::TAsset
{
public:
	TPathNetwork(TRefRef AssetRef);

	static TRef						GetAssetType_Static()										{	return TRef_Static(P,a,t,h,N);	}

	TPtr<TLPath::TPathNode>&		GetNode(TRefRef NodeRef)									{	return m_Nodes.FindPtr( NodeRef );	}
	TPtr<TLPath::TPathNode>&		GetRandomNode()												{	return m_Nodes.ElementRandom();	}

	TPtr<TLPath::TPathNode>&		GetNearestNode(const float2& Position);						//	find the nearest node to this position
	TPtr<TLPath::TPathNode>&		GetNearestNode(const float3& Position)						{	return GetNearestNode( Position.xy() );	}
	TPtrArray<TLPath::TPathNode>&	GetNodeArray()												{	return m_Nodes;	}
	const TPtrArray<TLPath::TPathNode>&	GetNodeArray() const									{	return m_Nodes;	}
	TRef							GetFreeNodeRef(TRef FromRef=TRef()) const;					//	return a ref for a node that isn't currently used
	TRef							GetFreeNodeRef(TRef FromRef=TRef());						//	non-const version enables sorting
	const TArray<TLPath::TPathLink>&	GetLinks() const										{	return m_Links;	}

	TPtr<TLPath::TPathNode>&		AddNode(TRef NodeRef,const float2& NodePos);				//	create a new node, returns NULL if it fails, e.g. if NodeRef already exists
	void							RemoveNode(TRef NodeRef);									//	remove node and clear up links

	void							LinkNodes(TLPath::TPathNode& NodeA,TLPath::TPathNode& NodeB,Bool OneWayDirection);	//	if OneWayDirection is specified, the one-way direction goes from A to B
	void							LinkNodes(TRefRef NodeARef,TRefRef NodeBRef,Bool OneWayDirection);					//	if OneWayDirection is specified, the one-way direction goes from A to B
	Bool							UnlinkNodes(TLPath::TPathNode& NodeA,TLPath::TPathNode& NodeB);		//	returns FALSE if they weren't linked

	TPtr<TLPath::TPathNode>&		DivideLink(TLPath::TPathNode& NodeA,TLPath::TPathNode& NodeB,float2* pDividePos=NULL);	//	split a line, adds a new node in between these two

	Bool							SetNodePosition(TRefRef NodeRef,const float2& NewPos);			//	change position of a node, returns TRUE if changed and invokes a changed message
	Bool							SetNodePosition(TLPath::TPathNode& Node,const float2& NewPos);	//	change position of a node, returns TRUE if changed and invokes a changed message

protected:
	virtual SyncBool				ImportData(TBinaryTree& Data);
	virtual SyncBool				ExportData(TBinaryTree& Data);

	virtual void					OnNodeAdded(TPtr<TLPath::TPathNode>& pNewNode);
	virtual void					OnNodesLinked(TLPath::TPathNode& NodeA,TLPath::TPathNode& NodeB)	{	}
	virtual void					OnNodesUnlinked(TLPath::TPathNode& NodeA,TLPath::TPathNode& NodeB)	{	}
	virtual void					OnNodeLinkDivided(TLPath::TPathNode& NodeA,TLPath::TPathNode& NewNode,TLPath::TPathNode& NodeB)	{	}
	virtual void					OnNodePosChanged(TLPath::TPathNode& Node);

protected:
	TPtrArray<TLPath::TPathNode>	m_Nodes;				//	todo: sort these
	THeapArray<TLPath::TPathLink>	m_Links;				//	all node/node links. todo; sort
	TLMaths::TBox2D					m_BoundsBox;			//	bounding box vertex extents
	TLMaths::TSphere2D				m_BoundsSphere;			//	bounding sphere
};






//----------------------------------------------
//	
//----------------------------------------------
FORCEINLINE void TLPath::TPathNode::SetPosition(TLAsset::TPathNetwork& PathNetwork,const float2& NewPos)
{	
	PathNetwork.SetNodePosition( *this, NewPos );
}

//----------------------------------------------
//	for sorting
//----------------------------------------------
FORCEINLINE Bool TLPath::TPathLink::operator<(const TPathLink& Link) const
{
	//	first sort by nodeA
	if ( GetNodeA() < Link.GetNodeA() )		return TRUE;
	if ( GetNodeA() > Link.GetNodeA() )		return FALSE;

	//	same NodeA, sort by NodeB...
	if ( GetNodeB() < Link.GetNodeB() )		return TRUE;
	if ( GetNodeB() > Link.GetNodeB() )		return FALSE;

	//	not less than
	return FALSE;
}

//----------------------------------------------
//	link comparisons must have A/B matching, but can be B/A
//----------------------------------------------
FORCEINLINE Bool TLPath::TPathLink::operator==(const TPathLink& Link) const
{
	//	match
	if ( GetNodeA() == Link.GetNodeA() && GetNodeB() == Link.GetNodeB() )
		return TRUE;

	//	match in reverse order
	if ( GetNodeA() == Link.GetNodeB() && GetNodeB() == Link.GetNodeA() )
		return TRUE;

	//	differ
	return FALSE;
}

