/*
 *  TTree.h
 *  TootleGui
 *
 *  Created by Graham Reeves on 27/01/2010.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */
#pragma once
#include "TControl.h"
#include <TootleCore/TBinaryTree.h>
#include <TootleCore/TLGraph.h>
#include <TootleAsset/TMenu.h>



namespace TLGui
{
	class TTree;
	class TTreeItem;
};

//-----------------------------------------------
//	overload this for your own virtual item functionality
//-----------------------------------------------
class TLGui::TTreeItem
{
public:
	TTreeItem() :
		m_pParent	( NULL ),
		m_pOwner	( NULL )
	{
	}
		
	virtual ~TTreeItem()
	{
	}
	
	void						SetOwner(TTree& Owner)		{	m_pOwner = &Owner;	}
	TTree&						GetOwner()					{	return *m_pOwner;	}
	bool						IsInTree() const			{	return m_pOwner != NULL;	}	//	owner is assigned first time the item is added to the tree
	TTreeItem*					GetParent()					{	return m_pParent;	}
	const TTreeItem*			GetParent() const			{	return m_pParent;	}
	void						SetNoParent()				{	m_pParent = NULL;	}	//	explicit "set parent to null"
	bool						IsRootItem() const			{	return !GetParent() && IsInTree();	}
	bool						HasChildren() const			{	return m_Children.GetSize() > 0;	}
	TPtrArray<TTreeItem>&		GetChildren()				{	return m_Children;	}
	const TPtrArray<TTreeItem>&	GetChildren() const			{	return m_Children;	}
	virtual bool				AddChild(TPtr<TTreeItem>& pChildItem);		//	add child item and notify owner
	virtual bool				RemoveChild(TTreeItem& ChildItem);			//	remove child from self and notify owner. Not using a TPtr as we don't need it and we're expecting a definate item
	
	virtual void				GetData(TString& DataString,TRefRef DataRef)	{};		//	get string to put in columns when TBinary data isn't availible
	virtual TBinary*			GetData(TRefRef DataRef)						{	return NULL;	}	//	get data to read/write from
	virtual bool				IsDataMutable(TRefRef DataRef)					{	TBinary* pData = GetData( DataRef );	return (pData!=NULL);	}
	virtual bool				SetData(const TString& DataString,TRefRef DataRef)	{	return false;	};		//	new data to put into this column
	virtual bool				SetData(const TBinary& NewData,TRefRef DataRef)	{	return false;	};			//	new data to put into this column
	virtual void				CreateChildren()=0;								//	create child items
	
	virtual void				OnChanged(TRefRef DataRef=TRef_Invalid);		//	call when data changes on this item. Invalid ref means all data
	virtual void				OnSelectedChanged(bool IsSelected)				{	}	//	selection state has changed
	virtual bool				OnRightClick()			{	return false;	}	//	popup a menu for this item on right-click

	FORCEINLINE Bool			operator==(const TTreeItem& Item) const		{	return (this == &Item);	}	//	not very sophisticated, but fine! :)

protected:
	void						SetParent(TTreeItem& NewParent);		//	set new parent (current parent should be null). internal use only: use AddChild to set parent
	
private:
	TTree*					m_pOwner;
	TTreeItem*				m_pParent;		//	note: NOT a smart pointer!
	TPtrArray<TTreeItem>	m_Children;		//	smart pointer children so automatically free'd
};




//--------------------------------------------------------
//	generic interface to a tree (with multiple columns) control
//--------------------------------------------------------
class TLGui::TTree : public TLGui::TControl
{
	friend class TTreeItem;		//	tree item has access to notification callbacks
public:
	TTree(TRefRef ControlRef,TPtr<TTreeItem>& pRootItem);
	
	virtual bool			Initialise(TWindow& Parent);
	
	TPtr<TTreeItem>&		GetRootItem()				{	return m_pRootItem;	}
	const TPtr<TTreeItem>&	GetRootItem() const			{	return m_pRootItem;	}

protected:	//	os implementation
	virtual bool			OnItemAdding(TPtr<TTreeItem>& pItem,TLGui::TTreeItem* pParent)=0;
	virtual bool			OnItemAdded(TPtr<TTreeItem>& pItem)=0;
	virtual bool			OnItemRemoving(TTreeItem& Item)=0;
	virtual bool			OnItemRemoved(TTreeItem& Item)=0;
	virtual bool			OnItemMoving(TPtr<TTreeItem>& pOldParent,TPtr<TTreeItem>& pItem)=0;
	virtual bool			OnItemMoved(TPtr<TTreeItem>& pOldParent,TPtr<TTreeItem>& pItem)=0;
	virtual bool			OnItemChanged(TTreeItem& Item,TRefRef DataChanged)=0;

	void					Debug_VerifyTree(const TLGui::TTreeItem* pItem=NULL) const;	//	verify the tree heirachy starting at the specified item (null starts at root)
	
private:
	TPtr<TTreeItem>			m_pRootItem;
};

