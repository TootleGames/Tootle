/*------------------------------------------------------
	
	Base graph class to do generic nodey/graph stuff without 
	needing to know the graph instance/type

	all returns and operations use TRef's so we don't need to
	know the type

-------------------------------------------------------*/
#pragma once

#include "TRef.h"
#include "TLMessaging.h"
#include "TRelay.h"
#include "TManager.h"

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
class TLGraph::TGraphBase : public TLCore::TManager
{
public:
	TGraphBase(TRefRef GraphRef) :
		TLCore::TManager	( GraphRef )
	{
	}
	virtual ~TGraphBase()		{}
	
	FORCEINLINE TRefRef			GetGraphRef() const						{	return TLCore::TManager::GetManagerRef();	}

	virtual Bool				SendMessageTo(TRefRef RecipientRef, TLMessaging::TMessage& Message) 		{	return SendMessageToNode(RecipientRef, Message);	}	// Generic manager SendMessage routine wrapper for the graph SendMessageToNode
	virtual Bool				SendMessageToNode(TRefRef NodeRef,TLMessaging::TMessage& Message)=0;	//	send message to node

	virtual TRef				CreateNode(TRefRef NodeRef,TRefRef TypeRef,TRefRef ParentRef,TLMessaging::TMessage* pInitMessage=NULL,Bool StrictNodeRef=FALSE)=0;	//	create node and add to the graph. returns ref of new node
	virtual Bool				IsInGraph(TRefRef NodeRef,Bool CheckRequestQueue=TRUE)=0;			//	return if a node exists
	void						RemoveNodes(const TArray<TRef>& NodeRefs);							//	remove an array of nodes by their ref
	virtual Bool				RemoveNode(TRefRef NodeRef)=0;										//	remove an array of nodes by their ref
	virtual Bool				RemoveChildren(TRefRef NodeRef)=0;									//	remove all children from this node
	virtual TRef				GetFreeNodeRef(TRefRef BaseRef=TRef())=0;							//	find an unused ref for a node - returns the ref

	FORCEINLINE Bool			ImportScheme(const TLAsset::TScheme* pScheme,TRefRef ParentNodeRef,Bool StrictNodeRefs=TRUE,TBinaryTree* pCommonInitData=NULL)		{	return pScheme ? ImportScheme( *pScheme, ParentNodeRef, StrictNodeRefs, pCommonInitData ) : FALSE;	}
	Bool						ImportScheme(const TLAsset::TScheme& Scheme,TRefRef ParentNodeRef,Bool StrictNodeRefs=TRUE,TBinaryTree* pCommonInitData=NULL);		//	import scheme into this graph
	FORCEINLINE Bool			ReimportScheme(const TLAsset::TScheme* pScheme,TRefRef ParentNodeRef,Bool StrictNodeRefs,Bool AddMissingNodes,Bool RemoveUnknownNodes,TBinaryTree* pCommonInitData=NULL)	{	return pScheme ? ReimportScheme( *pScheme, ParentNodeRef, StrictNodeRefs, AddMissingNodes, RemoveUnknownNodes, pCommonInitData ) : FALSE;	}
	Bool						ReimportScheme(const TLAsset::TScheme& Scheme,TRefRef ParentNodeRef,Bool StrictNodeRefs,Bool AddMissingNodes,Bool RemoveUnknownNodes,TBinaryTree* pCommonInitData=NULL);							//	re-import scheme into this graph. Nodes will be re-sent an Initialise message. Add missing and delete new (non-scheme) nodes via params. this system will kinda mess up if the original scheme wasn't loaded with strict refs
	Bool						ExportScheme(TLAsset::TScheme& Scheme,TRef SchemeRootNode,Bool IncludeSchemeRootNode);	//	export node tree to a scheme

	//	gr: exposed for the scheme editor...
	virtual TLGraph::TGraphNodeBase*	FindNodeBase(TRefRef NodeRef) = 0;
	virtual TLGraph::TGraphNodeBase*	GetRootNodeBase() = 0;

private:
	Bool						ImportSchemeNode(TLAsset::TSchemeNode& SchemeNode,TRefRef ParentRef,TArray<TRef>& ImportedNodes,Bool StrictNodeRefs,TBinaryTree* pCommonInitData);	//	import scheme node (tree) into this graph
	Bool						ReimportSchemeNode(TLAsset::TSchemeNode& SchemeNode,TRefRef ParentRef,Bool StrictNodeRefs,Bool AddMissingNodes,Bool RemoveUnknownNodes,TBinaryTree* pCommonInitData);		//	re-init and restore node tree
	TPtr<TLAsset::TSchemeNode>	ExportSchemeNode(TGraphNodeBase* pNode);
};



//--------------------------------------------------------------------
//	base graph node class
//--------------------------------------------------------------------
class TLGraph::TGraphNodeBase : public TLMessaging::TPublisherSubscriber
{
	friend class TLGraph::TGraphBase;
public:
	TGraphNodeBase(TRefRef NodeRef,TRefRef NodeTypeRef);
	virtual ~TGraphNodeBase()											{	}

	FORCEINLINE TRefRef			GetNodeRef() const						{	return m_NodeRef; }
	FORCEINLINE TRefRef			GetNodeTypeRef() const					{	return m_NodeTypeRef; }

	//	gr: just realised... this is wrong. why would we check our parent's type?
	Bool						IsKindOf(TRefRef TypeRef) const			{	return (GetNodeTypeRef() == TypeRef) ? TRUE : IsParentKindOf( TypeRef );	}
	FORCEINLINE Bool			IsParentKindOf(TRefRef TypeRef) const	{	return GetParentBase() ? GetParentBase()->IsKindOf(TypeRef) : FALSE;	}
	
	virtual void				UpdateNodeData()						{	}						//	overload this to make sure all properties on your node (that aren't stored/read directly out of the data) is up to date and reayd for export
	virtual TBinaryTree&		GetNodeData()							{	return m_NodeData;	}	//	overload this to handle UpdateData for specific nodes

	//	gr: big hack here! instead... use node data? it's more class based than instance based... cant think of a great solution right now...
	//	Will remove this at somepoint for some other interface, ie. when looking at audio graph, something will be rendering debug-style
	//	nodes, so we'd use them
	virtual TRef				GetRenderNodeRef() const				{	return TRef();	}	//	get a render node ref to represent this node. used in the scheme editor as the widget node. 
	
	//	gr: exposed for the scheme editor...
	virtual void					GetChildrenBase(TArray<TGraphNodeBase*>& ChildNodes) = 0;
	void							GetChildren(TArray<TRef>& ChildNodeRefs,Bool Recursive=FALSE);		//	get array of children's refs. if recursive will go through the whole tree
	FORCEINLINE void				GetChildrenTree(TArray<TRef>& ChildNodeRefs)						{	GetChildren( ChildNodeRefs, TRUE );	}
	virtual const TGraphNodeBase*	GetParentBase() const = 0;
	
protected:
	virtual void					Initialise(TLMessaging::TMessage& Message);				//	[soon to be] only called once for initialisation after being added to the graph. Does an initialising call to SetProperty to replace the legacy usage
	virtual void					SetProperty(TLMessaging::TMessage& Message);			//	base SetProperty function writes/updates the NodeData with any data in the message that hasn't already been read
	virtual void					GetProperty(TLMessaging::TMessage& Message, TLMessaging::TMessage& Response);	//	GetProperty message handler
	virtual void					ProcessMessage(TLMessaging::TMessage& Message)			{	}	//	gr: only implemented as its pure virtual. Maybe in the future move the common Initialise, shutdown, SetProperty into this class...

	FORCEINLINE void				SetNodeRef(TRefRef NodeRef)			{	m_NodeRef = NodeRef;	m_NodeRef.GetString( m_Debug_NodeRefString );	}

protected:
	TRef					m_NodeRef;			//	Node unique ID

private:
	TRef					m_NodeTypeRef;		//	node's type
	TBinaryTree				m_NodeData;			//	node data

	TBufferString<6>		m_Debug_NodeRefString;
	TBufferString<6>		m_Debug_NodeTypeRefString;
};

