/*------------------------------------------------------

	Menu controller

-------------------------------------------------------*/
#pragma once

#include <TootleCore/TLTypes.h>
#include <TootleAsset/TMenu.h>


namespace TLMenu
{
	class TMenu;

	typedef TLAsset::TMenu::TMenuItem TMenuItem;
};



//----------------------------------------------
//	a menu rendered to the screen
//----------------------------------------------
class TLMenu::TMenu
{
public:
	TMenu(TPtr<TLAsset::TMenu>& pMenuAsset) :	m_pMenuAsset ( pMenuAsset )				{	}
		
	FORCEINLINE TRefRef						GetMenuRef()								{	return m_pMenuAsset->GetAssetRef();	}
	FORCEINLINE const TLAsset::TMenu&		GetAsset() const							{	return *m_pMenuAsset;	}
	
	FORCEINLINE TPtrArray<TMenuItem>&		GetMenuItems()								{	return m_pMenuAsset->GetMenuItems();	}
	FORCEINLINE const TPtrArray<TMenuItem>&	GetMenuItems() const						{	return m_pMenuAsset->GetMenuItems();	}
	FORCEINLINE TPtr<TMenuItem>&		GetMenuItem(TRefRef MenuItemRef)				{	return GetMenuItems().FindPtr( MenuItemRef );	}
	FORCEINLINE TPtr<TMenuItem>&		GetCurrentMenuItem(TRefRef MenuItemRef)			{	return GetMenuItem( m_HighlightMenuItem );	}
	FORCEINLINE Bool					GetMenuItemExists(TRefRef MenuItemRef) const	{	return GetMenuItems().Exists( MenuItemRef );	}
	FORCEINLINE Bool					GetMenuItemExists(TRefRef MenuItemRef)			{	return GetMenuItems().Exists( MenuItemRef );	}
	FORCEINLINE TRefRef					GetSchemeRef()									{	return m_pMenuAsset->GetSchemeRef();	}

	FORCEINLINE void					SetHighlightedMenuItem(TRefRef MenuItemRef)		{	m_HighlightMenuItem = MenuItemRef;	}


protected:
	TPtr<TLAsset::TMenu>		m_pMenuAsset;			//	menu asset
	TRef						m_HighlightMenuItem;	//	currently highlighted menu item
};



