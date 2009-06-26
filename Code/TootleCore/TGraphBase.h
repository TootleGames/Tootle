/*------------------------------------------------------
	
	Base graph class to do generic nodey/graph stuff without 
	needing to know the graph instance/type

	all returns and operations use TRef's so we don't need to
	know the type

-------------------------------------------------------*/
#pragma once

#include "TRef.h"
#include "TLMessaging.h"

namespace TLAsset
{
	class TScheme;
	class TSchemeNode;
}

namespace TLGraph
{
	class TGraphBase;
	class TGraphNodeBase;
};


//--------------------------------------------------------------------
//	base graph class
//--------------------------------------------------------------------
class TLGraph::TGraphBase
{
public:
	TGraphBase()				{}
	
	virtual TRefRef				GetGraphRef() const = 0;

	virtual Bool				SendMessageToNode(TRefRef NodeRef,TLMessaging::TMessage& Message)=0;	//	send message to node

	virtual TRef				CreateNode(TRefRef NodeRef,TRefRef TypeRef,TRefRef ParentRef,TLMessaging::TMessage* pInitMessage=NULL,Bool StrictNodeRef=FALSE)=0;	//	create node and add to the graph. returns ref of new node
	virtual Bool				IsInGraph(TRefRef NodeRef,Bool CheckRequestQueue=TRUE)=0;			//	return if a node exists
	void						RemoveNodes(const TArray<TRef>& NodeRefs);							//	remove an array of nodes by their ref
	virtual Bool				RemoveNode(TRefRef NodeRef)=0;										//	remove an array of nodes by their ref

	FORCEINLINE Bool			ImportScheme(const TLAsset::TScheme* pScheme,TRefRef ParentNodeRef,Bool StrictNodeRefs=TRUE)		{	return pScheme ? ImportScheme( *pScheme, ParentNodeRef, StrictNodeRefs ) : FALSE;	}
	Bool						ImportScheme(const TLAsset::TScheme& Scheme,TRefRef ParentNodeRef,Bool StrictNodeRefs=TRUE);		//	import scheme into this graph
	FORCEINLINE Bool			ReimportScheme(const TLAsset::TScheme* pScheme,TRefRef ParentNodeRef,Bool StrictNodeRefs,Bool AddMissingNodes,Bool RemoveUnknownNodes)	{	return pScheme ? ReimportScheme( *pScheme, ParentNodeRef, StrictNodeRefs, AddMissingNodes, RemoveUnknownNodes ) : FALSE;	}
	Bool						ReimportScheme(const TLAsset::TScheme& Scheme,TRefRef ParentNodeRef,Bool StrictNodeRefs,Bool AddMissingNodes,Bool RemoveUnknownNodes);							//	re-import scheme into this graph. Nodes will be re-sent an Initialise message. Add missing and delete new (non-scheme) nodes via params. this system will kinda mess up if the original scheme wasn't loaded with strict refs
	TPtr<TLAsset::TScheme>		ExportScheme(TRef SchemeAssetRef,TRef SchemeRootNode=TRef(),Bool IncludeSchemeRootNode=TRUE);	//	export node tree to a scheme

protected:
	virtual TLGraph::TGraphNodeBase*	FindNodeBase(TRefRef NodeRef) = 0;
	virtual TLGraph::TGraphNodeBase*	GetRootNodeBase() = 0;

private:
	Bool						ImportSchemeNode(const TLAsset::TSchemeNode& SchemeNode,TRefRef ParentRef,TArray<TRef>& ImportedNodes,Bool StrictNodeRefs);	//	import scheme node (tree) into this graph
	Bool						ReimportSchemeNode(const TLAsset::TSchemeNode& SchemeNode,TRefRef ParentRef,Bool StrictNodeRefs,Bool AddMissingNodes,Bool RemoveUnknownNodes);		//	re-init and restore node tree
	TPtr<TLAsset::TSchemeNode>	ExportSchemeNode(TGraphNodeBase* pNode);
};



//--------------------------------------------------------------------
//	base graph node class
//--------------------------------------------------------------------
class TLGraph::TGraphNodeBase
{
	friend class TLGraph::TGraphBase;
public:
	TGraphNodeBase(TRefRef NodeRef,TRefRef NodeTypeRef);

	FORCEINLINE TRefRef			GetNodeRef() const						{	return m_NodeRef; }
	FORCEINLINE TRefRef			GetNodeTypeRef() const					{	return m_NodeTypeRef; }

	//	gr: just realised... this is wrong. why would we check our parent's type?
	Bool						IsKindOf(TRefRef TypeRef) const			{	return (GetNodeTypeRef() == TypeRef) ? TRUE : IsParentKindOf( TypeRef );	}
	FORCEINLINE Bool			IsParentKindOf(TRefRef TypeRef) const	{	return GetParentBase() ? GetParentBase()->IsKindOf(TypeRef) : FALSE;	}
	
	virtual const TBinaryTree&	GetNodeData(Bool UpdateData)			{	return m_NodeData;	}	//	overload this to handle UpdateData for specific nodes
	virtual TBinaryTree&		GetNodeData()							{	return m_NodeData;	}	//	overload this to handle UpdateData for specific nodes

protected:
	virtual void					Initialise(TLMessaging::TMessage& Message);	

	virtual void					GetChildrenBase(TArray<TGraphNodeBase*>& ChildNodes) = 0;
	virtual const TGraphNodeBase*	GetParentBase() const = 0;
	
	FORCEINLINE void				SetNodeRef(TRefRef NodeRef)			{	m_NodeRef = NodeRef;	m_NodeRef.GetString( m_Debug_NodeRefString );	}

protected:
	TRef					m_NodeRef;			//	Node unique ID

private:
	TRef					m_NodeTypeRef;		//	node's type
	TBinaryTree				m_NodeData;			//	node data

	TBufferString<6>		m_Debug_NodeRefString;
	TBufferString<6>		m_Debug_NodeTypeRefString;
};

