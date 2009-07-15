/*------------------------------------------------------

	Scheme asset

-------------------------------------------------------*/
#pragma once

#include "TLAsset.h"
#include "TAsset.h"
#include <TootleCore/TBinaryTree.h>


namespace TLAsset
{
	class TScheme;
	class TSchemeNode;
};



//---------------------------------------------------------
//	a parsed node to be inserted into a graph - tree format 
//	(as it was in the scheme)
//---------------------------------------------------------
class TLAsset::TSchemeNode : public TBinaryTree
{
	friend class TLAsset::TScheme;
public:
	TSchemeNode(TRefRef NodeRef=TRef(),TRefRef GraphRef=TRef(),TRefRef TypeRef=TRef());

	TPtrArray<TSchemeNode>&		GetChildren()						{	return m_Children;	}
	const TPtrArray<TSchemeNode>&	GetChildren() const				{	return m_Children;	}
	void						AddChild(TPtr<TSchemeNode>& pNode)	{	m_Children.Add( pNode );	}
	TBinaryTree&				GetData()							{	return *this;	}
	const TBinaryTree&			GetData() const						{	return *this;	}

	TRefRef						GetNodeRef() const					{	return TBinaryTree::GetDataRef();	}
	TRefRef						GetTypeRef() const					{	return m_TypeRef;	}
	TRefRef						GetGraphRef() const					{	return m_GraphRef;	}

protected:
	virtual SyncBool			ImportData(TBinaryTree& Data);		//	load asset data out binary data
	virtual SyncBool			ExportData(TBinaryTree& Data);		//	save asset data to binary data
	FORCEINLINE void			SetNodeRef(TRefRef NodeRef)			{	TBinaryTree::SetDataRef( NodeRef );	}

protected:
	TPtrArray<TSchemeNode>		m_Children;		// child nodes
	TRef						m_GraphRef;
	TRef						m_TypeRef;
};



//---------------------------------------------------------
//	scheme is a collection of node(tree)'s to be inserted in various
//	graphs
//---------------------------------------------------------
class TLAsset::TScheme : public TLAsset::TAsset
{
public:
	TScheme(TRefRef AssetRef);

	const TPtrArray<TSchemeNode>&	GetNodes() const					{	return m_Nodes;	}
	TPtrArray<TSchemeNode>&			GetNodes()							{	return m_Nodes;	}
	void							RemoveNodes()						{	m_Nodes.Empty();	}
	void							AddNode(TPtr<TSchemeNode>& pNode)	{	m_Nodes.Add( pNode );	}

protected:
	virtual SyncBool			ImportData(TBinaryTree& Data);	//	load asset data out binary data
	virtual SyncBool			ExportData(TBinaryTree& Data);	//	save asset data to binary data

protected:
	TPtrArray<TSchemeNode>		m_Nodes;		//	node's we've imported
};




