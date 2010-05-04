/*
 *  TMenuController.h
 *  TootleGame
 *
 *  Created by Duane Bradbury on 17/12/2009.
 *  Copyright 2009 Tootle. All rights reserved.
 *
 */

#pragma once

#include <TootleCore/TPublisher.h>
#include <TootleCore/TSubscriber.h>

#include "TMenu.h"


namespace TLMenu
{
	class TMenuController;
}

//----------------------------------------------
//	controls menu flow
//----------------------------------------------
class TLMenu::TMenuController : public TLMessaging::TPublisher, public TLMessaging::TSubscriber
{
public:
	TMenuController()	{};

	virtual TRefRef		GetSubscriberRef() const				{	static TRef Ref("MnuCon");	return Ref;	}
	virtual TRefRef		GetPublisherRef() const					{	return GetSubscriberRef();	}
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
	virtual TPtr<TLAsset::TMenu>	CreateMenu(TRefRef MenuRef);			//	create a menu. default just loads menu definition from assets, overload to create custom menus
	virtual Bool		ExecuteCommand(TRefRef MenuCommand)	{	return FALSE;	}	//	execute menu item command 
	virtual Bool		ExecuteCommand(TRefRef MenuCommand,TBinaryTree& MenuItemData)	{	return ExecuteCommand( MenuCommand );	}	//	execute menu item command - gr: new version, provides the data from the menu item as well to do specific stuff - can be null if we are executing a command without using a menu item
	
	TPtr<TMenuItem>		GetMenuItem(TRefRef MenuItemRef);	//	get menu item out of current menu
	
	//	incoming events
	virtual void		ProcessMessage(TLMessaging::TMessage& Message);
	
	virtual void		Update();
	
	//	outgoing events
	virtual void		OnMenuOpen();	//	moved onto new menu
	virtual void		OnMenuClose();						//	moved to previous menu
	virtual void		OnMenuCloseAll();					//	closed all menus
	virtual void		OnMenuItemHighlighted();			//	highlighted menu item
	virtual void		OnMenuItemExecuted(TRefRef MenuCommand, TBinaryTree& MenuData);	//	menu item executed
	
protected:
	TPtrArray<TMenu>	m_MenuStack;						//	menu stack
	
	TTypedRef			m_QueuedCommand;					// Queued command
};

