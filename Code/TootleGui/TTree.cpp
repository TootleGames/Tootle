/*
 *  TTree.cpp
 *  TootleGui
 *
 *  Created by Graham Reeves on 27/01/2010.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#include "TTree.h"



//-----------------------------------------------------------
//	add child item and notify owner
//-----------------------------------------------------------
bool TLGui::TTreeItem::AddChild(TPtr<TTreeItem>& pChildItem)
{
	if ( !pChildItem )
	{
		TLDebug_Break("Child expected");
		return false;
	}
	
	//	unparent child from old parent
	TTreeItem* pOldParent = pChildItem->GetParent();
	if ( pOldParent )
	{
		//	if this is already the parent, nothing to do
		if ( pOldParent == this )
			return true;
		
		pOldParent->RemoveChild( *pChildItem );
	}
	
	//	add to child list
	m_Children.Add( pChildItem );

	//	set new parent
	pChildItem->SetParent( *this );
	
	//	notify tree
	GetOwner().OnItemAdded( pChildItem );
	
	return true;
}


//---------------------------------------------------
//	set new parent (current parent should be null). internal use only: use AddChild to set parent 
//---------------------------------------------------
void TLGui::TTreeItem::SetParent(TTreeItem& NewParent)	
{
	if ( m_pParent )
	{
		TLDebug_Break("Expected no parent");
		return;
	}
	
	m_pParent = &NewParent;	
}


//-----------------------------------------------------------
//	remove child from self and notify owner. Not using a TPtr as we don't need it and we're expecting a definate item
//-----------------------------------------------------------
bool TLGui::TTreeItem::RemoveChild(TTreeItem& ChildItem)
{
	//	un-link from parent
	if ( ChildItem.GetParent() != this )
	{
		if ( !TLDebug_Break("Attempting to remove child from an item that's not its parent") )
			return false;
	}
	ChildItem.SetNoParent();
	
	//	remove from our list of children
	//	note: risk of destruction of child item here if the last TPtr is in the child list, eg. if
	//			caller is using a TPtr& reference to get the item to destruct
	if ( !m_Children.Remove( ChildItem ) )
	{
		if ( !TLDebug_Break("Attempting to remove child from item but not found") )
			return false;
	}

	//	remove from model
	GetOwner().OnItemRemoved( ChildItem );
	return true;
}

TLGui::TTree::TTree(TWindow& Parent,TRefRef ControlRef,TPtr<TTreeItem>& pRootItem) :
	TLGui::TControl			( Parent, ControlRef ),
	m_pRootItem				( pRootItem ),
	m_RootItemAdded			( false )
{
	if ( m_pRootItem )
	{
		m_pRootItem->SetOwner( *this );
	}
	else
	{
		TLDebug_Break( "Root item expected" );
	}
};



