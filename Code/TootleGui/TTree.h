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
	//	root item constructor
	explicit TTreeItem() :
		m_pParent	( NULL ),
		m_pOwner	( NULL )
	{
	}
		
	//	child item constructor
	TTreeItem(TTreeItem& Parent) :
		m_pParent	( &Parent ),
		m_pOwner	( &Parent.GetOwner() )
	{
	}
	virtual ~TTreeItem()
	{
	}
	
	void						SetOwner(TTree& Owner)		{	m_pOwner = &Owner;	}	//	only for root item really
	TTree&						GetOwner()					{	return *m_pOwner;	}
	TTreeItem*					GetParent()					{	return m_pParent;	}
	const TTreeItem*			GetParent() const			{	return m_pParent;	}
	void						SetNoParent()				{	m_pParent = NULL;	}	//	explicit "set parent to null"
	bool						HasChildren() const			{	return m_Children.GetSize() > 0;	}
	TPtrArray<TTreeItem>&		GetChildren()				{	return m_Children;	}
	const TPtrArray<TTreeItem>&	GetChildren() const			{	return m_Children;	}
	virtual bool				AddChild(TPtr<TTreeItem>& pChildItem);		//	add child item and notify owner
	virtual bool				RemoveChild(TTreeItem& ChildItem);			//	remove child from self and notify owner. Not using a TPtr as we don't need it and we're expecting a definate item
	
	virtual TBinary*			GetData(TRefRef DataRef)=0;					//	get data to read/write from
	virtual void				CreateChildren()=0;							//	create child items
	
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
	TTree(TWindow& Parent,TRefRef ControlRef,TPtr<TTreeItem>& pRootItem);
	
	TPtr<TTreeItem>&		GetRootItem()				{	return m_pRootItem;	}
	const TPtr<TTreeItem>&	GetRootItem() const			{	return m_pRootItem;	}

protected:	//	os implementation
	virtual bool			OnItemAdded(TPtr<TTreeItem>& pItem)=0;
	virtual bool			OnItemRemoved(TTreeItem& Item)=0;
	virtual bool			OnItemMoved(TPtr<TTreeItem>& pOldParent,TPtr<TTreeItem>& pItem)=0;
	
private:
	TPtr<TTreeItem>			m_pRootItem;
	bool					m_RootItemAdded;			//	to avoid the neccessity of a seperate Initialise() 
														//	function (we want to add the root item to the control in 
														//	the constructor really) we store this and add it automatically first time we can
};

