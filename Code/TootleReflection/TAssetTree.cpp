#include "TAssetTree.h"
#include "TLReflection.h"



//-------------------------------------------------------------------
//	root constructor
//-------------------------------------------------------------------
TLReflection::TAssetTreeItem::TAssetTreeItem()
{
}


//-------------------------------------------------------------------
//	child constructor
//-------------------------------------------------------------------
TLReflection::TAssetTreeItem::TAssetTreeItem(TTypedRef Asset) :
	m_Asset		( Asset )
{
}



//-------------------------------------------------------------
//	add child item and notify owner
//-------------------------------------------------------------
bool TLReflection::TAssetTreeItem::AddChild(TPtr<TLGui::TTreeItem>& pChildItem)
{
	//	do super stuff
	return TLGui::TTreeItem::AddChild( pChildItem );
}


//---------------------------------------------------------------
//	
//---------------------------------------------------------------
bool TLReflection::TAssetTreeItem::IsDataMutable(TRefRef DataRef)
{
	return false;
}


//---------------------------------------------------------------
//	
//---------------------------------------------------------------
bool TLReflection::TAssetTreeItem::SetData(const TString& DataString,TRefRef DataRef)
{
	return false;
}



//---------------------------------------------------------------
//	get data to read/write from
//---------------------------------------------------------------
void TLReflection::TAssetTreeItem::GetData(TString& DataString,TRefRef DataRef)
{
	switch( DataRef.GetData() )
	{
		case TRef_Static3(R,e,f):
			DataString << m_Asset.GetRef();
			break;
			
		case TRef_Static4(T,y,p,e):
			DataString << m_Asset.GetTypeRef();
			break;
	}
}

//---------------------------------------------------------------
//	create child items
//---------------------------------------------------------------
void TLReflection::TAssetTreeItem::CreateChildren()
{
	//	root item iterates all the assets, actual asset items don't have any children
	if ( !IsRootItem() )
		return;

	//	list assets
	TLAsset::TAssetManager& AssetManager = *TLAsset::g_pManager;
	TPtrArray<TLAsset::TAsset,100>& Assets = AssetManager.GetAllAssets();
	for ( u32 a=0;	a<Assets.GetSize();	a++ )
	{
		TLAsset::TAsset& Asset = *Assets[a];
		TTypedRef AssetRef = Asset.GetAssetAndTypeRef();
		TPtr<TTreeItem> pChildItem = new TAssetTreeItem( AssetRef );
		this->AddChild( pChildItem );
	}
}


//-------------------------------------------------------------
//	catch changes to the node we're associated with
//-------------------------------------------------------------
void TLReflection::TAssetTreeItem::ProcessMessage(TLMessaging::TMessage& Message)
{
}

	
//--------------------------------------------------------------
//	setup window & controls associated with this Asset
//--------------------------------------------------------------
TLReflection::TAssetTree::TAssetTree()
{
	//	create window
	TRef WindowRef("Asset");
	m_pWindow = TLGui::CreateGuiWindow( WindowRef );
	if ( !m_pWindow )
	{
		TLDebug_Print("Failed to create window for TLReflection::TAssetTree");
		return;
	}

	//	setup column structure for the tree
	TFixedArray<TRef,100> Columns;

	Columns.Add("Ref");		//	asset's ref
	Columns.Add("Type");	//	asset's type

	//	get a root item for the tree
	TPtr<TAssetTreeItem> pRootAssetItem = new TAssetTreeItem();
	TPtr<TLGui::TTreeItem> pRootItem = pRootAssetItem;
	
	//	setup a data view control on the window starting at the root node
	TRef ControlRef = WindowRef;
	m_pTree = TLGui::CreateTree( *m_pWindow, ControlRef, pRootItem, Columns );
	if ( !m_pTree )
	{
		TLDebug_Break("Failed to create tree control");
		return;
	}

	//	show the window
	if ( m_pWindow )
	{
		Type2<u16> Size( 200, 400 );
		m_pWindow->SetSize( Size );
		m_pWindow->Show();
	}
}


//--------------------------------------------------------------
//
//--------------------------------------------------------------
void TLReflection::TAssetTree::ProcessMessage(TLMessaging::TMessage& Message)
{
}

