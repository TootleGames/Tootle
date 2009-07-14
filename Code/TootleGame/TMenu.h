/*------------------------------------------------------

	Menu controller

-------------------------------------------------------*/
#pragma once

#include <TootleCore/TLTypes.h>
#include <TootleAsset/TMenu.h>
#include <TootleCore/TPublisher.h>
#include <TootleCore/TSubscriber.h>
#include "TWidget.h"


namespace TLMenu
{
	class TMenuController;
	class TMenu;

	typedef TLAsset::TMenu::TMenuItem TMenuItem;
};

namespace TLGame
{
	class TMenuWrapper;
	class TMenuWrapperScheme;
	class TMenuWrapperText;
}


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
	FORCEINLINE TRefRef					GetSchemeRef()									{	return m_pMenuAsset->GetSchemeRef();	}

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
	
	void				Shutdown()								{	TLMessaging::TPublisher::Shutdown();	TLMessaging::TSubscriber::Shutdown();	}

	//	external commands
	TPtr<TMenu>&		GetCurrentMenu()						{	return m_MenuStack.GetPtrLast();	}	//	check IsMenuOpen() before accessing first
	const TPtr<TMenu>&	GetCurrentMenu() const					{	return m_MenuStack.GetPtrLast();	}	//	check IsMenuOpen() before accessing first
	Bool				IsMenuOpen() const						{	return m_MenuStack.GetSize() > 0;	}
	Bool				OpenMenu(TRefRef MenuRef);				//	move onto new menu with this ref - returns false if no such menu
	void				CloseMenu();							//	close this menu and go back to previous
	void				CloseAllMenus();						//	clear menu stack
	Bool				HighlightMenuItem(TRef MenuItemRef);	//	highlight a menu item
	Bool				ExecuteMenuItem(TRefRef MenuItemRef);	//	execute menu item command
	Bool				GetMenuItemExists(TRefRef MenuItem) const	{	return IsMenuOpen() ? GetCurrentMenu()->GetMenuItemExists(MenuItem) : FALSE;	}

protected:
	virtual TPtr<TMenu>		CreateMenu(TRefRef MenuRef);			//	create a menu. default just loads menu definition from assets, overload to create custom menus
	virtual Bool			ExecuteCommand(TRefRef MenuCommand)	{	return FALSE;	}	//	execute menu item command 

	TPtr<TMenuItem>		GetMenuItem(TRefRef MenuItemRef);	//	get menu item out of current menu

	//	incoming events
	virtual void		ProcessMessage(TLMessaging::TMessage& Message);

	//	outgoing events
	virtual void		OnMenuOpen();						//	moved onto new menu
	virtual void		OnMenuClose();						//	moved to previous menu
	virtual void		OnMenuCloseAll();					//	closed all menus
	virtual void		OnMenuItemHighlighted();			//	highlighted menu item
	virtual void		OnMenuItemExecuted(TRefRef MenuCommand);	//	menu item executed

protected:
	TPtrArray<TMenu>	m_MenuStack;						//	menu stack
};




class TLGame::TMenuWrapper
{
protected:
	virtual ~TMenuWrapper();

protected:
	TRef	m_RenderNode;		//	root render node added to the parent specified in the constructor
};



//----------------------------------------------
//	gr: this class puts a menu and a scheme together to create clickable menu items.
//	todo: rename this and sort all these classes out into one simple, but overloadable system
//	this class will probably get renamed too
//----------------------------------------------
class TLGame::TMenuWrapperScheme : public TLGame::TMenuWrapper, public TLMessaging::TSubscriber
{
public:
	TMenuWrapperScheme(TLMenu::TMenuController& MenuController,TRefRef SchemeRef,TRefRef ParentRenderNodeRef,TRefRef RenderTargetRef);	//	create menu/render nodes etc
	~TMenuWrapperScheme();

	Bool					IsValid()			{	return m_MenuRef.IsValid();	}
	void					SetInvalid()		{	m_MenuRef.SetInvalid();	}
	
protected:
	virtual void					ProcessMessage(TLMessaging::TMessage& Message);	//	catch gui's messages and turn them into menu item execution for our owner menu controller

protected:
	TRef								m_MenuRef;
	TLMenu::TMenuController*			m_pMenuController;		//	owner menu controller
	TPtrArray<TLGui::TWidget>	m_Guis;					//	gui storage
};


//----------------------------------------------
//	This class creates text render nodes to make 
//	up a menu that looks like a popup menu
//----------------------------------------------
class TLGame::TMenuWrapperText : public TLGame::TMenuWrapper
{
public:
	TMenuWrapperText(TLMenu::TMenuController* pMenuController,TRefRef FontRef,TRefRef ParentRenderNodeRef,TRefRef RenderTargetRef);	//	create menu/render nodes etc

protected:
	TRef								m_MenuRef;
	TPtrArray<TLGui::TWidget>	m_Guis;		//	gui storage
};


