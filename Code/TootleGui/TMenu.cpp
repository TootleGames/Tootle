#include "TMenu.h"
#include <TootleGame/TMenuController.h>	//	gr: including game lib file :( see about moving the menu handling system to the core?


TLGui::TMenuWrapper::TMenuWrapper(TLMenu::TMenuController& MenuController,TLGui::TMenuHandler& MenuHandler,bool PopupMenu) :
	m_pMenuHandler		( &MenuHandler ),
	m_pMenuController	( &MenuController )
{
	if ( GetMenuController().IsMenuOpen() )
	{
		TLMenu::TMenu* pMenu = GetMenuController().GetCurrentMenu();
		m_MenuRef = GetMenuHandler().CreateMenu( pMenu->GetAsset(), PopupMenu );
	}
	else
	{
		TLDebug_Break("Menu expected to be open in menu controller");
	}
	
	//	subscribe to the window to get the menu callbacks
	this->SubscribeTo( &MenuHandler.GetPublisher() );
}

TLGui::TMenuWrapper::~TMenuWrapper()
{
	if ( m_pMenuHandler )
		m_pMenuHandler->DestroyMenu( m_MenuRef );
}



void TLGui::TMenuWrapper::ProcessMessage(TLMessaging::TMessage& Message)
{
	//	catch menu item being selected
	if ( Message.GetMessageRef() == "OnMenu" )
	{
		TRef ItemRef;
		if ( Message.ImportData("ItemRef", ItemRef ) )
		{
			//	notify the controller to execute the message
			//	gr: does this need to be deffered?
			GetMenuController().ExecuteMenuItem( ItemRef );
		}
	}
}
