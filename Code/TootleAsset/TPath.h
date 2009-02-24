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
};


//--------------------------------------------------
//	node on a path/path network
//--------------------------------------------------
class TLPath::TPathNode : public TBinaryTree
{
	friend class TLAsset::TPathNetwork;
public:
	TPathNode(TRefRef NodeRef=TRef(),const float2& NodePos=float2());

	TRefRef					GetNodeRef() const							{	return TBinaryTree::GetDataRef();	}
	FORCEINLINE Bool		IsJunction() const							{	return (m_Links.GetSize() >= 3);	}
	const float2&			GetPosition() const							{	return m_Position;	}

	Bool					AddLink(TRefRef NodeRef)					{	return m_Links.AddUnique( NodeRef ) != -1;	}
	Bool					AddLink(TPathNode& Node)					{	return m_Links.AddUnique( Node.GetNodeRef() ) != -1;	}
	Bool					RemoveLink(TRefRef NodeRef)					{	return m_Links.Remove( NodeRef );	}
	Bool					RemoveLink(TPathNode& Node)					{	return m_Links.Remove( Node.GetNodeRef() );	}

	TBinaryTree&			GetData()									{	return *this;	}
	const TBinaryTree&		GetData() const								{	return *this;	}

	TArray<TRef>&			GetLinks()									{	return m_Links;	}
	const TArray<TRef>&		GetLinks() const							{	return m_Links;	}

	FORCEINLINE Bool		operator==(TRefRef NodeRef) const			{	return GetNodeRef() == NodeRef;	}
	FORCEINLINE Bool		operator==(const TPathNode& Node) const		{	return GetNodeRef() == Node.GetNodeRef();	}

protected:
	Bool					ImportData(TBinaryTree& Data);
	Bool					ExportData(TBinaryTree& Data);

private:
	void					SetNodeRef(TRefRef NodeRef)					{	return TBinaryTree::SetDataRef( NodeRef );	}

protected:
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

	TPtr<TLPath::TPathNode>&		GetNode(TRefRef NodeRef)									{	return m_Nodes.FindPtr( NodeRef );	}
	TPtr<TLPath::TPathNode>&		GetRandomNode()												{	return m_Nodes.ElementRand();	}
	TPtr<TLPath::TPathNode>&		GetNearestNode(const float2& Position);						//	find the nearest node to this position
	TPtr<TLPath::TPathNode>&		GetNearestNode(const float3& Position)						{	return GetNearestNode( Position.xy() );	}
	TPtrArray<TLPath::TPathNode>&	GetNodeArray()												{	return m_Nodes;	}
	TRef							GetFreeNodeRef(TRef FromRef=TRef()) const;					//	return a ref for a node that isn't currently used

	TPtr<TLPath::TPathNode>&		AddNode(TRef NodeRef,const float2& NodePos);				//	create a new node, returns NULL if it fails, e.g. if NodeRef already exists
	void							RemoveNode(TRef NodeRef);									//	remove node and clear up links

	void							LinkNodes(TLPath::TPathNode& NodeA,TLPath::TPathNode& NodeB);		
	Bool							UnlinkNodes(TLPath::TPathNode& NodeA,TLPath::TPathNode& NodeB);		//	returns FALSE if they weren't linked

	TPtr<TLPath::TPathNode>&		DivideLink(TLPath::TPathNode& NodeA,TLPath::TPathNode& NodeB,float2* pDividePos=NULL);	//	split a line, adds a new node in between these two

protected:
	virtual SyncBool				ImportData(TBinaryTree& Data);
	virtual SyncBool				ExportData(TBinaryTree& Data);

	virtual void					OnNodeAdded(TPtr<TLPath::TPathNode>& pNewNode);
	virtual void					OnNodesLinked(TLPath::TPathNode& NodeA,TLPath::TPathNode& NodeB)	{	}
	virtual void					OnNodesUnlinked(TLPath::TPathNode& NodeA,TLPath::TPathNode& NodeB)	{	}
	virtual void					OnNodeLinkDivided(TLPath::TPathNode& NodeA,TLPath::TPathNode& NewNode,TLPath::TPathNode& NodeB)	{	}

protected:
	TPtrArray<TLPath::TPathNode>	m_Nodes;				//	todo: sort these
	TLMaths::TBox2D					m_BoundsBox;			//	bounding box vertex extents
	TLMaths::TSphere2D				m_BoundsSphere;			//	bounding sphere
};


