#include "TMenu.h"
#include <TootleAsset/TScheme.h>
#include <TootleRender/TRendergraph.h>
#include <TootleAudio/TAudiograph.h>



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

	TRefRef AudioRef = pMenuItem->GetAudioRef();;

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

	// Valid audio to play?
	// Create menu audio for command execution
	if(AudioRef.IsValid())
	{
		TLMessaging::TMessage Message(TLCore::InitialiseRef);
		Message.ExportData("Asset", AudioRef);
		Message.ExportData("Play", TRUE);
		Message.ExportData("RateOfDecay", 0.0f); // Make 2D
		Message.ExportData("MinRange", 100000.0f);
		Message.ExportData("MaxRange", 100000.0f);
		
		TLAudio::g_pAudiograph->StartAudio(MenuCommand, AudioRef);
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
void TLMenu::TMenuController::ProcessMessage(TLMessaging::TMessage& Message)
{
	TRefRef MessageRef = Message.GetMessageRef();
	
	//	open new menu
	if ( MessageRef == "Open" )
	{
		TRef MenuRef;
		if ( Message.ImportData( "MenuRef", MenuRef ) )
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
		if ( Message.ImportData( "ItemRef", MenuItemRef ) )
		{
			HighlightMenuItem( MenuItemRef );
		}
		return;
	}

	//	execute menu item
	if ( MessageRef == "Execute" )
	{
		TRef MenuItemRef;
		if ( Message.ImportData( "ItemRef", MenuItemRef ) )
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
	TLMessaging::TMessage Message("Open");
	PublishMessage( Message );
}

//----------------------------------------------
//	moved to previous menu
//----------------------------------------------
void TLMenu::TMenuController::OnMenuClose()
{
	TLMessaging::TMessage Message("Close");
	PublishMessage( Message );
}


//----------------------------------------------
//	closed all menus
//----------------------------------------------
void TLMenu::TMenuController::OnMenuCloseAll()
{
	TLMessaging::TMessage Message("CloseAll");
	PublishMessage( Message );
}


//----------------------------------------------
//	highlighted menu item
//----------------------------------------------
void TLMenu::TMenuController::OnMenuItemHighlighted()
{
	TLMessaging::TMessage Message("Highlight");
	PublishMessage( Message );
}


//----------------------------------------------
//	menu item executed
//----------------------------------------------
void TLMenu::TMenuController::OnMenuItemExecuted(TRefRef MenuCommand)
{
	TLMessaging::TMessage Message("Execute");
	
	//	write the command into the message
	Message.ExportData("Command", MenuCommand );

	PublishMessage( Message );
}



//---------------------------------------------------
//	cleanup
//---------------------------------------------------
TLGame::TMenuWrapper::~TMenuWrapper()
{
	//	delete render node
	TLRender::g_pRendergraph->RemoveNode( m_RenderNode );
}



//---------------------------------------------------
//	create menu/render nodes etc
//---------------------------------------------------
TLGame::TMenuWrapperScheme::TMenuWrapperScheme(TLMenu::TMenuController& MenuController,TRefRef SchemeRef,TRefRef ParentRenderNodeRef,TRefRef RenderTargetRef) :
	m_pMenuController	( &MenuController )
{
	TLMenu::TMenu& Menu = *MenuController.GetCurrentMenu();
	m_MenuRef = Menu.GetMenuRef();

	//	load scheme under this node
	if ( SchemeRef.IsValid() )
	{
		//	load asset
		TPtr<TLAsset::TScheme> pScheme = TLAsset::LoadAsset( SchemeRef, TRUE );
		if ( pScheme && pScheme->GetAssetType() != "Scheme" )
			pScheme = NULL;

		if ( !pScheme )
		{
#ifdef _DEBUG
			TTempString Debug_String("failed to find scheme ");
			SchemeRef.GetString( Debug_String );
			Debug_String.Append(" for menu ");
			m_MenuRef.GetString( Debug_String );
			TLDebug_Break( Debug_String );
#endif
			return;
		}

		//	create empty root render node to put scheme under
		m_RenderNode = TLRender::g_pRendergraph->CreateNode( m_MenuRef, TRef(), ParentRenderNodeRef );

		//	import scheme
		TLRender::g_pRendergraph->ImportScheme( pScheme, m_RenderNode );
	}

	//	create TWidget's for each menu item
	const TPtrArray<TLMenu::TMenuItem>& MenuItems = Menu.GetMenuItems();
	for ( u32 i=0;	i<MenuItems.GetSize();	i++ )
	{
		//	get render node ref usable for the TWidget
		TRefRef MenuItemRenderNodeRef = MenuItems[i]->GetMeshRef();
		if ( !MenuItemRenderNodeRef.IsValid() )
			continue;

		//	make the rendernode of this menu item clickable, the action coming out of the TWidget
		//	is the ref of the menu item that was clicked
		TPtr<TLGui::TWidget> pGui = new TLGui::TWidget( RenderTargetRef, MenuItemRenderNodeRef, "global", MenuItems[i]->GetMenuItemRef() );

		//	subscribe the menu controller to the gui to get the clicked messages
		//	gr: THIS now gets the gui messages and handles them and invokes execution of the menu item
		if ( this->SubscribeTo( pGui.GetObject() ) )
		{
			m_Guis.Add( pGui );
		}
	}

}


//---------------------------------------------------
//	delete render nodes
//---------------------------------------------------
TLGame::TMenuWrapperScheme::~TMenuWrapperScheme()
{
	//	dealloc guis - shut them down first to make sure all TPtr's are released
	m_Guis.FunctionAll( &TLGui::TWidget::Shutdown );
	m_Guis.Empty();
}

	
//---------------------------------------------------
//	catch gui's messages and turn them into menu item execution for our owner menu controller
//---------------------------------------------------
void TLGame::TMenuWrapperScheme::ProcessMessage(TLMessaging::TMessage& Message)
{
	//	action from a gui - match it to a menu item, then invoke the "click" of that menu item
	if ( Message.GetMessageRef() == TRef_Static(A,c,t,i,o) )
	{
		TRef ActionRef;
		if ( Message.Read( ActionRef ) )
		{
			TRef MenuItemRef = ActionRef;
			m_pMenuController->ExecuteMenuItem( MenuItemRef );
		}
	}
}


//---------------------------------------------------
//	create menu/render nodes etc
//---------------------------------------------------
TLGame::TMenuWrapperText::TMenuWrapperText(TLMenu::TMenuController* pMenuController,TRefRef SchemeRef,TRefRef ParentRenderNodeRef,TRefRef RenderTargetRef)
{
	TPtr<TLMenu::TMenu>& pMenu = pMenuController->GetCurrentMenu();
	m_MenuRef = pMenu->GetMenuRef();

	//	load scheme under this node
	if ( SchemeRef.IsValid() )
	{
		//	load asset
		TPtr<TLAsset::TScheme> pScheme = TLAsset::LoadAsset( SchemeRef, TRUE );
		if ( pScheme && pScheme->GetAssetType() != "Scheme" )
			pScheme = NULL;

		if ( !pScheme )
		{
#ifdef _DEBUG
			TTempString Debug_String("failed to find scheme ");
			SchemeRef.GetString( Debug_String );
			Debug_String.Append(" for menu ");
			m_MenuRef.GetString( Debug_String );
			TLDebug_Break( Debug_String );
#endif
			return;
		}

		//	create empty root render node to put scheme under
		m_RenderNode = TLRender::g_pRendergraph->CreateNode( m_MenuRef, TRef(), ParentRenderNodeRef );

		//	import scheme
		TLRender::g_pRendergraph->ImportScheme( pScheme, m_RenderNode );
	}

	//	create TWidget's for each menu item
	const TPtrArray<TLMenu::TMenuItem>& MenuItems = pMenu->GetMenuItems();
	for ( u32 i=0;	i<MenuItems.GetSize();	i++ )
	{
		//	get render node ref usable for the TWidget
		TRefRef MenuItemRenderNodeRef = MenuItems[i]->GetMeshRef();
		if ( !MenuItemRenderNodeRef.IsValid() )
			continue;

		//	make the rendernode of this menu item clickable, the action coming out of the TWidget
		//	is the ref of the menu item that was clicked
		TPtr<TLGui::TWidget> pGui = new TLGui::TWidget( RenderTargetRef, MenuItemRenderNodeRef, "global", MenuItems[i]->GetMenuItemRef() );

		//	subscribe the menu controller to the gui to get the clicked messages
		if ( pMenuController->SubscribeTo( pGui.GetObject() ) )
		{
			m_Guis.Add( pGui );
		}
	}

}

