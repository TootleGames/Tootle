/*------------------------------------------------------

	Path asset - currently just a PathNetwork type

-------------------------------------------------------*/
#pragma once

#include "TLAsset.h"
#include "TAsset.h"
#include <TootleMaths/TBox.h>
#include <TootleMaths/TSphere.h>


namespace TLAsset
{
	class TPathNetwork;
};

namespace TLPath
{
	class TPathNode;
	class TPathLink;

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
class TLPath::TPathLink
{
public:
	TPathLink();
	TPathLink(TRefRef LinkNodeRef,TDirection::Type Direction);

	FORCEINLINE TRefRef				GetLinkNodeRef() const		{	return m_LinkNodeRef;	}
	FORCEINLINE TDirection::Type 	GetDirection() const		{	return (TDirection::Type)m_Direction;	}

	FORCEINLINE Bool				operator==(TRefRef NodeRef) const			{	return GetLinkNodeRef() == NodeRef;	}
	FORCEINLINE Bool				operator==(const TPathLink& Link) const		{	return GetLinkNodeRef() == Link.GetLinkNodeRef();	}

protected:
	TRef					m_LinkNodeRef;		//	
	u8						m_Direction;		//	stored as a u8 so we can just Write/Read this class as a Data type
};


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
	const TPathLink&			GetLinkAt(u32 Index) const					{	return m_Links[Index];	}
	const TPathLink*			GetLink(TRefRef LinkNodeRef) const			{	return m_Links.FindConst( LinkNodeRef );	}
	TArray<TPathLink>&			GetLinks()									{	return m_Links;	}
	const TArray<TPathLink>&	GetLinks() const							{	return m_Links;	}

	FORCEINLINE Bool			operator==(TRefRef NodeRef) const			{	return GetNodeRef() == NodeRef;	}
	FORCEINLINE Bool			operator==(const TPathNode& Node) const		{	return GetNodeRef() == Node.GetNodeRef();	}

protected:
	Bool						ImportData(TBinaryTree& Data);
	Bool						ExportData(TBinaryTree& Data);
	
	FORCEINLINE void			SetPosition(const float2& NewPos)			{	m_Position = NewPos;	}	//	

	//	only accessible through the owner as it links both ways
	FORCEINLINE Bool			AddLink(TRefRef NodeRef,TDirection::Type Direction)	{	return m_Links.AddUnique( TPathLink( NodeRef, Direction ) ) != -1;	}
	FORCEINLINE Bool			AddLink(TPathNode& Node,TDirection::Type Direction)	{	return m_Links.AddUnique( TPathLink( Node.GetNodeRef(), Direction ) ) != -1;	}
	FORCEINLINE Bool			RemoveLink(TRefRef NodeRef)							{	return m_Links.Remove( NodeRef );	}
	FORCEINLINE Bool			RemoveLink(TPathNode& Node)							{	return m_Links.Remove( Node.GetNodeRef() );	}

private:
	FORCEINLINE void			SetNodeRef(TRefRef NodeRef)					{	return TBinaryTree::SetDataRef( NodeRef );	}

protected:
	float2						m_Position;			//	node position
	TArray<TPathLink>			m_Links;			//	links to other nodes
};




//--------------------------------------------------
//	network of nodes for a path
//--------------------------------------------------
class TLAsset::TPathNetwork : public TLAsset::TAsset
{
public:
	TPathNetwork(TRefRef AssetRef);

	TPtr<TLPath::TPathNode>&		GetNode(TRefRef NodeRef)									{	return m_Nodes.FindPtr( NodeRef );	}
	TPtr<TLPath::TPathNode>&		GetRandomNode()												{	return m_Nodes.ElementRand();	}
	TPtr<TLPath::TPathNode>&		GetNearestNode(const float2& Position);						//	find the nearest node to this position
	TPtr<TLPath::TPathNode>&		GetNearestNode(const float3& Position)						{	return GetNearestNode( Position.xy() );	}
	TPtrArray<TLPath::TPathNode>&	GetNodeArray()												{	return m_Nodes;	}
	TRef							GetFreeNodeRef(TRef FromRef=TRef()) const;					//	return a ref for a node that isn't currently used
	TRef							GetFreeNodeRef(TRef FromRef=TRef());						//	non-const version enables sorting

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
	virtual void					OnNodePosChanged(TLPath::TPathNode& Node)							{	}

protected:
	TPtrArray<TLPath::TPathNode>	m_Nodes;				//	todo: sort these
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


