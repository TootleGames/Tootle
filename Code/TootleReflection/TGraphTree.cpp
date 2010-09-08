/*
 *  TGraphTree.cpp
 *  TootleReflection
 *
 *  Created by Graham Reeves on 17/01/2010.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#include "TGraphTree.h"
#include "TLReflection.h"
#include <TootleRender/TScreenManager.h>
#include <TootleRender/TRenderGraph.h>



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
TLReflection::TGraphTreeItem::TGraphTreeItem(TRefRef NodeRef) :
	m_NodeRef			( NodeRef ),
	m_pGraphTree		( NULL )
{
}


TLReflection::TGraphTreeItem::~TGraphTreeItem()
{
}


//-------------------------------------------------------------
//	add child item and notify owner
//-------------------------------------------------------------
bool TLReflection::TGraphTreeItem::AddChild(TPtr<TLGui::TTreeItem>& pChildItem)
{
	//	set graph from parent item asap
	TPtr<TGraphTreeItem> pChildCast = pChildItem;
	pChildCast->SetGraphTree( GetGraphTree() );
	
	//	do super stuff
	if ( !TLGui::TTreeItem::AddChild( pChildItem ) )
		return false;
	
	//	notify our owner to update the node->item index
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
//	
//---------------------------------------------------------------
bool TLReflection::TGraphTreeItem::IsDataMutable(TRefRef DataRef)
{
	//	allow the change of the ref
	if ( DataRef == "Ref" )
		return true;
	
	//	allow change of type
	if ( DataRef == "Type" )
		return true;
	
	return false;
}


//---------------------------------------------------------------
//	
//---------------------------------------------------------------
bool TLReflection::TGraphTreeItem::SetData(const TString& DataString,TRefRef DataRef)
{
	TLGraph::TGraphBase& Graph = GetGraph();

	//	change the ref
	if ( DataRef == "Ref" )
	{
		//	get new ref
		TRef NewRef = DataString;
		if ( !Graph.RenameNode( GetNodeRef(), NewRef ) )
			return false;
	
		//	the change isn't instant so respond with failure
		return false;
	}

	//	change type
	if ( DataRef == "Type" )
	{
		TRef NewTypeRef = DataString;
		if ( !Graph.ChangeNodeType( GetNodeRef(), NewTypeRef ) )
			return false;
		
		//	the change isn't instant so respond with failure
		return false;
	}
	
	return false;
}

//---------------------------------------------------------------
//	Get data for this column
//---------------------------------------------------------------
TBinary* TLReflection::TGraphTreeItem::GetData(TRefRef DataRef)
{
	//	get node
	TLGraph::TGraphNodeBase* pNode = GetNode();

	//	this is possibly okay if the node hasn't been added to the graph yet...
	//	but I don't think we'll get here if the node hasn't been added
	if ( !pNode )
	{
	//	TLDebug_Break("Node not added to graph yet?");
		return NULL;
	}
	
	//	todo: do something about caching this so we only update the node data when it's changed
	pNode->UpdateNodeData();
	TPtr<TBinaryTree>& pData = pNode->GetNodeData().GetChild( DataRef );
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
		TPtr<TTreeItem> pChildItem = new TGraphTreeItem( ChildNodeRef );
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
//	this tree item was selected
//--------------------------------------------------------------
void TLReflection::TGraphTreeItem::OnSelectedChanged(bool Selected)
{
	//	do nothing if node no longer exists
	TLGraph::TGraphNodeBase* pNode = GetNode();
	if ( !pNode )
		return;

	//	if the node data window is up, change it to show this node
	if ( Selected && TLReflection::IsNodeDataWindowVisible() )
	{
		TLReflection::ShowNodeDataWindow( GetNodeRef(), GetGraph().GetGraphRef(), false );
	}
	else
	{
		//	hide data window if it's up?
	}

	//	gr: put graph-specific stuff in overloaded stuff? or send messages?
	//		would be nice to keep all refelction stuff OUT of the actual node code...
	
	//	if render node, then toggle the bounds boxes
	TRefRef GraphRef = GetGraphRef();
	if ( GraphRef == TLRender::g_pRendergraph->GetGraphRef() )
	{
		//TLRender::TRenderNode& Node = static_cast<TLRender::TRenderNode&>( *pNode );
		TLMessaging::TMessage SetMessage(TLCore::SetPropertyRef);
		TRef DebugDatumProperty = Selected ? TLRender::TRenderNode::Properties::Debug_Datum : TLRender::TRenderNode::Properties::Debug_DatumRemove;
		SetMessage.ExportData( DebugDatumProperty, TRef(TLRender_TRenderNode_DatumBoundsBox) );
		SetMessage.ExportData( DebugDatumProperty, TRef(TLRender_TRenderNode_DatumBoundsBox2D) );
		SetMessage.ExportData( DebugDatumProperty, TRef(TLRender_TRenderNode_DatumBoundsSphere) );
		SetMessage.ExportData( DebugDatumProperty, TRef(TLRender_TRenderNode_DatumBoundsSphere2D) );
		GetGraph().SendMessageToNode( m_NodeRef, SetMessage );
	}

}


//--------------------------------------------------------------
//	popup a menu for this item on right-click
//--------------------------------------------------------------
bool TLReflection::TGraphTreeItem::OnRightClick()	
{
	return GetGraphTree().CreatePopupMenu(*this);
}

	
//--------------------------------------------------------------
//	setup window & controls associated with this graph
//--------------------------------------------------------------
TLReflection::TGraphTree::TGraphTree(TLGraph::TGraphBase& Graph) :
	m_pGraph	( &Graph )
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
	TFixedArray<TRef,100> Columns;

	Columns.Add("Ref");		//	node's ref
	Columns.Add("Type");	//	node's type

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
	
	//	callback for the root item, must be called as it's usually from AddedChild
	OnItemAdded( pRootGraphItem );

	//	subscribe to graph changes
	this->SubscribeTo( m_pGraph );

	//	show the window
	if ( m_pWindow )
	{
		Type2<u16> Size( 300, 400 );
		m_pWindow->SetSize( Size );

		//	position it to the left of the main screen
		TLRender::TScreen* pDefaultScreen = TLRender::g_pScreenManager ? TLRender::g_pScreenManager->GetDefaultScreen().GetObjectPointer() : NULL;
		TLGui::TWindow* pDefaultScreenWindow = pDefaultScreen ? pDefaultScreen->GetWindow() : NULL;
		if ( pDefaultScreenWindow )
		{
			Type2<u16> Pos = pDefaultScreenWindow->GetPosition();
			Pos.x -= Size.x;
			m_pWindow->SetPosition( Pos );
		}
		
		m_pWindow->Show();
	}
}


//--------------------------------------------------------------
//
//--------------------------------------------------------------
void TLReflection::TGraphTree::ProcessMessage(TLMessaging::TMessage& Message)
{
	switch ( Message.GetMessageRef().GetData() )
	{
	//	catch graph changes and update the data tree
	//	todo: check message if from our graph
		case TRef_Static(N,o,d,e,R):
		{
			TRef Node;
			if ( !Message.ImportData( "Node", Node ) )
			{
				TLDebug_Break("Node expected");
				return;
			}
			RemoveNodeItem( Node );
		}
		break;

		case TRef_Static(N,o,d,e,A):
		{
			TRef Node,ParentNode;
			if ( !Message.ImportData("Node", Node) )
			{
				TLDebug_Break("NodeRef expected");
				return;
			}
			
			AddNodeItem( Node );
		}
		break;
		
		case TRef_Static(N,o,d,e,M):
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
		break;

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
	TPtr<TLReflection::TGraphTreeItem> pNodeGraphTreeItem = new TGraphTreeItem( NodeRef );
	
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
		return;
	}
/*
	//	now add the children - this is to populate the initial graph
	TLGraph::TGraphNodeBase* pNode = GetGraph().FindNodeBase( pItem->GetNodeRef() );
	if ( !pNode )
	{
		TLDebug_Break("Node expected");
		return;
	}
	
	THeapArray<TRef> ChildNodes;
	pNode->GetChildren( ChildNodes );
	for ( u32 c=0;	c<ChildNodes.GetSize();	c++ )
	{
		AddNodeItem( ChildNodes[c] );
	}
 */
}


//-------------------------------------------------------------
//	update the node->item key array
//-------------------------------------------------------------
void TLReflection::TGraphTree::OnItemRemoved(TGraphTreeItem& Item)
{
	m_NodeToItem.Remove( Item.GetNodeRef() );
}


//-------------------------------------------------------------
//	popup a menu for this item
//-------------------------------------------------------------
bool TLReflection::TGraphTree::CreatePopupMenu(TGraphTreeItem& Item)
{
	//	our hacky way of doing a node-based popup menu...
	m_PopupMenuNodes.Empty();
	m_PopupMenuNodes << Item.GetNodeRef();
	
	return OpenMenu("NodePop");
}



//-------------------------------------------------------------
//	create menu for the node-popup for these nodes
//-------------------------------------------------------------
TPtr<TLAsset::TMenu> TLReflection::TGraphTree::CreateNodePopupMenu(TRefRef MenuRef,const TArray<TRef>& Nodes)
{
	TPtr<TLAsset::TMenu> pMenu = new TLAsset::TMenu( MenuRef );
	
	//	node-data we stick on each menu item
	TBinaryTree MenuItemData("ItemData");
	MenuItemData.ExportArray("Nodes", Nodes );
	MenuItemData.ExportData("Graph", GetGraph().GetGraphRef() );
	
	{
		TLAsset::TMenu::TMenuItem& MenuItem = *pMenu->AddMenuItem();
		MenuItem.SetString("Add child");
		MenuItem.SetCommand("AddNode");
		MenuItem.SetData( MenuItemData );
		MenuItem.SetEnabled( Nodes.GetSize() == 1 );
	}
	
	{
		TLAsset::TMenu::TMenuItem& MenuItem = *pMenu->AddMenuItem();
		MenuItem.SetString("Show data...");
		MenuItem.SetCommand("ShowData");
		MenuItem.SetData( MenuItemData );
	}
	
	{
		TLAsset::TMenu::TMenuItem& MenuItem = *pMenu->AddMenuItem();
		MenuItem.SetString("Delete");
		MenuItem.SetCommand("RmNode");
		MenuItem.SetData( MenuItemData );
		
		//	don't allow the root to be deleted
		if ( Nodes.Exists( GetGraph().GetRootNodeRef() ) )
			MenuItem.SetEnabled( false );
		else
			MenuItem.SetEnabled( Nodes.GetSize() > 0 );
	}
	
	{
		TLAsset::TMenu::TMenuItem& MenuItem = *pMenu->AddMenuItem();
		MenuItem.SetString("Delete Children");
		MenuItem.SetCommand("RmChildren");
		MenuItem.SetData( MenuItemData );
		
		//	only allow if there is at least one node with children
		bool HasChildren = false;
		for ( u32 i=0;	i<Nodes.GetSize() && !HasChildren;	i++ )
		{
			TLGraph::TGraphNodeBase* pNode = GetGraph().FindNodeBase( Nodes[i] );
			if ( !pNode )
				continue;
			
			HasChildren |= pNode->HasChildren();
		}

		MenuItem.SetEnabled( HasChildren );
	}
	
	{
		TLAsset::TMenu::TMenuItem& MenuItem = *pMenu->AddMenuItem();
		MenuItem.SetString("Export Scheme...");
		MenuItem.SetCommand("ExScheme");
		MenuItem.SetData( MenuItemData );			
		MenuItem.SetEnabled( Nodes.GetSize() == 1 );
	}

	return pMenu;
}


//-------------------------------------------------------------
//	
//-------------------------------------------------------------
TPtr<TLAsset::TMenu> TLReflection::TGraphTree::CreateMenu(TRefRef MenuRef)
{
	if ( MenuRef == "NodePop" )
	{
		TPtr<TLAsset::TMenu> pMenu = CreateNodePopupMenu( MenuRef, m_PopupMenuNodes );
		return pMenu;
	}
	
	return TLMenu::TMenuController::CreateMenu( MenuRef );
}

//-------------------------------------------------------------
//	
//-------------------------------------------------------------
Bool TLReflection::TGraphTree::ExecuteCommand(TRefRef MenuCommand,TBinaryTree& MenuItemData)
{
	//	get nodes
	THeapArray<TRef> Nodes;
	MenuItemData.ImportArrays("Nodes", Nodes );
//	MenuItemData.ExportData("Graph", GetGraph().GetGraphRef() );
	TLGraph::TGraphBase& Graph = GetGraph();
	
	switch ( MenuCommand.GetData() )
	{
		case TRef_Static(A,d,d,N,o):
		{
			//	create new node[s]
			THeapArray<TRef> NewNodes;
			for ( u32 n=0;	n<Nodes.GetSize();	n++ )
			{
				Graph.CreateNode( "New", TRef(), Nodes[n] );
			}
		}
		break;
		
		case TRef_Static(R,m,N,o,d):
		{
			//	remove nodes from graph
			for ( u32 n=0;	n<Nodes.GetSize();	n++ )
				Graph.RemoveNode( Nodes[n] );
		}
		break;

		case TRef_Static(R,m,C,h,i):
		{
			//	remove nodes from graph
			for ( u32 n=0;	n<Nodes.GetSize();	n++ )
				Graph.RemoveChildren( Nodes[n] );
		}
		break;
	
		case TRef_Static(E,x,S,c,h):
		{
			TDebugString Debug_String;
			Debug_String << "todo: export scheme from nodes ";
			for ( u32 n=0;	n<Nodes.GetSize();	n++ )
				Debug_String << Nodes[n];
			
			TLDebug_Break( Debug_String );
		}
		break;
			
		case TRef_Static(S,h,o,w,D):
		{
			//	popup a data window for each node
			for ( u32 n=0;	n<Nodes.GetSize();	n++ )
			{
				TLReflection::ShowNodeDataWindow( Nodes[n], Graph.GetGraphRef() );
			}
		}
		break;
	}
	
	return TLMenu::TMenuController::ExecuteCommand( MenuCommand, MenuItemData );	
}

//-------------------------------------------------------------
//	moved onto new menu
//-------------------------------------------------------------
void TLReflection::TGraphTree::OnMenuOpen()
{
	//	replace old menu
	TRef MenuRef = GetCurrentMenu()->GetMenuRef();
	bool IsPopupMenu = (MenuRef=="NodePop");

	TLGui::TMenuHandler* pMenuHandler = m_pWindow->GetMenuHandler();
	if ( pMenuHandler )
	{
		m_pMenuWrapper = new TLGui::TMenuWrapper( *this, *pMenuHandler, IsPopupMenu );
	}
	else
	{
		TLDebug_Break("trying to open menu on a window that doesn't support menus. Change reflection code to catch this earlier?");
	}
	
	TLMenu::TMenuController::OnMenuOpen();
}

//-------------------------------------------------------------
//	closed all menus
//-------------------------------------------------------------
void TLReflection::TGraphTree::OnMenuCloseAll()
{
	m_pMenuWrapper = NULL;
	TLMenu::TMenuController::OnMenuCloseAll();
}
