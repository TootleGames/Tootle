/*
 *  TGraphTree.h
 *  TootleReflection
 *
 *  Created by Graham Reeves on 17/01/2010.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */
#pragma once

#include <TootleGui/TWindow.h>
#include <TootleGui/TTree.h>
#include <TootleCore/TGraphBase.h>
#include <TootleCore/TLMessaging.h>


namespace TLReflection
{
	class TGraphTreeItem;	//	item for a TLGui::Tree which maps items to nodes and vice versa
	
	//	gr: rename this to TGraphTreeWindow ?
	class TGraphTree;		//	window which contains a tree displaying the contents of a graph
}


//---------------------------------------------------------
//	todo: subscribe to the node's messages and update item if any of it's
//	exposed (column'd) data changes
//---------------------------------------------------------
class TLReflection::TGraphTreeItem : public TLGui::TTreeItem, public TLMessaging::TSubscriber
{
public:
	TGraphTreeItem(TGraphTree& GraphTree);			//	root constructor
	TGraphTreeItem(TTreeItem& Parent,TRefRef NodeRef);	//	child constructor
	~TGraphTreeItem();

	virtual TRefRef				GetSubscriberRef() const			{	return m_NodeRef;	}
	TRefRef						GetNodeRef() const 					{	return m_NodeRef;	}
	
	//	gr: these are overloaded so we can update the node->item index
	virtual bool				AddChild(TPtr<TTreeItem>& pChildItem);		//	add child item and notify owner
	virtual bool				RemoveChild(TTreeItem& ChildItem);			//	remove child from self and notify owner. Not using a TPtr as we don't need it and we're expecting a definate item
	
protected:
	virtual void				ProcessMessage(TLMessaging::TMessage& Message);	//	catch changes to the node we're associated with

	virtual TBinary*			GetData(TRefRef DataRef);	//	get data to read/write from
	virtual void				CreateChildren();			//	create child items
	
	TLReflection::TGraphTree&	GetGraphTree()				{	return *m_pGraphTree;	}
	TLGraph::TGraphBase&		GetGraph();//					{	return GetGraphTree().GetGraph();	}
	TLGraph::TGraphNodeBase*	GetNode()					{	return GetGraph().FindNodeBase( GetNodeRef() );	}

private:
	void						SetGraphTree(TGraphTree& GraphTree);	//	associate with this graph
	
protected:
	TRef						m_NodeRef;		//	node ref
	TGraphTree*					m_pGraphTree;	//	pointer back to the graph tree
};


//--------------------------------------------------------------
//	window which contains a tree displaying the contents of a graph
//--------------------------------------------------------------
class TLReflection::TGraphTree : public TLMessaging::TSubscriber
{
	friend class TLReflection::TGraphTreeItem;
public:
	TGraphTree(TPtr<TLGraph::TGraphBase>& pGraph);

	virtual TRefRef			GetSubscriberRef() const			{	static TRef Ref("GraphTree");	return Ref;	}
	bool					IsValid() const						{	return (m_pWindow && m_pTree && m_pGraph);	}	//	did this init/construct okay and is usable?

protected:
	virtual void			ProcessMessage(TLMessaging::TMessage& Message);
	virtual void			GetColumns(TArray<TRef>& Columns);					//	setup the columns for this tree. overload this to make specific tree displays.

	TGraphTreeItem*			AddNodeItem(TRefRef NodeRef);						//	Add a node to the tree
	void					RemoveNodeItem(TRefRef NodeRef);					//	remove a node from the tree

	void					OnItemAdded(TPtr<TGraphTreeItem>& pItem);							//	update the node->item key array
	void					OnItemRemoved(TGraphTreeItem& Item);								//	update the node->item key array
	TRef					GetNodeFromItem(TGraphTreeItem* pItem);
	TPtr<TGraphTreeItem>&	GetItemFromNode(TRefRef Node);

	TLGraph::TGraphBase&				GetGraph()		{	return *m_pGraph;	}	//	should always be valid, so reference is returned
	
protected:
	TPtr<TLGraph::TGraphBase>			m_pGraph;		//	graph we're associated with
	TPtr<TLGui::TWindow>				m_pWindow;
	TPtr<TLGui::TTree>					m_pTree;		//	todo: don't store this, make the owner control (m_pWindow in this case) be responsible for destruction (as this is what wx does). In theory the smart pointer should still be good but I think as wx explicitly deletes the control this will break the smart pointers
	TKeyArray<TRef,TPtr<TGraphTreeItem> >	m_NodeToItem;	//	node-item lookup table. Might want an alternative to this, eg. store the node on the item's data
};



//------------------------------------------------------
//	
//------------------------------------------------------
FORCEINLINE TLGraph::TGraphBase& TLReflection::TGraphTreeItem::GetGraph()
{
	return GetGraphTree().GetGraph();	
}


