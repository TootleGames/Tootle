#include "TMenu.h"




TLAsset::TMenu::TMenu(TRefRef MenuAssetRef) :
	TAsset	( "Menu", MenuAssetRef )
{
}


TLAsset::TMenu::TMenuItem::TMenuItem(TRefRef MenuItemRef) :
	m_MenuItemRef	( MenuItemRef )
{
}


//----------------------------------------------
//	create a menu item - returns NULL if duplicated menu item ref
//----------------------------------------------
TPtr<TLAsset::TMenu::TMenuItem> TLAsset::TMenu::AddMenuItem(TRefRef MenuItemRef)
{
	//	check an item doesnt already exist with this ref
	if ( m_MenuItems.Exists( MenuItemRef ) )
		return NULL;

	TPtr<TLAsset::TMenu::TMenuItem> pNewItem = new TLAsset::TMenu::TMenuItem( MenuItemRef );
	m_MenuItems.Add( pNewItem );

	return pNewItem;
}

