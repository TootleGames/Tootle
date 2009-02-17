#include "TMenu.h"




//----------------------------------------------
//	move onto new menu with this ref - returns false if no such menu
//----------------------------------------------
Bool TLMenu::TMenuController::OpenMenu(TRefRef MenuRef)
{
	//	create new menu
	TPtr<TMenu> pNewMenu = CreateMenu(MenuRef);
	if ( !pNewMenu )
		return FALSE;

	//	add to end of stack
	m_MenuStack.Add( pNewMenu );

	//	todo: show menu-is-hidden message?

	//	send menu-is-open message
	OnMenuOpen();

	return TRUE;
}

//----------------------------------------------
//	close this menu and go back to previous
//----------------------------------------------
void TLMenu::TMenuController::CloseMenu()
{
	if ( m_MenuStack.GetSize() == 0 )
	{
		TLDebug_Break("No menus in stack to close");
		return;
	}

	//	remove last menu from stack
	m_MenuStack.RemoveLast();

	//	send out message
	//	if all gone, send all-closed message
	if ( m_MenuStack.GetSize() == 0 )
		OnMenuCloseAll();
	else
		OnMenuClose();

	//	we have a new menu item at top, so "reopen" that
	if ( GetCurrentMenu() )
		OnMenuOpen();
}


//----------------------------------------------
//	clear menu stack
//----------------------------------------------
void TLMenu::TMenuController::CloseAllMenus()
{
	//	clear menu stack
	m_MenuStack.Empty();

	//	send message to say all closed
	OnMenuCloseAll();
}

//----------------------------------------------
//	highlight a menu item
//----------------------------------------------
Bool TLMenu::TMenuController::HighlightMenuItem(TRef MenuItemRef)
{
	//	set menu item as highlighted on menu
	TPtr<TMenu> pCurrentMenu = GetCurrentMenu();
	if ( !pCurrentMenu )
		return FALSE;

	//	is this a valid menu item?
	if ( MenuItemRef.IsValid() )
	{
		if ( !pCurrentMenu->GetMenuItemExists(MenuItemRef) )
		{
			TLDebug_Break("Attempting to highlight non-existant menu item");
			MenuItemRef.SetInvalid();
		}
	}

	//	update highlight on menu
	pCurrentMenu->SetHighlightedMenuItem( MenuItemRef );
	
	//	notify change
	OnMenuItemHighlighted();

	return TRUE;
}

//----------------------------------------------
//	execute menu item command
//----------------------------------------------
Bool TLMenu::TMenuController::ExecuteMenuItem(TRefRef MenuItemRef)
{
	TPtr<TMenuItem> pMenuItem = GetMenuItem(MenuItemRef);
	if ( !pMenuItem )
	{
		TLDebug_Break("No such menu item");
		return FALSE;
	}

	//	get command of menu item
	TRefRef MenuCommand = pMenuItem->GetMenuCommand();

	//	open-menu command
	if ( MenuCommand == "Open" )
	{
		if ( !OpenMenu( pMenuItem->GetNextMenu() ) )
			return FALSE;
	}
	else if ( MenuCommand == "Close" )
	{
		CloseMenu();
	}
	else
	{
		//	do non standard command
		if ( !ExecuteCommand( MenuCommand ) )
			return FALSE;
	}

	//	publish that command has been executed
	OnMenuItemExecuted( MenuCommand );

	return TRUE;
}


//----------------------------------------------
//	create a menu. default just loads menu definition from assets, overload to create custom menus
//----------------------------------------------
TPtr<TLMenu::TMenu> TLMenu::TMenuController::CreateMenu(TRefRef MenuRef)
{
	//	find menu asset - note: this has to be block loaded because its not too easy to make this an async operation
	TPtr<TLAsset::TAsset>& pMenuAsset = TLAsset::LoadAsset( MenuRef, TRUE );
	if ( !pMenuAsset )
	{
		TTempString Debug_String("Failed to find menu asset ");
		MenuRef.GetString( Debug_String );
		TLDebug_Warning( Debug_String );
		return NULL;
	}

	//	not a "menu" asset
	if ( pMenuAsset->GetAssetType() != "Menu" )
	{
		TTempString Debug_String("Menu asset ");
		MenuRef.GetString( Debug_String );
		Debug_String.Append(" is not a Menu, its a ");
		pMenuAsset->GetAssetType().GetString( Debug_String );
		TLDebug_Warning( Debug_String );
		return NULL;
	}

	TPtr<TLMenu::TMenu> pNewMenu = new TLMenu::TMenu( pMenuAsset );
	return pNewMenu;
}


//----------------------------------------------
//	get menu item out of current menu
//----------------------------------------------
TPtr<TLMenu::TMenuItem> TLMenu::TMenuController::GetMenuItem(TRefRef MenuItemRef)
{
	TPtr<TMenu> pCurrentMenu = GetCurrentMenu();
	if ( !pCurrentMenu )
	{
		TLDebug_Break("Current menu expected");
		return NULL;
	}

	return pCurrentMenu->GetMenuItem( MenuItemRef );
}


//----------------------------------------------
//	
//----------------------------------------------
void TLMenu::TMenuController::ProcessMessage(TPtr<TLMessaging::TMessage>& pMessage)
{
	TRefRef MessageRef = pMessage->GetMessageRef();
	
	//	open new menu
	if ( MessageRef == "Open" )
	{
		TRef MenuRef;
		if ( pMessage->ImportData( "MenuRef", MenuRef ) )
		{
			OpenMenu( MenuRef );
		}
		return;
	}

	//	close current menu
	if ( MessageRef == "Close" )
	{
		CloseMenu();
		return;
	}

	//	close all menus
	if ( MessageRef == "CloseAll" )
	{
		CloseAllMenus();
		return;
	}

	//	highlight menu item
	if ( MessageRef == "Highlight" )
	{
		TRef MenuItemRef;
		if ( pMessage->ImportData( "ItemRef", MenuItemRef ) )
		{
			HighlightMenuItem( MenuItemRef );
		}
		return;
	}

	//	execute menu item
	if ( MessageRef == "Execute" )
	{
		TRef MenuItemRef;
		if ( pMessage->ImportData( "ItemRef", MenuItemRef ) )
		{
			ExecuteMenuItem( MenuItemRef );
		}
		return;
	}


}


//----------------------------------------------
//	moved onto new menu
//----------------------------------------------
void TLMenu::TMenuController::OnMenuOpen()
{
	TPtr<TLMessaging::TMessage> pMessage = new TLMessaging::TMessage("Open");
	PublishMessage( pMessage );
}

//----------------------------------------------
//	moved to previous menu
//----------------------------------------------
void TLMenu::TMenuController::OnMenuClose()
{
	TPtr<TLMessaging::TMessage> pMessage = new TLMessaging::TMessage("Close");
	PublishMessage( pMessage );
}


//----------------------------------------------
//	closed all menus
//----------------------------------------------
void TLMenu::TMenuController::OnMenuCloseAll()
{
	TPtr<TLMessaging::TMessage> pMessage = new TLMessaging::TMessage("CloseAll");
	PublishMessage( pMessage );
}


//----------------------------------------------
//	highlighted menu item
//----------------------------------------------
void TLMenu::TMenuController::OnMenuItemHighlighted()
{
	TPtr<TLMessaging::TMessage> pMessage = new TLMessaging::TMessage("Highlight");
	PublishMessage( pMessage );
}


//----------------------------------------------
//	menu item executed
//----------------------------------------------
void TLMenu::TMenuController::OnMenuItemExecuted(TRefRef MenuCommand)
{
	TPtr<TLMessaging::TMessage> pMessage = new TLMessaging::TMessage("Execute");
	
	//	write the command into the message
	pMessage->ExportData("Command", MenuCommand );

	PublishMessage( pMessage );
}


