/*
	Tree which shows our loaded assets
*/
#pragma once

#include <TootleGui/TWindow.h>
#include <TootleGui/TMenu.h>
#include <TootleGui/TTree.h>
#include <TootleCore/TGraphBase.h>
#include <TootleCore/TLMessaging.h>
#include <TootleGame/TMenuController.h>	//	gr: including game lib file :( see about moving the menu handling system to the core?


namespace TLReflection
{
	class TAssetTreeItem;	//	item for a TLGui::Tree which maps items to assets and vice versa
	class TAssetTree;	//	window which contains a tree which shows all our assets
}


//---------------------------------------------------------
//
//---------------------------------------------------------
class TLReflection::TAssetTreeItem : public TLGui::TTreeItem, public TLMessaging::TSubscriber
{
public:
	TAssetTreeItem();					//	root constructor
	TAssetTreeItem(TTypedRef AssetRef);	//	child constructor

	virtual TRefRef				GetSubscriberRef() const			{	return m_Asset.GetRef();	}
	const TTypedRef&			GetAssetRef() const 				{	return m_Asset;	}
	
	//	gr: these are overloaded so we can update the node->item index
	virtual bool				AddChild(TPtr<TTreeItem>& pChildItem);		//	add child item and notify owner
	
	
protected:
	virtual void				ProcessMessage(TLMessaging::TMessage& Message);	//	catch changes to the node we're associated with

	virtual void				GetData(TString& DataString,TRefRef DataRef);
	virtual bool				IsDataMutable(TRefRef DataRef);
	virtual bool				SetData(const TString& DataString,TRefRef DataRef);
	virtual void				CreateChildren();			//	create child items
	
protected:
	TTypedRef					m_Asset;
};




//--------------------------------------------------------------
//	window which contains a tree displaying the contents of the assets
//--------------------------------------------------------------
class TLReflection::TAssetTree : public TLMenu::TMenuController
{
	friend class TLReflection::TAssetTreeItem;
public:
	TAssetTree();

	virtual TRefRef			GetSubscriberRef() const			{	static TRef Ref("AssetTree");	return Ref;	}
	bool					IsValid() const						{	return (m_pWindow && m_pTree);	}	//	did this init/construct okay and is usable?

protected:
	virtual void					ProcessMessage(TLMessaging::TMessage& Message);

	TPtr<TLAsset::TMenu>			CreateNodePopupMenu(TRefRef MenuRef,const TArray<TRef>& Nodes);	//	create menu for the node-popup for these nodes
	
protected:
	TPtr<TLGui::TWindow>				m_pWindow;
	TPtr<TLGui::TTree>					m_pTree;		//	todo: don't store this, make the owner control (m_pWindow in this case) be responsible for destruction (as this is what wx does). In theory the smart pointer should still be good but I think as wx explicitly deletes the control this will break the smart pointers
	TPtr<TLGui::TMenuWrapper>			m_pMenuWrapper;		//	menu wrapper
	THeapArray<TRef>					m_PopupMenuNodes;	//	our hacky method of customising the node popup menu...
};




