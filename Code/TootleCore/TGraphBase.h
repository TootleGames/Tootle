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

	virtual Bool				SendMessageToNode(TRefRef NodeRef,TPtr<TLMessaging::TMessage>& pMessage)=0;	//	send message to node

	virtual TRef				CreateNode(TRefRef NodeRef,TRefRef TypeRef,TRefRef ParentRef,TPtr<TLMessaging::TMessage> pInitMessage=NULL)=0;	//	create node and add to the graph. returns ref of new node
	void						RemoveNodes(const TArray<TRef>& NodeRefs);							//	remove an array of nodes by their ref
	virtual Bool				RemoveNode(TRefRef NodeRef)=0;										//	remove an array of nodes by their ref

	Bool						ImportScheme(const TPtr<TLAsset::TScheme>& pScheme,TRefRef ParentNodeRef=TRef());		//	import scheme into this graph
	TPtr<TLAsset::TScheme>		ExportScheme(TRef SchemeAssetRef,TRef SchemeRootNode=TRef(),Bool IncludeSchemeRootNode=TRUE);	//	export node tree to a scheme

protected:
	virtual TLGraph::TGraphNodeBase*	FindNodeBase(TRefRef NodeRef) = 0;
	virtual TLGraph::TGraphNodeBase*	GetRootNodeBase() = 0;

private:
	Bool						ImportSchemeNode(const TPtr<TLAsset::TSchemeNode>& pSchemeNode,TRefRef ParentRef,TArray<TRef>& ImportedNodes);				//	import scheme node (tree) into this graph
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

	Bool						IsKindOf(TRefRef TypeRef) const			{	return (GetNodeTypeRef() == TypeRef) ? TRUE : IsParentKindOf( TypeRef );	}
	FORCEINLINE Bool			IsParentKindOf(TRefRef TypeRef) const	{	return GetParentBase() ? GetParentBase()->IsKindOf(TypeRef) : FALSE;	}
	
	virtual const TBinaryTree&	GetNodeData(Bool UpdateData)			{	return m_NodeData;	}	//	overload this to handle UpdateData for specific nodes

protected:
	FORCEINLINE void				SetNodeRef(TRefRef NodeRef)			{	m_NodeRef = NodeRef;	m_NodeRef.GetString( m_Debug_NodeRefString );	}

	virtual void					GetChildrenBase(TArray<TGraphNodeBase*>& ChildNodes) = 0;
	virtual const TGraphNodeBase*	GetParentBase() const = 0;
	
	virtual TBinaryTree&			GetNodeData()						{	return m_NodeData;	}	//	overload this to handle UpdateData for specific nodes

protected:
	TRef					m_NodeRef;			//	Node unique ID

private:
	TRef					m_NodeTypeRef;		//	node's type
	TBinaryTree				m_NodeData;			//	node data

	TBufferString<6>		m_Debug_NodeRefString;
	TBufferString<6>		m_Debug_NodeTypeRefString;
};

