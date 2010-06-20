/*
 *  TTree.h
 *  TootleGui
 *
 *  Created by Graham Reeves on 28/01/2010.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */
#if !defined(TL_ENABLE_WX)
#error Should only be built in wx only build
#endif // TL_ENABLE_WX



#pragma once
#include "../TTree.h"

#include "TLWx.h"
#include "wx/dataview.h"


#if !wxUSE_DATAVIEWCTRL
#error "DataView control required: set wxUSE_DATAVIEWCTRL to 1 and rebuild the library"
#endif

namespace wx
{
	class Tree;
	class TreeDataModel;

	static wxDataViewItem			GetwxItem(const TLGui::TTreeItem* pItem)	{	return wxDataViewItem( const_cast<void*>( reinterpret_cast<const void*>( pItem ) ) );	}
	static TLGui::TTreeItem*		GetTreeItem(const wxDataViewItem& wxItem)	{	return reinterpret_cast<TLGui::TTreeItem*>( wxItem.GetID() );	}	
}


//----------------------------------------------------------
//	overloaded data model class - do as little work as possible in this class and 
//	put the majority of the functionality in wx::Tree, just so it keeps most of the code in one place
//	don't bother making anything in this class protected, it should only be used by the wx::Tree anyway
//----------------------------------------------------------
class wx::TreeDataModel : public wxDataViewModel
{
public:
	TreeDataModel(const TArray<TRef>& Columns,wx::Tree& Owner);

	wx::Tree&				GetOwner()													{	return *m_pOwner;	}
	const wx::Tree&			GetOwner() const											{	return *m_pOwner;	}
	virtual unsigned int	GetColumnCount() const										{	return m_Columns.GetSize();	}
	virtual wxString		GetColumnType( unsigned int col ) const						{	return wxString();	}	//	gr: in the implementation at TT this had no effect and was never called...
	virtual void			GetValue( wxVariant &variant,const wxDataViewItem &item, unsigned int col ) const;
	virtual bool			SetValue(const wxVariant &variant,const wxDataViewItem &item,unsigned int col);

	virtual wxDataViewItem	GetParent( const wxDataViewItem &item ) const;
	virtual bool			IsContainer( const wxDataViewItem &item ) const;
	virtual unsigned int	GetChildren( const wxDataViewItem &item, wxDataViewItemArray &children ) const;
	
	TBinary*				GetData(const wxDataViewItem& Item,u32 Column);				//	return binary tree entry for this item at this column. returns NULL on error or if the data doesn't exist
	const TBinary*			GetData(const wxDataViewItem& Item,u32 Column) const;		//	return binary tree entry for this item at this column. returns NULL on error or if the data doesn't exist
	
	TPtr<TLGui::TTreeItem>&			GetRootTreeItem();
	const TPtr<TLGui::TTreeItem>&	GetRootTreeItem() const;
	
protected:
	THeapArray<TRef>		m_Columns;		//	data-column -> binarydata-on-item lookup. column[N] = TreeItem.GetData(Ref)
	
private:
	wx::Tree*				m_pOwner;		//	store owner for node access - never null
};


//-----------------------------------------------------------
//	wx implementation of the tree control. It's a wxDataViewCtrl and
//	contains the data model for it, which we have no purpose giving access
//	to outside the class.
//-----------------------------------------------------------
class wx::Tree : public TLGui::TTree, public wxDataViewCtrl
{
	friend class wx::TreeDataModel;
public:
	Tree(TLGui::TWindow& Parent,TRefRef ControlRef,TPtr<TLGui::TTreeItem>& pRootItem,const TArray<TRef>& Columns);

	virtual bool		OnItemAdded(TPtr<TLGui::TTreeItem>& pItem);
	virtual bool		OnItemRemoved(TLGui::TTreeItem& Item);
	virtual bool		OnItemMoved(TPtr<TLGui::TTreeItem>& pOldParent,TPtr<TLGui::TTreeItem>& pItem);
	
private:
	TreeDataModel	m_DataModel;		//	data model for the view control
};



FORCEINLINE TPtr<TLGui::TTreeItem>& wx::TreeDataModel::GetRootTreeItem()
{
	return GetOwner().GetRootItem();
}

FORCEINLINE const TPtr<TLGui::TTreeItem>& wx::TreeDataModel::GetRootTreeItem() const							
{
	return GetOwner().GetRootItem();	
}


