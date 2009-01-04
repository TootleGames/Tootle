/*------------------------------------------------------

	Menu controller

-------------------------------------------------------*/
#pragma once

#include <TootleCore/TLTypes.h>
#include <TootleAsset/TMenu.h>
#include <TootleCore/TPublisher.h>
#include <TootleCore/TSubscriber.h>


namespace TLMenu
{
	class TMenuController;
	class TMenu;

	typedef TLAsset::TMenu::TMenuItem TMenuItem;
};



//----------------------------------------------
//	a menu rendered to the screen
//----------------------------------------------
class TLMenu::TMenu
{
public:
	TMenu(TPtr<TLAsset::TMenu> pMenuAsset) :	m_pMenuAsset ( pMenuAsset )				{	}
		
	FORCEINLINE TRefRef						GetMenuRef()								{	return m_pMenuAsset->GetAssetRef();	}

	FORCEINLINE TPtrArray<TMenuItem>&		GetMenuItems()								{	return m_pMenuAsset->GetMenuItems();	}
	FORCEINLINE const TPtrArray<TMenuItem>&	GetMenuItems() const						{	return m_pMenuAsset->GetMenuItems();	}
	FORCEINLINE TPtr<TMenuItem>&		GetMenuItem(TRefRef MenuItemRef)				{	return GetMenuItems().FindPtr( MenuItemRef );	}
	FORCEINLINE TPtr<TMenuItem>&		GetCurrentMenuItem(TRefRef MenuItemRef)			{	return GetMenuItem( m_HighlightMenuItem );	}
	FORCEINLINE Bool					GetMenuItemExists(TRefRef MenuItemRef) const	{	return GetMenuItems().Exists( MenuItemRef );	}
	FORCEINLINE Bool					GetMenuItemExists(TRefRef MenuItemRef)			{	return GetMenuItems().Exists( MenuItemRef );	}

	FORCEINLINE void					SetHighlightedMenuItem(TRefRef MenuItemRef)		{	m_HighlightMenuItem = MenuItemRef;	}
	
protected:
	TPtr<TLAsset::TMenu>		m_pMenuAsset;			//	menu asset
	TRef						m_HighlightMenuItem;	//	currently highlighted menu item
};



//----------------------------------------------
//	controls menu flow
//----------------------------------------------
class TLMenu::TMenuController : public TLMessaging::TPublisher, public TLMessaging::TSubscriber
{
public:
	TMenuController()	{};
	
	//	external commands
	TPtr<TMenu>&		GetCurrentMenu()						{	return m_MenuStack.GetPtrLast();	}	//	check IsMenuOpen() before accessing first
	Bool				IsMenuOpen() const						{	return m_MenuStack.GetSize() > 0;	}
	Bool				OpenMenu(TRefRef MenuRef);				//	move onto new menu with this ref - returns false if no such menu
	void				CloseMenu();							//	close this menu and go back to previous
	void				CloseAllMenus();						//	clear menu stack
	Bool				HighlightMenuItem(TRef MenuItemRef);	//	highlight a menu item
	Bool				ExecuteMenuItem(TRefRef MenuItemRef);	//	execute menu item command
	Bool				GetMenuItemExists(TRefRef MenuItem) const	{	return IsMenuOpen() ? m_MenuStack.ElementLastConst()->GetMenuItemExists(MenuItem) : FALSE;	}

protected:
	virtual TPtr<TMenu>		CreateMenu(TRefRef MenuRef);			//	create a menu. default just loads menu definition from assets, overload to create custom menus
	virtual Bool			ExecuteCommand(TRefRef MenuCommand)	{	return FALSE;	}	//	execute menu item command 

	TPtr<TMenuItem>		GetMenuItem(TRefRef MenuItemRef);	//	get menu item out of current menu

	//	incoming events
	virtual void		ProcessMessage(TPtr<TLMessaging::TMessage>& pMessage);

	//	outgoing events
	void				OnMenuOpen();						//	moved onto new menu
	void				OnMenuClose();						//	moved to previous menu
	void				OnMenuCloseAll();					//	closed all menus
	void				OnMenuItemHighlighted();			//	highlighted menu item
	void				OnMenuItemExecuted(TRefRef MenuCommand);	//	menu item executed

protected:
	TPtrArray<TMenu>	m_MenuStack;						//	menu stack
};

