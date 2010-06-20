/*
 *  TGraphTree.cpp
 *  TootleReflection
 *
 *  Created by Graham Reeves on 17/01/2010.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#include "TGraphTree.h"



//-------------------------------------------------------------------
//	root constructor
//-------------------------------------------------------------------
TLReflection::TGraphTreeItem::TGraphTreeItem(TLReflection::TGraphTree& GraphTree) :
	m_pGraphTree	( NULL )
{
	SetGraphTree( GraphTree );
	
	//	root constructor uses the root node in the graph
	m_NodeRef = GetGraph().GetRootNodeBase()->GetNodeRef();
}


//-------------------------------------------------------------------
//	child constructor
//-------------------------------------------------------------------
TLReflection::TGraphTreeItem::TGraphTreeItem(TTreeItem& Parent,TRefRef NodeRef) :
	TLGui::TTreeItem	( Parent ),
	m_NodeRef			( NodeRef ),
	m_pGraphTree		( NULL )
{
	//	copy graph from parent item
	TGraphTreeItem& ParentItem = static_cast<TGraphTreeItem&>( Parent );
	SetGraphTree( ParentItem.GetGraphTree() );
}


TLReflection::TGraphTreeItem::~TGraphTreeItem()
{
}


//-------------------------------------------------------------
//	add child item and notify owner
//-------------------------------------------------------------
bool TLReflection::TGraphTreeItem::AddChild(TPtr<TLGui::TTreeItem>& pChildItem)
{
	//	do super stuff
	if ( !TLGui::TTreeItem::AddChild( pChildItem ) )
		return false;
	
	//	notify our owner to update the node->item index
	TPtr<TGraphTreeItem> pChildCast = pChildItem;
	GetGraphTree().OnItemAdded( pChildCast );
	
	return true;
}

//-------------------------------------------------------------
//	remove child from self and notify owner. Not using a TPtr as we don't need it and we're expecting a definate item
//-------------------------------------------------------------
bool TLReflection::TGraphTreeItem::RemoveChild(TTreeItem& ChildItem)
{
	//	do super stuff
	if ( !TLGui::TTreeItem::RemoveChild( ChildItem ) )
		return false;
	
	//	notify our owner to update the node->item index
	TGraphTreeItem& ChildCast = static_cast<TGraphTreeItem&>( ChildItem );
	GetGraphTree().OnItemRemoved( ChildCast );
	
	return true;
}


//---------------------------------------------------------------
//	associate with this graph
//---------------------------------------------------------------
void TLReflection::TGraphTreeItem::SetGraphTree(TLReflection::TGraphTree& GraphTree)
{
	//	set pointer
	m_pGraphTree = &GraphTree;
	if ( !m_pGraphTree )
		TLDebug_Break("Graph expected. Crash expected next");

	//	get graph changes
	this->SubscribeTo( &GetGraph() );
}


//---------------------------------------------------------------
//	get data to read/write from
//---------------------------------------------------------------
TBinary* TLReflection::TGraphTreeItem::GetData(TRefRef DataRef)
{
	//	get node
	TLGraph::TGraphNodeBase* pNode = GetNode();
	
	//	this is possibly okay if the node hasn't been added to the graph yet...
	//	but I don't think we'll get here if the node hasn't been added
	if ( !pNode )
	{
		TLDebug_Break("Node not added to graph yet?");
		return NULL;
	}
	
	//	todo: do something about caching this so we only update the node data when it's changed
	pNode->UpdateNodeData();
	TPtr<TBinaryTree>& pData = pNode->GetNodeData().GetChild( DataRef );
	if ( !pData )
	{
		TTempString Debug_String;
		Debug_String << "Data " << DataRef << " not found on node " << GetNodeRef();
		TLDebug_Print( Debug_String );
	}
		
	return pData;
}

//---------------------------------------------------------------
//	create child items
//---------------------------------------------------------------
void TLReflection::TGraphTreeItem::CreateChildren()
{
	//	get node
	TLGraph::TGraphNodeBase* pNode = GetNode();
	if ( !pNode )
	{
		TLDebug_Break("Node expected");
		return;
	}
	
	//	add an item for every child node
	THeapArray<TRef> ChildNodeRefs;
	pNode->GetChildren( ChildNodeRefs );
	for ( u32 c=0;	c<ChildNodeRefs.GetSize();	c++ )
	{
		TRefRef ChildNodeRef = ChildNodeRefs[c];
		TPtr<TTreeItem> pChildItem = new TGraphTreeItem( *this, ChildNodeRef );
		this->AddChild( pChildItem );
	}
}


//-------------------------------------------------------------
//	catch changes to the node we're associated with
//-------------------------------------------------------------
void TLReflection::TGraphTreeItem::ProcessMessage(TLMessaging::TMessage& Message)
{
	//	only interested in messages from our node
	if ( Message.GetSenderRef() != GetNodeRef() )
		return;
	
	//	if data changed, call Tree.ItemChanged(this)
}

	
//--------------------------------------------------------------
//	setup window & controls associated with this graph
//--------------------------------------------------------------
TLReflection::TGraphTree::TGraphTree(TPtr<TLGraph::TGraphBase>& pGraph) :
	m_pGraph	( pGraph )
{
	//	create window
	TRef WindowRef = GetGraph().GetGraphRef();
	m_pWindow = TLGui::CreateGuiWindow( WindowRef );
	if ( !m_pWindow )
	{
		TLDebug_Print("Failed to create window for TLReflection::TGraphTree");
		return;
	}

	//	check params
	if ( !m_pGraph )
	{
		TLDebug_Break("Graph expected");
		return;
	}
	
	//	setup column structure for the tree
	THeapArray<TRef> Columns;
#pragma message("note: use of virtuals in constructor. May have to make this a constructor param. Overloaded classes can provide their columns through that")
	GetColumns( Columns );

	//	get a root item for the tree
	TPtr<TGraphTreeItem> pRootGraphItem = new TGraphTreeItem( *this );
	TPtr<TLGui::TTreeItem> pRootItem = pRootGraphItem;
	
	//	setup a data view control on the window starting at the root node
	TRef ControlRef = GetGraph().GetGraphRef();
	m_pTree = TLGui::CreateTree( *m_pWindow, ControlRef, pRootItem, Columns );
	if ( !m_pTree )
	{
		TLDebug_Break("Failed to create tree control");
		return;
	}
	
	//	subscribe to graph changes
	this->SubscribeTo( m_pGraph );

	//	have to manually notify of the root being added atm...
	OnItemAdded( pRootGraphItem );

	//	show the window
	if ( m_pWindow )
		m_pWindow->Show();
}


//--------------------------------------------------------------
//	setup the columns for this tree. overload this to make specific tree displays.
//--------------------------------------------------------------
void TLReflection::TGraphTree::GetColumns(TArray<TRef>& Columns)
{
	//	generic graph node information
	Columns.Add("Ref");		//	node's ref
	Columns.Add("Type");	//	node's type
}
	

//--------------------------------------------------------------
//
//--------------------------------------------------------------
void TLReflection::TGraphTree::ProcessMessage(TLMessaging::TMessage& Message)
{
	//	catch graph changes and update the data tree
	if ( Message.GetMessageRef() == "NodeRemoved" )
	{
		TRef Node;
		if ( !Message.ImportData( "Node", Node ) )
		{
			TLDebug_Break("Node expected");
			return;
		}
		RemoveNodeItem( Node );
	}
	else if ( Message.GetMessageRef() == "NodeAdded" )
	{
		TRef Node,ParentNode;
		if ( !Message.ImportData("Node", Node) )
		{
			TLDebug_Break("NodeRef expected");
			return;
		}
		
		AddNodeItem( Node );
	}
	else if ( Message.GetMessageRef() == "NodeMoved" )
	{
		TRef Node;
		if ( !Message.ImportData( "Node", Node ) )
		{
			TLDebug_Break("NodeRef expected");
			return;
		}

		TRef NewParentNode;
		if ( !Message.ImportData( "Parent", NewParentNode ) )
		{
			TLDebug_Break("New parent node ref expected");
			return;
		}
		
		//	re-parent the item
		TPtr<TLGui::TTreeItem> pNodeItem = GetItemFromNode( Node );
		if ( !pNodeItem )
		{
			TLDebug_Break("Item for node not found");
			return;
		}

		//	add to new parent
		TPtr<TLGui::TTreeItem> pNewParentNodeItem = GetItemFromNode( NewParentNode );
		if ( !pNewParentNodeItem )
		{
			TLDebug_Break("New parent node item not found");
		}
		else
		{
			pNewParentNodeItem->AddChild( pNodeItem );
		}
	}
}


//---------------------------------------------------------------
//	
//---------------------------------------------------------------
TRef TLReflection::TGraphTree::GetNodeFromItem(TLReflection::TGraphTreeItem* pItem)
{
	//	find the KEY (Node) from the value (Item)
	const TRef* pNodeRef = m_NodeToItem.FindKey( pItem );
	return pNodeRef ? *pNodeRef : TRef_Invalid;
}


//---------------------------------------------------------------
//
//---------------------------------------------------------------
TPtr<TLReflection::TGraphTreeItem>& TLReflection::TGraphTree::GetItemFromNode(TRefRef NodeRef)
{
	//	find the VALUE (Item) from the KEY (Node)
	TPtr<TLReflection::TGraphTreeItem>* ppItem = m_NodeToItem.Find( NodeRef );
	if ( !ppItem )
		return TLPtr::GetNullPtr<TLReflection::TGraphTreeItem>();
	
	return *ppItem;
}



//---------------------------------------------------------------
//	add an entry to the tree for this node. returns the tree item's ref
//---------------------------------------------------------------
TLReflection::TGraphTreeItem* TLReflection::TGraphTree::AddNodeItem(TRefRef NodeRef)
{
	//	check this doesn't already exist
	TPtr<TGraphTreeItem>& pCurrentItem = GetItemFromNode( NodeRef );
	if ( pCurrentItem )
	{
		TTempString Debug_String;
		Debug_String << "TreeItem already exists for node " << NodeRef;
		TLDebug_Break( Debug_String );
		return pCurrentItem;
	}
	
	//	get the node's parent so we can get the parent item to add this new item under
	TLGraph::TGraphNodeBase* pNode = GetGraph().FindNodeBase( NodeRef );
	if ( !pNode )
	{
		TTempString Debug_String;
		Debug_String << "Failed to find node " << NodeRef << " in graph " << GetGraph().GetGraphRef();
		TLDebug_Break( Debug_String );
		return NULL;
	}
	
	//	get the node's parent
	const TLGraph::TGraphNodeBase* pParentNode = pNode->GetParentBase();
	if ( !pParentNode )
	{
		TTempString Debug_String;
		Debug_String << "Adding node (" << NodeRef << " in graph " << GetGraph().GetGraphRef() << ") to tree with no parent node... can't be adding root so error.";
		TLDebug_Break( Debug_String );
		return NULL;
	}
	
	//	get the tree item for the parent
	TPtr<TGraphTreeItem>& pParentItem = GetItemFromNode( pParentNode->GetNodeRef() );
	if ( !pParentItem )
	{
		TTempString Debug_String;
		Debug_String << "Failed to add node (" << NodeRef << " in graph " << GetGraph().GetGraphRef() << ") to tree, parent item (" << pParentNode->GetNodeRef() << ") is not in tree.";
		TLDebug_Break( Debug_String );
		return NULL;
	}
	
	//	create new item
	TPtr<TLReflection::TGraphTreeItem> pNodeGraphTreeItem = new TGraphTreeItem( *pParentItem, NodeRef );
	
	//	cast TPtr down and add new item to tree
	TPtr<TLGui::TTreeItem> pNodeItem = pNodeGraphTreeItem;
	if ( !pParentItem->AddChild( pNodeItem ) )
	{
		TLDebug_Break("Failed to add new node item to parent item.");
		return NULL;
	}
	
	TTempString Debug_String;
	Debug_String << "Added node " << NodeRef << " to tree";
	TLDebug_Print( Debug_String );
	
	return pNodeGraphTreeItem;
}


//-------------------------------------------------------------
//	remove item from the tree based on the NODE ref
//-------------------------------------------------------------
void TLReflection::TGraphTree::RemoveNodeItem(TRefRef NodeRef)
{
	//	remove entry from lookup table
	TPtr<TGraphTreeItem> pItem = GetItemFromNode( NodeRef );
	
	//	must not be in the tree...
	if ( !pItem )
		return;
	
	//	remove from tree
	TLGui::TTreeItem* pItemParent = pItem->GetParent();
	if ( !pItemParent )
	{
		TTempString Debug_String;
		Debug_String << "trying to remove node " << NodeRef << " from tree, but failed to find item's parent... cannot remove root tree item!";
		TLDebug_Break( Debug_String );
		return;
	}

	//	remove from parent
	pItemParent->RemoveChild( *pItem );
}


//-------------------------------------------------------------
//	update the node->item key array
//-------------------------------------------------------------
void TLReflection::TGraphTree::OnItemAdded(TPtr<TGraphTreeItem>& pItem)
{
	if ( !pItem )
	{
		TLDebug_Break("Item expected");
		return;
	}
	
	//	add node -> item lookup entry
	if ( !m_NodeToItem.Add( pItem->GetNodeRef(), pItem ) )
	{
		TLDebug_Break("Failed to add node->item lookup entry");
	}
}


//-------------------------------------------------------------
//	update the node->item key array
//-------------------------------------------------------------
void TLReflection::TGraphTree::OnItemRemoved(TGraphTreeItem& Item)
{
	m_NodeToItem.Remove( Item.GetNodeRef() );
}

