/*
 *  TTree.h
 *  TootleGui
 *
 *  Created by Graham Reeves on 28/01/2010.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */
#if !defined(TL_ENABLE_QT)
#error Should only be built in QT only build
#endif // TL_ENABLE_QT

#pragma once
#include "../TTree.h"
#include "Window.h"
#include <QTCore/QAbstractItemModel>
#include <QTGui/QTreeView>

namespace Qt
{
	class Tree;
	class TreeDataModel;	//	data model for the tree
	
}


//----------------------------------------------------------
//	overloaded data model class - do as little work as possible in this class and 
//	put the majority of the functionality in wx::Tree, just so it keeps most of the code in one place
//	don't bother making anything in this class protected, it should only be used by the wx::Tree anyway
//----------------------------------------------------------
class Qt::TreeDataModel : public QAbstractItemModel
{
	Q_OBJECT
public:
	TreeDataModel(const TArray<TRef>& Columns,Qt::Tree& Owner);
	
	Qt::Tree&				GetOwner()				{	return *m_pOwner;	}
	const Qt::Tree&			GetOwner() const		{	return *m_pOwner;	}
	
	const TArray<TRef>&		GetColumns() const		{	return m_Columns;	}
	//	qt model virtuals
	virtual QModelIndex		index(int row, int column,const QModelIndex &parent) const;
    virtual QModelIndex		parent(const QModelIndex &child) const;
    virtual int				rowCount(const QModelIndex &parent) const;
    virtual int				columnCount(const QModelIndex &parent) const;	
    virtual QVariant		data(const QModelIndex &index, int role) const;
	virtual QVariant		headerData(int section, Qt::Orientation orientation,int role) const;
    virtual Qt::ItemFlags	flags(const QModelIndex &index) const;
    virtual bool			setData(const QModelIndex &index, const QVariant &value, int role);

	bool					OnItemAdding(TLGui::TTreeItem& Item,TLGui::TTreeItem* pParent);		//	added item
	bool					OnItemAdded(TLGui::TTreeItem& Item);		//	added item
	bool					OnItemRemoving(TLGui::TTreeItem& Item);
	bool					OnItemRemoved(TLGui::TTreeItem& Item);
	bool					OnItemMoving(TLGui::TTreeItem& OldParent,TLGui::TTreeItem& Item);
	bool					OnItemMoved(TLGui::TTreeItem& OldParent,TLGui::TTreeItem& Item);
	bool					OnItemChanged(TLGui::TTreeItem& Item,TRefRef Data);

	TPtr<TLGui::TTreeItem>&			GetRootItem();
	const TPtr<TLGui::TTreeItem>&	GetRootItem() const;

public:
	TLGui::TTreeItem*			GetTreeItem(const QModelIndex& ModelIndex);
	const TLGui::TTreeItem*		GetTreeItem(const QModelIndex& ModelIndex) const;
	QModelIndex					GetModelIndex(const TLGui::TTreeItem& TreeItem,int Column=0);
	
protected:
	THeapArray<TRef>		m_Columns;			//	data-column -> binarydata-on-item lookup. column[N] = TreeItem.GetData(Ref)
	bool					m_RootItemAdded;	//	we have to pretend the root item doesn't exist until it's added
private:
	Qt::Tree*				m_pOwner;		//	store owner for node access - never null
};


//-----------------------------------------------------------
//	qt tree control handling, which is mostly a wrapper to the model
//-----------------------------------------------------------
class Qt::Tree : public QTreeView, public TLGui::TTree, public Qt::TWidgetWrapper, public Qt::TMenuHandler, public TLMessaging::TPublisher
{
	Q_OBJECT
	friend class Qt::TreeDataModel;
public:
	Tree(TRefRef ControlRef,TPtr<TLGui::TTreeItem>& pRootItem,const TArray<TRef>& Columns);
	
	virtual bool		Initialise(TLGui::TWindow& Parent);				//	initialise - this is seperated from the constructor so we can use virtual functions

	virtual bool		OnItemAdding(TPtr<TLGui::TTreeItem>& pItem,TLGui::TTreeItem* pParent);
	virtual bool		OnItemAdded(TPtr<TLGui::TTreeItem>& pItem);
	virtual bool		OnItemRemoving(TLGui::TTreeItem& Item);
	virtual bool		OnItemRemoved(TLGui::TTreeItem& Item);
	virtual bool		OnItemMoving(TPtr<TLGui::TTreeItem>& pOldParent,TPtr<TLGui::TTreeItem>& pItem);
	virtual bool		OnItemMoved(TPtr<TLGui::TTreeItem>& pOldParent,TPtr<TLGui::TTreeItem>& pItem);
	virtual bool		OnItemChanged(TLGui::TTreeItem& Item,TRefRef Data);

protected:
	virtual QWidget&					GetWidget()						{	return *this;	}
	virtual TLMessaging::TPublisher&	GetPublisher()					{	return *this;	}
	virtual void						BindAction(QAction& Action);
	virtual QMenu*						AllocMenu(const TLAsset::TMenu& Menu);	
	virtual TMenuHandler*				GetMenuHandler()				{	return this;	}
	
	virtual TRefRef						GetPublisherRef() const			{	return GetRef();	}
	
protected slots:
    virtual void		selectionChanged(const QItemSelection &selected, const QItemSelection &deselected);
	void				OnItemClicked(const QModelIndex& index);
	void				OnAction()				{	TMenuHandler::Slot_OnAction(static_cast<QAction*>( sender() ) );	}

private:
	TreeDataModel		m_DataModel;		//	data model for the view control
};





FORCEINLINE TPtr<TLGui::TTreeItem>& Qt::TreeDataModel::GetRootItem()
{
	return GetOwner().GetRootItem();
}

FORCEINLINE const TPtr<TLGui::TTreeItem>& Qt::TreeDataModel::GetRootItem() const							
{
	return GetOwner().GetRootItem();	
}

