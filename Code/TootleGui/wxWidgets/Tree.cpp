/*
 *  TTree.cpp
 *  TootleGui
 *
 *  Created by Graham Reeves on 28/01/2010.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */
#if !defined(TL_ENABLE_WX)
#error Should only be built in wx only build
#endif // TL_ENABLE_WX

#include "Tree.h"
#include "Window.h"



TPtr<TLGui::TTree> TLGui::CreateTree(TLGui::TWindow& Parent,TRefRef Ref,TPtr<TLGui::TTreeItem>& pRootItem,const TArray<TRef>& Columns)
{
	TPtr<TLGui::TTree> pControl = new wx::Tree( Parent, Ref, pRootItem, Columns );
	return pControl;
}




wx::Tree::Tree(TLGui::TWindow& Parent,TRefRef ControlRef,TPtr<TLGui::TTreeItem>& pRootItem,const TArray<TRef>& Columns) :
	TLGui::TTree			( Parent, ControlRef, pRootItem ),
	wxDataViewCtrl			( GetWindow(Parent), GetID(ControlRef) ),
	m_DataModel				( Columns, *this )
{
	//	link the data model and control
	this->AssociateModel( &m_DataModel );
	
	//	add columns
	//	todo: define custom column types... we would want to use the node's data type hint...
	//			but we don't have an instance yet...
	for ( u32 c=0;	c<Columns.GetSize();	c++ )
	{
		TTempString ColumnTitle;
		ColumnTitle << Columns[c];
		int Width = -1;
		this->AppendTextColumn( GetString(ColumnTitle), c, wxDATAVIEW_CELL_EDITABLE, Width, wxALIGN_NOT, wxDATAVIEW_COL_SORTABLE|wxDATAVIEW_COL_RESIZABLE );
	}
}



//--------------------------------------------------------
//	Add node to control
//--------------------------------------------------------
Bool wx::Tree::OnItemAdded(TPtr<TLGui::TTreeItem>& pItem)
{
	//	add to the control
	wxDataViewItem ParentItem = wx::GetwxItem( pItem->GetParent() );
	wxDataViewItem NodeItem = wx::GetwxItem( pItem );
	if ( !m_DataModel.ItemAdded( ParentItem, NodeItem ) )
	{
		TLDebug_Break("Failed to add item to tree... Parent not added?");
		return false;
	}
	
	return true;
}


//-----------------------------------------------------------
//	move node around in control
//-----------------------------------------------------------
Bool wx::Tree::OnItemMoved(TPtr<TLGui::TTreeItem>& pOldParent,TPtr<TLGui::TTreeItem>& pItem)
{
	//	remove from old parent
	wxDataViewItem Item = GetwxItem( pItem );
	wxDataViewItem OldParentItem = GetwxItem( pOldParent );
	if ( !m_DataModel.ItemDeleted( OldParentItem, Item ) )
	{
		TLDebug_Print("Failed to remove item from old parnet in tree");
	}
	
	//	add to new parent
	wxDataViewItem NewParentItem = GetwxItem( pItem->GetParent() );
	if ( !m_DataModel.ItemAdded( NewParentItem, Item ) )
	{
		TLDebug_Print("Failed to add item to new parent in tree");
		return false;
	}
	
	return true;
}


//---------------------------------------------------
//	remove node from control
//---------------------------------------------------
Bool wx::Tree::OnItemRemoved(TLGui::TTreeItem& Item)
{
	//	remove item from control
	wxDataViewItem TreeItem = GetwxItem( &Item );
	wxDataViewItem ParentTreeItem = GetwxItem( Item.GetParent() );
	if ( !m_DataModel.ItemDeleted( TreeItem, ParentTreeItem ) )
	{
		TLDebug_Print("Failed to remove item from tree");
		return false;
	}
	
	return true;
}



wx::TreeDataModel::TreeDataModel(const TArray<TRef>& Columns,wx::Tree& Owner) :
	m_pOwner	( &Owner ),
	m_Columns	( Columns )
{
	if ( !m_pOwner )
	{
		TLDebug_Break("Owner expected - crash will occur!");
	}
}


//------------------------------------------------------------------
//	return binary tree entry for this item at this column. returns NULL on error or if the data doesn't exist
//------------------------------------------------------------------
TBinary* wx::TreeDataModel::GetData(const wxDataViewItem& Item,u32 Column)
{
	//	invalid column
	if ( Column >= m_Columns.GetSize() )
	{
		TLDebug_Break("Column out of range");
		return NULL;
	}
	
	//	get the item
	TLGui::TTreeItem* pTreeItem = GetTreeItem( Item );
	if ( !pTreeItem )
	{
		TLDebug_Break("Item expected");
		return NULL;
	}
	
	//	fetch the data
	TRefRef ItemDataRef = m_Columns[Column];
	TBinary* pData = pTreeItem->GetData( ItemDataRef );
	
	return pData;
}

//------------------------------------------------------------------
//	return binary tree entry for this item at this column. returns NULL on error or if the data doesn't exist
//------------------------------------------------------------------
const TBinary* wx::TreeDataModel::GetData(const wxDataViewItem& Item,u32 Column) const
{
	//	bit of a hack for now, but reduces code until it's all finalised
	wx::TreeDataModel* pThis = const_cast<wx::TreeDataModel*>( this );
	return pThis->GetData( Item, Column );
}


void wx::TreeDataModel::GetValue( wxVariant &variant,const wxDataViewItem &item, unsigned int col ) const
{
	const TBinary* pData = GetData( item, col );
	if ( !pData )
	{
		TString Debug_String;
		Debug_String << m_Columns[col] << " not found";
		variant = GetString( Debug_String );
		return;
	}
	
	//	convert the data into a variant to return
	wx::GetVariant( variant, *pData );
}


bool wx::TreeDataModel::SetValue(const wxVariant &variant,const wxDataViewItem &item,unsigned int col)
{
	//	get data
	TBinary* pData = GetData( item, col );
	
	//	return no changes
	if ( !pData )
		return false;
	
	//	set data based on the variant
	if ( !wx::SetData( *pData, variant ) )
		return false;
	
	return true;
}


wxDataViewItem wx::TreeDataModel::GetParent( const wxDataViewItem &item ) const
{
	const TLGui::TTreeItem* pTreeItem = GetTreeItem( item );
	
	//	root has no parent
	if ( !pTreeItem )
		return wxDataViewItem();
	
	//	turn the node's parent into a dataviewitem
	const TLGui::TTreeItem* pParentTreeItem = pTreeItem->GetParent();
	wxDataViewItem ParentItem = GetwxItem( pParentTreeItem );
	return ParentItem;
}

//----------------------------------------------------------
//	as per TT, to enable the more flexible tree generation, make everything a container
//	and then maybe work around the rendering later
//----------------------------------------------------------
bool wx::TreeDataModel::IsContainer( const wxDataViewItem &item ) const
{
	/*
	TLGui::TTreeItem* pTreeItem = GetTreeItem( item );
	
	//	root is always a container
	if ( !pTreeItem )
		return true;

	return pTreeItem->HasChildren();
	*/
	return true;
}

unsigned int wx::TreeDataModel::GetChildren( const wxDataViewItem &item, wxDataViewItemArray &children ) const
{
	const TLGui::TTreeItem* pTreeItem = GetTreeItem( item );
	
	//	root item has just the root node for a child
	if ( !pTreeItem )
	{
		//	get the root item and add to the list
		const TLGui::TTreeItem* pRootItem = GetOwner().GetRootItem();
		wxDataViewItem ChildItem = GetwxItem( pRootItem );
		children.push_back( ChildItem );
		return children.size();
	}

	//	enumerate children
	const TPtrArray<TLGui::TTreeItem>& Children = pTreeItem->GetChildren();
	for ( u32 c=0;	c<Children.GetSize();	c++ )
	{
		const TLGui::TTreeItem* pChild = Children[c];
		wxDataViewItem ChildItem = GetwxItem( pChild );
		children.push_back( ChildItem );
	}
	
	return children.size();
}

