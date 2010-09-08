#include "TMenu.h"




TLAsset::TMenu::TMenu(TRefRef MenuAssetRef) :
	TAsset	( GetAssetType_Static(), MenuAssetRef )
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
		TBinaryTree& ItemData = *Items[i].GetObjectPointer();
		ItemData.ResetReadPos();

		//	read the ref of the new item
		TRef MenuItemRef;
		if ( !ItemData.Read( MenuItemRef ) )
			continue;

		//	create menu item
		TPtr<TMenuItem>& pMenuItem = AddMenuItem( MenuItemRef );
		if ( !pMenuItem )
		{
			TLDebug_Break("Menu item expected");
			Data.Debug_PrintTree();
			continue;
		}

		//	read out other bits of data
		ItemData.ImportData("Command", pMenuItem->m_Command );
		
		ItemData.ImportData("MeshRef", pMenuItem->m_MeshRef );
		ItemData.ImportData("NextMenu", pMenuItem->m_NextMenu );
		ItemData.ImportDataString("String", pMenuItem->m_String );
		ItemData.ImportData("AudioRef", pMenuItem->m_AudioRef );

		TPtr<TBinaryTree>& pItemDataData = ItemData.GetChild("Data");
		if ( pItemDataData )
			pMenuItem->GetData().ReferenceDataTree( *pItemDataData );
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
		TMenuItem& MenuItem = *m_MenuItems[i].GetObjectPointer();
		TPtr<TBinaryTree>& pItemData = Data.AddChild("Item");
		pItemData->Write( MenuItem.GetMenuItemRef() );

		//	export other bits of data that exist
		if ( MenuItem.GetMenuCommand().IsValid() )
			pItemData->ExportData("Command", MenuItem.GetMenuCommand() );

		if ( MenuItem.GetMeshRef().IsValid() )
			pItemData->ExportData("MeshRef", MenuItem.GetMeshRef() );

		if ( MenuItem.GetNextMenu().IsValid() )
			pItemData->ExportData("NextMenu", MenuItem.GetNextMenu() );

		pItemData->ExportDataString("String", MenuItem.GetString() );

		if ( MenuItem.GetAudioRef().IsValid() )
			pItemData->ExportData("AudioRef", MenuItem.GetAudioRef() );

		//	export all other data
		//	gr: ref check just in case it's been changed when it shouldn't have
		if ( MenuItem.GetData().GetDataRef() != STRef4(D,a,t,a) )
		{
			TTempString Debug_String("MenuItem ");
			MenuItem.GetMenuItemRef().GetString( Debug_String );
			Debug_String.Append("'s data has had it's ref changed to ");
			MenuItem.GetData().GetDataRef().GetString( Debug_String );
			TLDebug_Break( Debug_String );
			MenuItem.GetData().SetDataRef( STRef4(D,a,t,a) );
		}

		//	can't reference the original data as there's no TPtr and this is just for
		//	export so the expense (duplication) can be here instead in the menu item (TPtr dereferencing and alloc)
		TPtr<TBinaryTree>& pItemDataData = pItemData->AddChild("data");
		pItemDataData->CopyDataTree( MenuItem.GetData() );
	}

	//	write back any data we didn't recognise
	ExportUnknownData( Data );

	//	gr: grahams test
#ifdef _DEBUG
	//TLDebug_Print("Menu exporting data...");
	//Data.Debug_PrintTree();

	TPtrArray<TBinaryTree> SchemeDatas;
	if ( Data.GetChildren( "SchemeRef", SchemeDatas ) > 1 )
	{
		TLDebug_Break("asset is exporting too much data... or the data supplied already had data in it - export bug tell graham!");
	}
#endif
	return SyncTrue;
}	


//----------------------------------------------
//	add menu item to the menu - returns NULL if duplicated menu item ref. 
//----------------------------------------------
TPtr<TLAsset::TMenu::TMenuItem>& TLAsset::TMenu::AddMenuItem(const TLAsset::TMenu::TMenuItem& MenuItem)
{
	//	alloc the item
	TPtr<TMenuItem>& pItem = AddMenuItem( MenuItem.GetMenuItemRef() );
	if ( !pItem )
		return pItem;
	
	//	copy details
	(*pItem) = MenuItem;
	
	return pItem;
}



//----------------------------------------------
//	create a menu item - returns NULL if duplicated menu item ref
//----------------------------------------------
TPtr<TLAsset::TMenu::TMenuItem>& TLAsset::TMenu::AddMenuItem(TRef MenuItemRef)
{
	//	if invalid ref specified, then come up with an unused one
	if ( !MenuItemRef.IsValid() )
	{
		do
		{
			MenuItemRef.Increment();
		}
		while ( m_MenuItems.Exists( MenuItemRef ) );
	}
	else
	{
		//	check an item doesnt already exist with this ref
		if ( m_MenuItems.Exists( MenuItemRef ) )
			return TLPtr::GetNullPtr<TLAsset::TMenu::TMenuItem>();
	}

	//	create and add new menu item
	TPtr<TLAsset::TMenu::TMenuItem> pNewItem = new TLAsset::TMenu::TMenuItem( MenuItemRef );
	return m_MenuItems.AddPtr( pNewItem );
}



TLAsset::TMenu::TMenuItem::TMenuItem(TRefRef MenuItemRef) :
	m_MenuItemRef	( MenuItemRef ),
	m_Data			( TRef_Static4(D,a,t,a) )
{
	//	default the string to the ref
	m_String << MenuItemRef;
}



TLAsset::TMenu::TMenuItem::TMenuItem(TRefRef MenuItemRef,const TString& String) :
	m_MenuItemRef	( MenuItemRef ),
	m_Data			( TRef_Static4(D,a,t,a) ),
	m_String		( String )
{
}


void TLAsset::TMenu::TMenuItem::SetEnabled(bool Enabled)
{
	//	remove any enabled data
	if ( Enabled )
	{
		GetData().RemoveChild("Enabled");
	}
	else
	{
		GetData().ReplaceData("Enabled", Enabled );
	}
}


bool TLAsset::TMenu::TMenuItem::IsEnabled() const
{
	bool Enabled = true;
	TBinaryTree& Data = const_cast<TBinaryTree&>( GetData() );
	
	Data.ImportData("Enabled", Enabled);
	return Enabled;
}
