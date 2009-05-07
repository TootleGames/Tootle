#include "TMenu.h"




TLAsset::TMenu::TMenu(TRefRef MenuAssetRef) :
	TAsset	( "Menu", MenuAssetRef )
{
}





//-------------------------------------------------------
//	load asset data out binary data
//-------------------------------------------------------
SyncBool TLAsset::TMenu::ImportData(TBinaryTree& Data)		
{
	Data.ImportData( "SchemeRef", m_SchemeRef );

	//	get all the data for the items
	TPtrArray<TBinaryTree> Items;
	Data.GetChildren("Item", Items );
	for ( u32 i=0;	i<Items.GetSize();	i++ )
	{
		TBinaryTree& ItemData = *Items[i].GetObject();
		ItemData.ResetReadPos();

		//	read the ref of the new item
		TRef MenuItemRef;
		if ( !ItemData.Read( MenuItemRef ) )
			continue;

		//	create menu item
		TPtr<TMenuItem>& pMenuItem = AddMenuItem( MenuItemRef );

		//	read out other bits of data
		ItemData.ImportData("Command", pMenuItem->m_Command );
		ItemData.ImportData("MeshRef", pMenuItem->m_MeshRef );
		ItemData.ImportData("NextMenu", pMenuItem->m_NextMenu );
		ItemData.ImportData("String", pMenuItem->m_Text );
	}

	//	store off any data we haven't read to keep this data intact
	ImportUnknownData( Data );

	return SyncTrue;
}


//-------------------------------------------------------
//	save asset data to binary data
//-------------------------------------------------------
SyncBool TLAsset::TMenu::ExportData(TBinaryTree& Data)				
{
	Data.ExportData( "SchemeRef", m_SchemeRef );

	for ( u32 i=0;	i<m_MenuItems.GetSize();	i++ )
	{
		TMenuItem& MenuItem = *m_MenuItems[i].GetObject();
		TPtr<TBinaryTree>& ItemData = Data.AddChild("Item");
		ItemData->Write( MenuItem.GetMenuItemRef() );

		//	export other bits of data that exist
		if ( MenuItem.GetMenuCommand().IsValid() )
			ItemData->ExportData("Command", MenuItem.GetMenuCommand() );

		if ( MenuItem.GetMeshRef().IsValid() )
			ItemData->ExportData("MeshRef", MenuItem.GetMeshRef() );

		if ( MenuItem.GetNextMenu().IsValid() )
			ItemData->ExportData("NextMenu", MenuItem.GetNextMenu() );

		ItemData->ExportData("String", MenuItem.GetText() );
	}

	//	write back any data we didn't recognise
	ExportUnknownData( Data );

	return SyncTrue;
}	



//----------------------------------------------
//	create a menu item - returns NULL if duplicated menu item ref
//----------------------------------------------
TPtr<TLAsset::TMenu::TMenuItem>& TLAsset::TMenu::AddMenuItem(TRefRef MenuItemRef)
{
	//	check an item doesnt already exist with this ref
	if ( m_MenuItems.Exists( MenuItemRef ) )
		return TLPtr::GetNullPtr<TLAsset::TMenu::TMenuItem>();

	TPtr<TLAsset::TMenu::TMenuItem> pNewItem = new TLAsset::TMenu::TMenuItem( MenuItemRef );
	return m_MenuItems.AddPtr( pNewItem );
}



TLAsset::TMenu::TMenuItem::TMenuItem(TRefRef MenuItemRef) :
	m_MenuItemRef	( MenuItemRef )
{
}

