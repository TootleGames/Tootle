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
	class TPath;
	class TPathNode;
};


//--------------------------------------------------
//	node on a path/path network
//--------------------------------------------------
class TLPath::TPathNode
{
	friend class TLAsset::TPathNetwork;
public:
	TPathNode(TRefRef NodeRef=TRef(),const float2& NodePos=float2());

	TRefRef					GetNodeRef() const							{	return m_NodeRef;	}
	FORCEINLINE Bool		IsJunction() const							{	return (m_Links.GetSize() >= 3);	}
	const float2&			GetPosition() const							{	return m_Position;	}

	Bool					AddLink(TRefRef NodeRef)					{	return m_Links.AddUnique( NodeRef ) != -1;	}
	Bool					AddLink(TPathNode& Node)					{	return m_Links.AddUnique( Node.GetNodeRef() ) != -1;	}
	Bool					RemoveLink(TRefRef NodeRef)					{	return m_Links.Remove( NodeRef );	}
	Bool					RemoveLink(TPathNode& Node)					{	return m_Links.Remove( Node.GetNodeRef() );	}

	FORCEINLINE Bool		operator==(TRefRef NodeRef) const			{	return GetNodeRef() == NodeRef;	}
	FORCEINLINE Bool		operator==(const TPathNode& Node) const		{	return GetNodeRef() == Node.GetNodeRef();	}

protected:
	Bool					ImportData(TBinaryTree& Data);
	Bool					ExportData(TBinaryTree& Data);

protected:
	TRef					m_NodeRef;			//	node ref
	float2					m_Position;			//	node position
	TArray<TRef>			m_Links;			//	links to other nodes
};




//--------------------------------------------------
//	network of nodes for a path
//--------------------------------------------------
class TLAsset::TPathNetwork : public TLAsset::TAsset
{
public:
	TPathNetwork(TRefRef AssetRef);

	TLPath::TPathNode*			GetNode(TRefRef NodeRef)									{	return m_Nodes.Find( NodeRef );	}
	TArray<TLPath::TPathNode>&	GetNodeArray()												{	return m_Nodes;	}
	TRef						GetRandomNode()												{	return m_Nodes.ElementRand().GetNodeRef();	}
	TRef						GetFreeNodeRef(TRef FromRef=TRef()) const;					//	return a ref for a node that isn't currently used

	TLPath::TPathNode*			AddNode(TRef NodeRef,const float2& NodePos);				//	create a new road node

	void						LinkNodes(TLPath::TPathNode& NodeA,TLPath::TPathNode& NodeB);		
	void						UnlinkNodes(TLPath::TPathNode& NodeA,TLPath::TPathNode& NodeB);	

	TRef						DivideLink(TLPath::TPathNode& NodeA,TLPath::TPathNode& NodeB,float2* pDividePos=NULL);	//	split a line, adds a new node in between these two

protected:
	virtual SyncBool			ImportData(TBinaryTree& Data);
	virtual SyncBool			ExportData(TBinaryTree& Data);

	virtual void				OnNodeAdded(TLPath::TPathNode& NewNode);
	virtual void				OnNodesLinked(TLPath::TPathNode& NodeA,TLPath::TPathNode& NodeB)	{	}
	virtual void				OnNodesUnlinked(TLPath::TPathNode& NodeA,TLPath::TPathNode& NodeB)	{	}
	virtual void				OnNodeLinkDivided(TLPath::TPathNode& NodeA,TLPath::TPathNode& NewNode,TLPath::TPathNode& NodeB)	{	}

protected:
	TArray<TLPath::TPathNode>	m_Nodes;				//	
	TLMaths::TBox2D				m_BoundsBox;			//	bounding box vertex extents
	TLMaths::TSphere2D			m_BoundsSphere;			//	bounding sphere
};




//--------------------------------------------------
//	path in a network - could be an asset? no need at the moment
//--------------------------------------------------
class TLPath::TPath
{
public:
	TPath(TRefRef PathNetwork);

protected:
	TRef					m_PathNetwork;			//	ref of path network asset
	TArray<TRef>			m_PathNodes;			//	list of nodes along the network to make up the path
};

