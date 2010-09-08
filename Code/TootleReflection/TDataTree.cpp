/*
 *  TDataTree.cpp
 *  TootleReflection
 *
 *  Created by Graham Reeves on 17/01/2010.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#include "TDataTree.h"

#include <TootleRender/TScreenManager.h>




TLReflection::TDataTreeNodeItem::TDataTreeNodeItem(TRefRef Node,TRefRef Graph) :
	m_Node	( Node ),
	m_Graph	( Graph )
{
	OnNodeRefChanged(TRef_Invalid);
}


//---------------------------------------------------------
//	call when the node changes to update the subscription etc
//---------------------------------------------------------
void TLReflection::TDataTreeNodeItem::OnNodeRefChanged(TRefRef PreviousNode)
{
	TLGraph::TGraphBase* pGraph = GetGraph();
	if ( !pGraph )
		return;
	
	//	unsubscribe from previous node
	TLGraph::TGraphNodeBase* pPrevNode = pGraph->FindNodeBase( PreviousNode );
	if ( pPrevNode )
		this->UnsubscribeFrom( pPrevNode );
	
	//	subscribe to new node
	TLGraph::TGraphNodeBase* pNewNode = pGraph->FindNodeBase( m_Node );
	if ( pNewNode )
	{
		this->SubscribeTo( pNewNode );
	}
	else if ( m_Node.IsValid() )
	{
		TLDebug_Break("Node expected to be found in graph");
	}

}


//---------------------------------------------------------
//	get the binary tree from the node we're associated with
//---------------------------------------------------------
TBinaryTree* TLReflection::TDataTreeNodeItem::GetBinaryTree()
{
	TLGraph::TGraphNodeBase* pNode = GetNode();
	return pNode ? &pNode->GetNodeData() : NULL;
}


//---------------------------------------------------------
//	
//---------------------------------------------------------
void TLReflection::TDataTreeNodeItem::ProcessMessage(TLMessaging::TMessage& Message)
{
	//	catch data changes with the node
	if ( Message.GetMessageRef() == TLCore::Message::OnPropertyChanged )
	{
		TRef DataRef;
		if ( Message.ImportData( TRef_Static4(D,a,t,a), DataRef ) )
		{
			//	if a specific bit of data changed then we need to see if it was ourselves
			TBinaryTree* pData = GetBinaryTree();
			if ( !pData )
				return;
			
			if ( pData->GetDataRef() != DataRef )
			{
				TDebugString Debug_String;
				Debug_String << "wrong data changed, was " << DataRef << ", expecting " << pData->GetDataRef();
				TLDebug_Print( Debug_String );
				return;
			}
		}
		
		//	notify that the value column has changed.
		//OnChanged("Value");
		//OnChanged("Size");
		OnChanged( TRef() );
	}
}			


//---------------------------------------------------------
//	get our node
//---------------------------------------------------------
TLGraph::TGraphBase* TLReflection::TDataTreeNodeItem::GetGraph()
{
	//	get graph
	TLGraph::TGraphBase* pGraph = TLCore::g_pCoreManager->GetManager<TLGraph::TGraphBase>( m_Graph );
	return pGraph;
}

//---------------------------------------------------------
//	get our node
//---------------------------------------------------------
TLGraph::TGraphNodeBase* TLReflection::TDataTreeNodeItem::GetNode()
{
	//	get graph
	TLGraph::TGraphBase* pGraph = GetGraph();
	
	//	missing graph?
	if ( !pGraph )
		return NULL;

	//	get node
	TLGraph::TGraphNodeBase* pNode = pGraph->FindNodeBase( m_Node );
	return pNode;
}


//-------------------------------------------------------------------
//	let the root item dictate how the data is changed
//-------------------------------------------------------------------
bool TLReflection::TDataTreeNodeItem::SetBinaryTreeData(TDataTreeItem& Item,const TBinary& NewData,TRefRef DataRef)
{
	//	only support changing the value atm
	if ( DataRef != "Value" )
		return false;
	
	//	multi-depth data might be a little more complicated!
	if ( Item.GetParent() != this )
		return false;

	TBinaryTree* pItemData = Item.GetBinaryTree();
	if ( !pItemData )
	{
		TLDebug_Break("Item's binary tree expected");
		return false;
	}		

	TLGraph::TGraphBase* pGraph = GetGraph();
	if ( !pGraph )
		return false;	
	
	//	take this new data and send a set property message
	TLMessaging::TMessage SetMessage( TLCore::SetPropertyRef );
	TRef ItemDataRef = pItemData->GetDataRef();
	TBinaryTree* pNewData = SetMessage.AddChild( ItemDataRef );
	pNewData->Copy( NewData );
	
	//	apply this to node
	if ( !pGraph->SendMessageToNode( m_Node, SetMessage ) )
		return false;

	//	no immediate return, so assume failed and a data-change message will come back
	return false;
}

/*
//-------------------------------------------------------------------
//	root constructor
//-------------------------------------------------------------------
TLReflection::TDataTreeItem::TDataTreeItem(TLReflection::TDataTree& DataTree) :
m_pDataTree	( NULL )
{
	SetDataTree( DataTree );
	
	//	root constructor uses the root node in the Data
	m_NodeRef = GetData().GetRootNodeBase()->GetNodeRef();
}


//-------------------------------------------------------------------
//	child constructor
//-------------------------------------------------------------------
TLReflection::TDataTreeItem::TDataTreeItem(TRefRef NodeRef) :
m_NodeRef			( NodeRef ),
m_pDataTree		( NULL )
{
}


TLReflection::TDataTreeItem::~TDataTreeItem()
{
}


//-------------------------------------------------------------
//	add child item and notify owner
//-------------------------------------------------------------
bool TLReflection::TDataTreeItem::AddChild(TPtr<TLGui::TTreeItem>& pChildItem)
{
	//	set Data from parent item asap
	TPtr<TDataTreeItem> pChildCast = pChildItem;
	pChildCast->SetDataTree( GetDataTree() );
	
	//	do super stuff
	if ( !TLGui::TTreeItem::AddChild( pChildItem ) )
		return false;
	
	//	notify our owner to update the node->item index
	GetDataTree().OnItemAdded( pChildCast );
	
	return true;
}

//-------------------------------------------------------------
//	remove child from self and notify owner. Not using a TPtr as we don't need it and we're expecting a definate item
//-------------------------------------------------------------
bool TLReflection::TDataTreeItem::RemoveChild(TTreeItem& ChildItem)
{
	//	do super stuff
	if ( !TLGui::TTreeItem::RemoveChild( ChildItem ) )
		return false;
	
	//	notify our owner to update the node->item index
	TDataTreeItem& ChildCast = static_cast<TDataTreeItem&>( ChildItem );
	GetDataTree().OnItemRemoved( ChildCast );
	
	return true;
}


//---------------------------------------------------------------
//	associate with this Data
//---------------------------------------------------------------
void TLReflection::TDataTreeItem::SetDataTree(TLReflection::TDataTree& DataTree)
{
	//	set pointer
	m_pDataTree = &DataTree;
	if ( !m_pDataTree )
		TLDebug_Break("Data expected. Crash expected next");
	
	//	get Data changes
	this->SubscribeTo( &GetData() );
}

*/

//---------------------------------------------------------------
//	get data to read/write from
//---------------------------------------------------------------
void TLReflection::TDataTreeItem::GetData(TString& DataString,TRefRef DataRef)
{
	TBinaryTree* pData = GetBinaryTree();
	TBinaryTree& Data = *pData;
	if ( !pData )
		return;
	
	switch( DataRef.GetData() )
	{
		case TRef_Static3(R,e,f):
			DataString << Data.GetDataRef();
			break;
			
		case TRef_Static4(T,y,p,e):
			DataString << Data.GetDataTypeHint();
			break;

		case TRef_Static4(S,i,z,e):
			DataString << Data.GetSize();
			break;
	}
}

//---------------------------------------------------------------
//	get data to read/write from
//---------------------------------------------------------------
TBinary* TLReflection::TDataTreeItem::GetData(TRefRef DataRef)
{
	//	only applicable to the value
	if ( DataRef != "Value" )
		return NULL;

	TBinaryTree* pData = GetBinaryTree();
	return pData;
}


//---------------------------------------------------------------
//	create child items
//---------------------------------------------------------------
void TLReflection::TDataTreeItem::CreateChildren()
{
	//	get our data
	TBinaryTree* pData = GetBinaryTree();
	if ( !pData )
	{
		TLDebug_Break("Data expected");
		return;
	}

	//	add an item for every child data
	TPtrArray<TBinaryTree>& DataChildren = pData->GetChildren();
	for ( u32 c=0;	c<DataChildren.GetSize();	c++ )
	{
		TPtr<TBinaryTree>& pChildData = DataChildren[c];
		TPtr<TTreeItem> pChildItem = new TDataTreeChildItem( pChildData );
		this->AddChild( pChildItem );
	}
}

/*

//--------------------------------------------------------------
//	this tree item was selected
//--------------------------------------------------------------
bool TLReflection::TDataTreeItem::OnSelected()
{
	return false;
}

//--------------------------------------------------------------
//	Tree item lost selection
//--------------------------------------------------------------
bool TLReflection::TDataTreeItem::OnUnselected()
{
	return false;
}

//--------------------------------------------------------------
//	popup a menu for this item on right-click
//--------------------------------------------------------------
bool TLReflection::TDataTreeItem::OnRightClick()	
{
	//return GetDataTree().CreatePopupMenu(*this);
	return false;
}

*/

//-------------------------------------------------------------
//	new data to put into this column
//--------------------------------------------------------------
bool TLReflection::TDataTreeItem::SetData(const TBinary& NewData,TRefRef DataRef)
{
	//	due to the way this works, we propogate the changes up the tree
	return SetBinaryTreeData( *this, NewData, DataRef );
}


//--------------------------------------------------------------
//	setup window & controls associated with this Data
//--------------------------------------------------------------
TLReflection::TNodeDataTree::TNodeDataTree(TRefRef Node,TRefRef Graph)
{
	//	create window
	TRef WindowRef = Node;
	m_pWindow = TLGui::CreateGuiWindow( WindowRef );
	if ( !m_pWindow )
	{
		TLDebug_Print("Failed to create window for TLReflection::TDataTree");
		return;
	}
	
	//	check params
	if ( !Node.IsValid() || !Graph.IsValid() )
	{
		TLDebug_Break("valid node/graph expected");
		return;
	}
	
	if ( !CreateTree( Node, Graph ) )
		return;

	//	show the window
	if ( m_pWindow )
	{
		Type2<u16> Size( 300, 400 );
		m_pWindow->SetSize( Size );
		m_pWindow->Show();
	}
}


//--------------------------------------------------------------
//	re/create the tree
//--------------------------------------------------------------
bool TLReflection::TNodeDataTree::CreateTree(TRefRef Node,TRefRef Graph)
{
	//	setup column structure for the tree
	TFixedArray<TRef,100> Columns;
	
	Columns.Add("Ref");		//	data's ref
	Columns.Add("Value");	//	data's value
	Columns.Add("Size");	//	data's value size
	Columns.Add("Type");	//	data's type
	
	//	get a root item for the tree
	TPtr<TDataTreeNodeItem> pRootDataItem = new TDataTreeNodeItem( Node, Graph );
	TPtr<TLGui::TTreeItem> pRootItem = pRootDataItem;
	
	//	setup a data view control on the window starting at the root node
	TRef ControlRef = Node;
	m_pTree = TLGui::CreateTree( *m_pWindow, ControlRef, pRootItem, Columns );
	if ( !m_pTree )
	{
		TLDebug_Break("Failed to create tree control");
		return false;
	}

	return true;
}
/*

//-------------------------------------------------------------
//	popup a menu for this item
//-------------------------------------------------------------
bool TLReflection::TDataTree::CreatePopupMenu(TDataTreeItem& Item)
{
	return false;
}

/*

//-------------------------------------------------------------
//	
//-------------------------------------------------------------
TPtr<TLAsset::TMenu> TLReflection::TDataTree::CreateMenu(TRefRef MenuRef)
{	
	return TLMenu::TMenuController::CreateMenu( MenuRef );
}

//-------------------------------------------------------------
//	
//-------------------------------------------------------------
Bool TLReflection::TDataTree::ExecuteCommand(TRefRef MenuCommand,TBinaryTree& MenuItemData)
{
	return TLMenu::TMenuController::ExecuteCommand( MenuCommand, MenuItemData );	
}

//-------------------------------------------------------------
//	moved onto new menu
//-------------------------------------------------------------
void TLReflection::TDataTree::OnMenuOpen()
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
void TLReflection::TDataTree::OnMenuCloseAll()
{
	m_pMenuWrapper = NULL;
	TLMenu::TMenuController::OnMenuCloseAll();
}
 */


TLReflection::TDataTreeChildItem::TDataTreeChildItem(TPtr<TBinaryTree>& pData) :
	m_pData		( pData )
{
}



//-------------------------------------------------------------
//	let the root item dictate how the data is changed
//-------------------------------------------------------------
bool TLReflection::TDataTreeChildItem::SetBinaryTreeData(TDataTreeItem& Item,const TBinary& NewData,TRefRef DataRef)
{
	//	propogate upwards and let the root handle the data
	TDataTreeItem* pParentItem = static_cast<TDataTreeItem*>( GetParent() );
	//	shouldn't have a child item which is root...
	if ( !pParentItem )
	{
		TLDebug_Break("We have a child data item as the root. wrong setup!");
		return false;
	}

	return pParentItem->SetBinaryTreeData( Item, NewData, DataRef );
}

