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
//	call when data changes on this item. Invalid ref means all data
//-----------------------------------------------------------
void TLGui::TTreeItem::OnChanged(TRefRef DataRef)
{
	if ( !IsInTree() )
		return;
	
	GetOwner().OnItemChanged( *this, DataRef );
}


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
	
	if ( !IsInTree() )
	{
		TLDebug_Break("Owner expected... this is not in tree?");
		return false;
	}
	
	//	unparent child from old parent
	TTreeItem* pOldParent = pChildItem->GetParent();
	if ( pOldParent )
	{
		//	if this is already the parent, nothing to do
		if ( pOldParent == this )
			return true;
		
		GetOwner().OnItemRemoving( *pChildItem );
		pOldParent->RemoveChild( *pChildItem );
		GetOwner().OnItemRemoved( *pChildItem );
	}
	
	GetOwner().OnItemAdding( pChildItem, this );
	
	//	add to child list
	m_Children.Add( pChildItem );

	//	set new parent
	pChildItem->SetParent( *this );
	
	//	notify tree
	pChildItem->SetOwner( this->GetOwner() );
	GetOwner().OnItemAdded( pChildItem );
	
	//	when we successfully add a child, invoke it's own create-children func to recursively populate the tree
	pChildItem->CreateChildren();
	
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
	
	//	remove from model
	GetOwner().OnItemRemoving( ChildItem );

	//	remove from our list of children
	//	note: risk of destruction of child item here if the last TPtr is in the child list, eg. if
	//			caller is using a TPtr& reference to get the item to destruct
	if ( !m_Children.Remove( ChildItem ) )
	{
		if ( !TLDebug_Break("Attempting to remove child from item but not found") )
			return false;
	}
	ChildItem.SetNoParent();

	//	remove from model
	GetOwner().OnItemRemoved( ChildItem );
	return true;
}

TLGui::TTree::TTree(TRefRef ControlRef,TPtr<TTreeItem>& pRootItem) :
	TLGui::TControl			( ControlRef ),
	m_pRootItem				( pRootItem )
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


bool TLGui::TTree::Initialise(TWindow& Parent)
{
	//	do base gui stuff first
	if ( !TLGui::TControl::Initialise( Parent ) )
		return false;
	
	//	add the root item - as the root item is required, make this a succeed/fail thing
	if ( !OnItemAdding( GetRootItem(), NULL ) )
		return false;
	if ( !OnItemAdded( GetRootItem() ) )
		return false;
	
	//	after adding the root item, initially populate
	GetRootItem()->CreateChildren();
	
	return true;
}
	

//-----------------------------------------------------------
//	verify the tree heirachy starting at the specified item (null starts at root)
//-----------------------------------------------------------
void TLGui::TTree::Debug_VerifyTree(const TLGui::TTreeItem* pItem) const
{
	const TLGui::TTreeItem* pRootItem = GetRootItem();
	
	//	start at root if no item specified
	if ( !pItem )
	{
		pItem = pRootItem;
		
		//	mothing to check if no root item
		if ( !pItem )
			return;
	}
	
	//	ensure this item is somewhere under the root item
	const TLGui::TTreeItem* pItemParent = pItem->GetParent();
	while ( pItemParent )
	{
		//	found the root, all ok
		if ( pItemParent == pRootItem )
			break;
		
		//	move up the tree
		pItemParent = pItemParent->GetParent();
		if ( !pItemParent )
		{
			TLDebug_Break("Gone up tree, and not found the root item. Item is lost.");
			return;
		}
	}
	
	//	loop through every child and make sure it's parent matches item
	const TLGui::TTreeItem& Parent = *pItem;
	const TPtrArray<TLGui::TTreeItem>& Children = Parent.GetChildren();
	for ( u32 c=0;	c<Children.GetSize();	c++ )
	{
		const TLGui::TTreeItem* pChild = Children[c];
		if ( !pChild )
		{
			TLDebug_Break("Null child found");
			continue;
		}
		
		//	check parent
		const TLGui::TTreeItem* pChildParent = pChild->GetParent();
		if ( pChildParent != &Parent )
		{
			TLDebug_Break("Child's parent mis-match");
			continue;
		}
		
		//	recurse into child
		Debug_VerifyTree( pChild );
	}
}
