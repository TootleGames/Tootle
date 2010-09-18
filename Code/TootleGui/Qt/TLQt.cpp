#include "TLQt.h"
#include <TootleRender/TScreenManager.h>
#include "../TWindow.h"
#include "Window.h"
#include "OpenglCanvas.h"
#include "Tree.h"
#include "App.h"
#include <TootleFileSys/TLFile.h>



#if defined(TL_TARGET_PC) && defined(_DEBUG)
	#pragma comment(lib,"QtCored4.lib")
	#pragma comment(lib,"QtGuid4.lib")
	#pragma comment(lib,"QtOpengld4.lib")
#elif defined(TL_TARGET_PC)
	#pragma comment(lib,"libQtCore4.a")
	#pragma comment(lib,"libQtGui4.a")
	#pragma comment(lib,"libQtOpengl4.a")
#endif


//------------------------------------------------------
//	
//------------------------------------------------------
SyncBool TLGui::Platform::Init()
{
	//	todo: create QT mouse and keyboard input devices to push data into
	return SyncTrue;
}

//------------------------------------------------------
//	
//------------------------------------------------------
SyncBool TLGui::Platform::Shutdown()
{
	return SyncTrue;
}


//------------------------------------------------------
//	
//------------------------------------------------------
TPtr<TLGui::TWindow> TLGui::CreateGuiWindow(TRefRef Ref)
{
	TPtr<TLGui::TWindow> pWindow = new Qt::TWindow( Ref );
	return pWindow;
}



TPtr<TLGui::TTree> TLGui::CreateTree(TLGui::TWindow& Parent,TRefRef Ref,TPtr<TLGui::TTreeItem>& pRootItem,const TArray<TRef>& Columns)
{
	TPtr<TLGui::TTree> pControl = new Qt::Tree( Ref, pRootItem, Columns );
	if ( pControl && !pControl->Initialise(Parent) )
		pControl = NULL;
	return pControl;
}



//------------------------------------------------------
//	
//------------------------------------------------------
TPtr<TLGui::TOpenglCanvas> TLGui::CreateOpenglCanvas(TWindow& Parent,TRefRef Ref)
{
	TPtr<TLGui::TOpenglCanvas> pControl = new Qt::TOpenglCanvas( Ref );
	if ( pControl && !pControl->Initialise(Parent) )
		pControl = NULL;
	return pControl;
}


//----------------------------------------------------------
//	get the desktop dimensions. note: need a window so we can decide which desktop?
//----------------------------------------------------------
void TLGui::Platform::GetDesktopSize(Type4<s32>& DesktopSize)
{
	//	some arbitry defaults
	DesktopSize.x = 0;
	DesktopSize.y = 0;
	DesktopSize.Width() = 400;
	DesktopSize.Height() = 400;

	//	get the screen size from the desktop widget
	QDesktopWidget* pDesktop = QApplication::desktop();
	if ( pDesktop )
	{
		QRect DesktopRect = pDesktop->availableGeometry();
		DesktopSize.x = DesktopRect.left();
		DesktopSize.y = DesktopRect.top();;
		DesktopSize.Width() = DesktopRect.width();
		DesktopSize.Height() = DesktopRect.height();
	}	
}


//------------------------------------------------------
//	convert QT variant data to our own format. if DataType specified then explicitly convert to that type
//------------------------------------------------------
bool Qt::GetData(TBinary& Data,const QVariant& Value,TRefRef DataType)
{
	//	just convert to a string and then use our own conversion stuff
	TTempString DataString;
	DataString << Value.toString();
	if ( TLFile::ImportBinaryData( DataString, Data, DataType ) != SyncTrue )
		return false;
	
	return true;
}


//----------------------------------------------------
//	create a menu from this menu
//----------------------------------------------------
TRef Qt::TMenuHandler::CreateMenu(const TLAsset::TMenu& MenuAsset,bool PopupMenu)
{
	QMenu* pMenu = PopupMenu ? AllocPopupMenu() : AllocMenu( MenuAsset );
	if ( !pMenu )
		return TRef();
	QMenu& Menu = *pMenu;
	
	TRef MenuRef = MenuAsset.GetAssetRef();
	
	//	if a menu with this ref already exists, we replace it
	QMenu** ppExistingMenu = m_Menus.Find( MenuRef );
	if ( ppExistingMenu && *ppExistingMenu )
	{
		DestroyMenu( MenuRef );
		ppExistingMenu = NULL;
	}
	
	//	create the child items
	const TPtrArray<TLAsset::TMenu::TMenuItem>& MenuItems = MenuAsset.GetMenuItems();
	for ( u32 i=0;	i<MenuItems.GetSize();	i++ )
	{
		const TLAsset::TMenu::TMenuItem& Item = *MenuItems[i];
			
		//	create new action for the menu
		QAction* pAction = new QAction( Qt::GetString( Item.GetString() ), &GetWidget() );			
		BindAction( *pAction );
		
		//	enable/disable the action
		pAction->setEnabled( Item.IsEnabled() );
		
		//	add to Qt menu
		Menu.addAction( pAction );

		//	store the item/action reference
		m_MenuItemActions.Add( Item.GetMenuItemRef(), pAction );
	}
		
	//	add to list
	m_Menus.Add( MenuRef, &Menu );
		
	return MenuRef;
}


//----------------------------------------------------
//	delete a menu
//----------------------------------------------------
void Qt::TMenuHandler::DestroyMenu(TRefRef MenuRef)
{
	//	get the menu
	QMenu** ppExistingMenu = m_Menus.Find( MenuRef );
	if ( !ppExistingMenu )
		return;
	
	//	remove from menu bar
	//	gr: err can only remove the whole thing... look into this
	//	menuBar().removeMenu( *ppExistingMenu );
		
	//	remove from list
	m_Menus.Remove( MenuRef );
}

//----------------------------------------------------
//	action callback
//----------------------------------------------------
void Qt::TMenuHandler::Slot_OnAction(QAction* pAction)
{
	//	assume the sender is an action and find out which of our TMenu actions it represents
	const TRef* pItemRef = m_MenuItemActions.FindKey( pAction );
	
	//	unknown
	if ( !pItemRef )
	{
		TLDebug_Break("failed to match menu callback action with a menu item ref");
		return;
	}
	
	//	send a menu-item executed message
	TLMessaging::TMessage Message("OnMenu");
	Message.ExportData("ItemRef", *pItemRef );
	
	TLMessaging::TPublisher& Publisher = GetPublisher();
	Publisher.PublishMessage( Message );
}


//----------------------------------------------------
//	create a qt popup menu owned by this widget
//----------------------------------------------------
QMenu* Qt::TMenuHandler::AllocPopupMenu()
{
	QMenu* pPopupMenu = new QMenu( &GetWidget() );
	pPopupMenu->popup( QCursor::pos() );
	return pPopupMenu;
}	


//--------------------------------------------
//	common main-like entry
//--------------------------------------------
int Qt::Main(const TString& Params)
{
	int Result = 0;
	if ( TLGui::OnCommandLine( Params, Result ) )
		return Result;
	
	TLGui::Platform::App TheApp( 0, NULL );
	if ( !TheApp.Init() )
		return 255;
		
	//	execute the OS event loop
	Result = TheApp.exec();
	
	//	cleanup
	TheApp.Shutdown();
	
	return Result;
}


int main(int argc, char *argv[])
{
	//	handle command line
	TStringLowercase<TTempString> Params;
	for ( int i=1;	i<argc;	i++ )
	{
		const char* pParamString = argv[i];
		if ( i>1 )
			Params << " ";
		Params << pParamString;
	}

	return Qt::Main( Params );
}

//	windows entry
#if defined(TL_TARGET_PC)
int WINAPI WinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance,LPSTR lpCmdLine,int nCmdShow)
{
	TLGui::Platform::g_HInstance = hInstance;
	TStringLowercase<TTempString> Params = lpCmdLine;

	return Qt::Main( Params );
}
#endif
