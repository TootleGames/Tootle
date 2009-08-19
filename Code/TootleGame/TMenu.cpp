#include "TMenu.h"
#include <TootleAsset/TScheme.h>
#include <TootleRender/TRendergraph.h>
#include <TootleRender/TRenderNodeText.h>
#include <TootleAudio/TAudiograph.h>
#include <TootleGame/TWidgetButton.h>



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
		if ( !ExecuteCommand( MenuCommand ) )
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
	OnMenuItemExecuted( MenuCommand );

	return TRUE;
}


//----------------------------------------------
//	create a menu. default just loads menu definition from assets, overload to create custom menus
//----------------------------------------------
TPtr<TLMenu::TMenu> TLMenu::TMenuController::CreateMenu(TRefRef MenuRef)
{
	//	find menu asset - note: this has to be block loaded because its not too easy to make this an async operation
	TPtr<TLAsset::TMenu> pMenuAsset = TLAsset::GetAssetPtr<TLAsset::TMenu>( MenuRef );
	if ( !pMenuAsset )
	{
		TTempString Debug_String("Failed to find menu asset ");
		MenuRef.GetString( Debug_String );
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
void TLMenu::TMenuController::OnMenuItemExecuted(TRefRef MenuCommand)
{
	TLMessaging::TMessage Message("Execute");
	
	//	write the command into the message
	Message.ExportData("Command", MenuCommand );

	PublishMessage( Message );
}


TLGame::TMenuWrapper::TMenuWrapper(TLMenu::TMenuController& MenuController) :
	m_pMenuController	( &MenuController )
{
	if ( !m_pMenuController )
	{
		TLDebug_Break("Menu controller expected");
	}
}


//---------------------------------------------------
//	cleanup
//---------------------------------------------------
TLGame::TMenuWrapper::~TMenuWrapper()
{
	//	delete render node
	TLRender::g_pRendergraph->RemoveNode( m_RenderNode );

	//	callback so we can do extra widget-removed code
	OnWidgetsRemoved( m_Widgets );

	//	dealloc widgets - shut them down first to make sure all TPtr's are released
	m_Widgets.FunctionAll( &TLGui::TWidget::Shutdown );
	m_Widgets.Empty();
}



//---------------------------------------------------
//	create menu/render nodes etc
//---------------------------------------------------
TLGame::TMenuWrapperScheme::TMenuWrapperScheme(TLMenu::TMenuController& MenuController,TRefRef SchemeRef,TRefRef ParentRenderNodeRef,TRefRef RenderTargetRef) :
	TMenuWrapper		( MenuController )
{
	if ( !m_pMenuController )
		return;

	TLMenu::TMenu& Menu = *m_pMenuController->GetCurrentMenu();
	m_MenuRef = Menu.GetMenuRef();

	//	load scheme under this node
	if ( SchemeRef.IsValid() )
	{
		//	load asset
		TLAsset::TScheme* pScheme = TLAsset::GetAsset<TLAsset::TScheme>( SchemeRef );
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
		m_RenderNode = TLRender::g_pRendergraph->CreateNode( TRef(), TRef(), ParentRenderNodeRef );

		//	import scheme
		TLRender::g_pRendergraph->ImportScheme( pScheme, m_RenderNode );
	}

	//	create TWidget's for each menu item
	TPtrArray<TLMenu::TMenuItem>& MenuItems = Menu.GetMenuItems();
	for ( u32 i=0;	i<MenuItems.GetSize();	i++ )
	{
		//	get widget data
		TPtr<TBinaryTree> pWidgetData = MenuItems[i]->GetData().GetChild("Widget");

		//	no widget data, make up our own based on old implementation
		if ( !pWidgetData )
		{
			//	get render node ref usable for the TWidget
			TRefRef MenuItemRenderNodeRef = MenuItems[i]->GetMeshRef();
			if ( !MenuItemRenderNodeRef.IsValid() )
				continue;
			
			TLDebug_Warning("Using old widget-generation method for a menu item - please switch to using widget <data>");
			pWidgetData = new TBinaryTree( TRef_Static(W,i,d,g,e) );
			pWidgetData->ExportData( TRef_Static4(N,o,d,e), MenuItemRenderNodeRef );
		}

		//	the action from the widget is the menu item ref
		pWidgetData->ExportData("ActDown", MenuItems[i]->GetMenuItemRef() );

		//	make the rendernode of this menu item clickable, the action coming out of the TWidget
		//	is the ref of the menu item that was clicked
		TPtr<TLGui::TWidgetButton> pWidget = new TLGui::TWidgetButton( RenderTargetRef, *pWidgetData );

		//	subscribe the menu controller to the gui to get the clicked messages
		//	gr: THIS now gets the gui messages and handles them and invokes execution of the menu item
		if ( this->SubscribeTo( pWidget ) )
		{
			m_Widgets.Add( pWidget );
		}
	}

}

	
//---------------------------------------------------
//	catch widget's messages and turn them into menu item execution for our owner menu controller
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
TLGame::TMenuWrapperText::TMenuWrapperText(TLMenu::TMenuController& MenuController,TRefRef FontRef,float FontScale,TRefRef ParentRenderNodeRef,TRefRef RenderTargetRef,TRef ParentRenderNodeDatum) :
	TMenuWrapper		( MenuController )
{
	if ( !m_pMenuController )
		return;

	TLMenu::TMenu& Menu = *m_pMenuController->GetCurrentMenu();
	m_MenuRef = Menu.GetMenuRef();

	//	create root render node to store the text render nodes under - this also saves us manually cleaning up the other nodes
	m_RenderNode = TLRender::g_pRendergraph->CreateNode("_MenuRoot", TRef(), ParentRenderNodeRef );

	TPtrArray<TLAsset::TMenu::TMenuItem>& MenuItems = Menu.GetMenuItems();

	float3 TextPosition( 0.f, 0.f, 2.f );
	float3 TextScale( FontScale, FontScale, 1.f );
	float3 TextPositionStep( 0.f, TextScale.y * 1.0f, 0.f );

	//	create Text for each menu item
	for ( u32 i=0;	i<MenuItems.GetSize();	i++ )
	{
		TLAsset::TMenu::TMenuItem& MenuItem = *MenuItems[i];
		
		//	create text render node
		TLMessaging::TMessage InitMessage(TLCore::InitialiseRef);

		//	if string is missing, convert the menu item ref
		if ( MenuItem.GetText().GetLength() == 0 )
		{
			TTempString RefString;
			MenuItem.GetMenuItemRef().GetString( RefString );
			InitMessage.ExportDataString("string", RefString );
		}
		else
		{
			InitMessage.ExportDataString("string", MenuItem.GetText() );
		}

		//InitMessage.ExportData("DbgDatum", TRef("_BnB2") );
		InitMessage.ExportData("translate", TextPosition );
		InitMessage.ExportData("scale", TextScale );
		InitMessage.ExportData("FontRef", FontRef );
		
		//	gr: todo; make all the alignment stuff optional
		if ( ParentRenderNodeDatum.IsValid() )
		{
			InitMessage.ExportData("HAlign", TLRenderText::HAlignCenter );
			InitMessage.ExportData("BoxNode", ParentRenderNodeRef );
			InitMessage.ExportData("BoxDatum", ParentRenderNodeDatum );
		}

		//	create
		TRef MenuItemRenderNodeRef = TLRender::g_pRendergraph->CreateNode("_Text", "TxText", m_RenderNode, &InitMessage );
		if ( !MenuItemRenderNodeRef.IsValid() )
		{
			TLDebug_Break("failed to create text menu item render node");
			continue;
		}

		//	move along position
		TextPosition += TextPositionStep;

		//	create widget
		TBinaryTree WidgetData( TRef_Static(W,i,d,g,e) );
		WidgetData.ExportData( TRef_Static4(N,o,d,e), MenuItemRenderNodeRef );

		//	the action from the widget is the menu item ref
		WidgetData.ExportData("ActDown", MenuItem.GetMenuItemRef() );

		//	make the rendernode of this menu item clickable, the action coming out of the TWidget
		//	is the ref of the menu item that was clicked
		TPtr<TLGui::TWidgetButton> pWidget = new TLGui::TWidgetButton( RenderTargetRef, WidgetData );

		//	subscribe the menu controller to the gui to get the clicked messages
		//	gr: THIS now gets the gui messages and handles them and invokes execution of the menu item
		if ( this->SubscribeTo( pWidget ) )
		{
			m_Widgets.Add( pWidget );
		}
	}
}



//---------------------------------------------------
//	catch widget's messages and turn them into menu item execution for our owner menu controller
//---------------------------------------------------
void TLGame::TMenuWrapperText::ProcessMessage(TLMessaging::TMessage& Message)
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
