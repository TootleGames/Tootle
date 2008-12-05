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
	class TMenuItem;
	class TMenu;
};



//----------------------------------------------
//	individual menu item
//----------------------------------------------
class TLMenu::TMenuItem
{
public:
	TMenuItem(TRefRef MenuItemRef);

	TRefRef						GetMenuItemRef() const	{	return m_MenuItemRef;	}
	TString&					GetString()				{	return m_String;	}
	TRefRef						GetMenuCommand()		{	return m_Command;	}
	TRefRef						GetNextMenu()			{	return m_NextMenu;	}
	
	TRefRef						GetMeshRef()			{ return m_refMeshID; }

	void						SetString(const TString& String)	{	m_String = String;	}
	void						SetMenuCommand(TRefRef Command)		{	m_Command = Command;	}
	void						SetNextMenu(TRefRef NextMenu)		{	m_NextMenu = NextMenu;	SetMenuCommand("open");	};
	void						SetMeshRef(TRefRef MeshID)			{	m_refMeshID = MeshID;	}

	Bool						IsHighlightable() const	{	return m_Command.IsValid();	}	

	inline Bool					operator==(TRefRef MenuItemRef) const	{	return GetMenuItemRef() == MenuItemRef;	}
//	inline Bool					operator<(TRefRef MenuItemRef) const	{	return GetMenuItemRef() < MenuItemRef;	}
//	inline Bool					operator<(const TMenuItem& MenuItem) const	{	return GetMenuItemRef() < MenuItem.GetMenuItemRef();	}

protected:
	TRef						m_MenuItemRef;		//	ref for menu item
	TString						m_String;			//	string displayed on menu
	TRef						m_Command;			//	menu command, invalid commands cannot be highlighted
	TRef						m_NextMenu;			//	if menu command is "open" then this is the menu we open
	TRef						m_refMeshID;		// Mesh reference - for use instead of a string
};


//----------------------------------------------
//	a menu rendered to the screen
//----------------------------------------------
class TLMenu::TMenu
{
public:
	
	explicit TMenu(TRef refMenuID) :
		m_refMenuID(refMenuID)
	{
	}
	
	TPtrArray<TMenuItem>&		GetMenuItems()									{	return m_MenuItems;	}
	TPtr<TMenuItem>				GetMenuItem(TRefRef MenuItemRef)				{	return m_MenuItems.FindPtr( MenuItemRef );	}
	Bool						GetMenuItemExists(TRefRef MenuItemRef) const	{	return m_MenuItems.Exists( MenuItemRef );	}
	TPtr<TMenuItem>				GetCurrentMenuItem(TRefRef MenuItemRef)			{	return GetMenuItem( m_HighlightMenuItem );	}
	Bool						GetMenuItemExists(TRefRef MenuItemRef)			{	return m_MenuItems.Exists( MenuItemRef );	}

	TPtr<TLMenu::TMenuItem>		AddMenuItem(TRefRef MenuItemRef);		//	create a menu item - returns NULL if duplicated menu item ref

	void						SetHighlightedMenuItem(TRefRef MenuItemRef)	{	m_HighlightMenuItem = MenuItemRef;	}
	
	TRefRef						GetMenuID()				{ return m_refMenuID; }

protected:
	TRef						m_refMenuID;				//	Id of the menu
	TRef						m_HighlightMenuItem;		//	currently highlighted menu item
	TPtrArray<TMenuItem>		m_MenuItems;				//	list of menu items
};



//----------------------------------------------
//	controls menu flow
//----------------------------------------------
class TLMenu::TMenuController : public TLMessaging::TPublisher, public TLMessaging::TSubscriber
{
public:
	TMenuController()	{};
	
	//	external commands
	TPtr<TMenu>			GetCurrentMenu()					{	return m_MenuStack.GetPtrLast();	}	//	check IsMenuOpen() before accessing first
	Bool				IsMenuOpen() const					{	return m_MenuStack.GetSize() > 0;	}
	Bool				OpenMenu(TRefRef MenuRef);			//	move onto new menu with this ref - returns false if no such menu
	void				CloseMenu();						//	close this menu and go back to previous
	void				CloseAllMenus();					//	clear menu stack
	Bool				HighlightMenuItem(TRef MenuItemRef);	//	highlight a menu item
	Bool				ExecuteMenuItem(TRefRef MenuItemRef);	//	execute menu item command
	Bool				GetMenuItemExists(TRefRef MenuItem) const	{	return IsMenuOpen() ? m_MenuStack.ElementLastConst()->GetMenuItemExists(MenuItem) : FALSE;	}

protected:
	virtual TPtr<TMenu>	CreateMenu(TRefRef MenuRef);			//	create a menu. default just loads menu definition from assets, overload to create custom menus
	virtual Bool		ExecuteCommand(TRefRef MenuCommand)	{	return FALSE;	}	//	execute menu item command 

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

