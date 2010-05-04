/*
 *  TMenuController.cpp
 *  TootleGame
 *
 *  Created by Duane Bradbury on 17/12/2009.
 *  Copyright 2009 Tootle. All rights reserved.
 *
 */

#include "TMenuController.h"

#include <TootleAudio/TAudiograph.h>


//----------------------------------------------
//	move onto new menu with this ref - returns false if no such menu
//----------------------------------------------
Bool TLMenu::TMenuController::OpenMenu(TRefRef MenuRef)
{
	//	create new menu, get/create the asset to base it on...
	TPtr<TLAsset::TMenu> pNewMenuAsset = CreateMenu(MenuRef);
	if ( !pNewMenuAsset )
		return FALSE;

	//	now create the runtime menu
	TPtr<TMenu> pNewMenu = new TMenu( pNewMenuAsset );
	if ( !pNewMenu )
		return false;
	
	//	add to end of stack
	if ( m_MenuStack.Add( pNewMenu ) == -1 )
		return false;
	
	//	todo: show menu-is-hidden message?
	
	//	send menu-is-open message
	OnMenuOpen();
	
	return true;
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
		// Already has a queued command?
		if(m_QueuedCommand.GetRef().IsValid())
			return FALSE;
		
		// Queue up the command.  
		m_QueuedCommand.SetRef(MenuCommand);
		m_QueuedCommand.SetTypeRef(pMenuItem->GetNextMenu());
	}
	else if ( MenuCommand == "Close" )
	{
		// Already has a queued command?
		if(m_QueuedCommand.GetRef().IsValid())
			return FALSE;
		
		// Queue up the command.  
		m_QueuedCommand.SetRef(MenuCommand);
	}
	else
	{
		//	do non standard command
		if ( !ExecuteCommand( MenuCommand, pMenuItem->GetData() ) )
			return FALSE;
	}
	
	// Valid audio to play?
	// Create menu audio for command execution
	//	gr: this should go in menu renderer code, not menu logic code
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
	OnMenuItemExecuted( MenuCommand, pMenuItem->GetData() );
	
	return TRUE;
}


//----------------------------------------------
//	create a menu. default just loads menu definition from assets, overload to create custom menus
//----------------------------------------------
TPtr<TLAsset::TMenu> TLMenu::TMenuController::CreateMenu(TRefRef MenuRef)
{
	//	find menu asset - note: this has to be block loaded because its not too easy to make this an async operation
	TPtr<TLAsset::TMenu>& pMenuAsset = TLAsset::GetAssetPtr<TLAsset::TMenu>( MenuRef );
	if ( !pMenuAsset )
	{
		TTempString Debug_String("Failed to find menu asset ");
		MenuRef.GetString( Debug_String );
		TLDebug_Warning( Debug_String );
	}
	
	return pMenuAsset;
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


void TLMenu::TMenuController::Update()
{
	if(m_QueuedCommand.GetRef().IsValid())
	{
		if(m_QueuedCommand.GetRef() == "Open")
		{
			// Open new menu
			OpenMenu( m_QueuedCommand.GetTypeRef() );
		}
		else if(m_QueuedCommand.GetRef() == "close")
		{
			CloseMenu();
		}
		
		// Invalidate
		m_QueuedCommand.SetRef(TRef());
		m_QueuedCommand.SetTypeRef(TRef());
	}
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
		if(!m_QueuedCommand.GetRef().IsValid())
		{
			TRef MenuRef;
			if ( Message.ImportData( "MenuRef", MenuRef ) )
			{
				// Queue up the command.  
				m_QueuedCommand.SetRef(MessageRef);
				m_QueuedCommand.SetTypeRef(MenuRef);
			}
		}
		return;
	}
	
	//	close current menu
	if ( MessageRef == "Close" )
	{
		if(!m_QueuedCommand.GetRef().IsValid())
		{
			// Queue up the command.  
			m_QueuedCommand.SetRef(MessageRef);			
		}
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
	
	if(MessageRef == TLCore::UpdateRef)
	{
		Update();
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
void TLMenu::TMenuController::OnMenuItemExecuted(TRefRef MenuCommand, TBinaryTree& MenuData)
{
	TLMessaging::TMessage Message("Execute");
	
	//	write the command into the message
	Message.ExportData("Command", MenuCommand );
	
	TPtr<TBinaryTree> pChild = Message.AddChild("Menudata");
	if(pChild)
		pChild->ReferenceDataTree(MenuData);
	
	PublishMessage( Message );
}
