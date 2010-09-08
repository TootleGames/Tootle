/*
 *  TDataTree.h
 *  TootleReflection
 *
 *  Created by Graham Reeves on 17/01/2010.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */
#pragma once

#include <TootleGui/TWindow.h>
#include <TootleGui/TMenu.h>
#include <TootleGui/TTree.h>
#include <TootleCore/TBinaryTree.h>
#include <TootleCore/TLMessaging.h>


namespace TLReflection
{
	class TDataTreeItem;	//	item for a TLGui::Tree which maps items to nodes and vice versa
	class TDataTreeNodeItem;
	class TDataTreeChildItem;
	
	class TNodeDataTree;		//	window which contains a tree displaying the contents of a Data
}


//---------------------------------------------------------
//	base binarydata item type
//---------------------------------------------------------
class TLReflection::TDataTreeItem : public TLGui::TTreeItem/*, public TLMessaging::TSubscriber*/
{
public:
	/*
//	virtual TRefRef				GetSubscriberRef() const			{	return m_NodeRef;	}
//	TRefRef						GetNodeRef() const 					{	return m_NodeRef;	}
	
	virtual bool				AddChild(TPtr<TTreeItem>& pChildItem);		//	add child item and notify owner
	virtual bool				RemoveChild(TTreeItem& ChildItem);			//	remove child from self and notify owner. Not using a TPtr as we don't need it and we're expecting a definate item
	
	virtual bool				OnSelected();		//	item has been selected
	virtual bool				OnUnselected();		//	item has been selected
	virtual bool				OnRightClick();		//	popup a menu for this item on right-click
	*/
	virtual bool				SetData(const TBinary& NewData,TRefRef DataRef);			//	new data to put into this column

	virtual TBinaryTree*		GetBinaryTree()=0;
	virtual bool				SetBinaryTreeData(TDataTreeItem& Item,const TBinary& NewData,TRefRef DataRef)=0;	//	let the root item dictate how the data is changed
	

protected:
//	virtual void				ProcessMessage(TLMessaging::TMessage& Message);	//	catch changes to the node we're associated with

	virtual void				GetData(TString& DataString,TRefRef DataRef);		//	get string to put in columns when TBinary data isn't availible
	virtual TBinary*			GetData(TRefRef DataRef);	//	get data to read/write from
	virtual void				CreateChildren();			//	create child items
};





//---------------------------------------------------------
//	root item associated with a node
//---------------------------------------------------------
class TLReflection::TDataTreeNodeItem : public TLReflection::TDataTreeItem, public TLMessaging::TSubscriber
{
public:
	TDataTreeNodeItem(TRefRef Node,TRefRef Graph);
			
	virtual TRefRef				GetSubscriberRef() const	{	static TRef Ref("ndata");	return Ref;	}
	
	virtual TBinaryTree*		GetBinaryTree();	//	get the binary tree from the node we're associated with
	virtual bool				SetBinaryTreeData(TDataTreeItem& Item,const TBinary& NewData,TRefRef DataRef);	//	let the root item dictate how the data is changed

	TLGraph::TGraphNodeBase*	GetNode();			//	get our node
	TLGraph::TGraphBase*		GetGraph();

protected:
	virtual void				ProcessMessage(TLMessaging::TMessage& Message);
	void						OnNodeRefChanged(TRefRef PreviousNode);	//	call when the node changes to update the subscription etc

protected:
	TRef						m_Node;
	TRef						m_Graph;
};



//---------------------------------------------------------
//
//---------------------------------------------------------
class TLReflection::TDataTreeChildItem : public TLReflection::TDataTreeItem
{
public:
	TDataTreeChildItem(TPtr<TBinaryTree>& pData);
	
	virtual TBinaryTree*		GetBinaryTree()		{	return m_pData;	}
	virtual bool				SetBinaryTreeData(TDataTreeItem& Item,const TBinary& NewData,TRefRef DataRef);	//	let the root item dictate how the data is changed

protected:
	TPtr<TBinaryTree>			m_pData;
};



//--------------------------------------------------------------
//	window which contains a tree displaying the contents of a Data
//--------------------------------------------------------------
class TLReflection::TNodeDataTree// : public TLMenu::TMenuController
{
	friend class TLReflection::TDataTreeItem;
public:
	TNodeDataTree(TRefRef Node,TRefRef Graph);
	
	virtual TRefRef			GetSubscriberRef() const			{	static TRef Ref("DataTree");	return Ref;	}
	bool					IsValid() const						{	return (m_pWindow && m_pTree);	}	//	did this init/construct okay and is usable?

	TLGui::TWindow&			GetWindow()							{	return *m_pWindow;	}	//	get window. Assume you've checked IsValid first
	bool					CreateTree(TRefRef Node,TRefRef Graph);		//	re/create the tree

protected:
	/*
	TDataTreeItem*			AddNodeItem(TRefRef NodeRef);						//	Add a node to the tree
	void					RemoveNodeItem(TRefRef NodeRef);					//	remove a node from the tree
	
	void					OnItemAdded(TPtr<TDataTreeItem>& pItem);			//	update the node->item key array
	void					OnItemRemoved(TDataTreeItem& Item);				//	update the node->item key array
	TRef					GetNodeFromItem(TDataTreeItem* pItem);
	TPtr<TDataTreeItem>&	GetItemFromNode(TRefRef Node);
	bool					CreatePopupMenu(TDataTreeItem& Item);	//	popup a menu for this item
	
	virtual TPtr<TLAsset::TMenu>	CreateMenu(TRefRef MenuRef);	//	create a menu. default just loads menu definition from assets, overload to create custom menus
	virtual Bool					ExecuteCommand(TRefRef MenuCommand,TBinaryTree& MenuItemData);	//	execute menu item command - gr: new version, provides the data from the menu item as well to do specific stuff - can be null if we are executing a command without using a menu item
	virtual void					OnMenuOpen();					//	moved onto new menu
	virtual void					OnMenuClose()					{	OnMenuCloseAll();	}
	virtual void					OnMenuCloseAll();				//	closed all menus
	 */
private:
	TPtr<TLGui::TWindow>				m_pWindow;
	TPtr<TLGui::TTree>					m_pTree;			//	todo: don't store this, make the owner control (m_pWindow in this case) be responsible for destruction (as this is what wx does). In theory the smart pointer should still be good but I think as wx explicitly deletes the control this will break the smart pointers
};


