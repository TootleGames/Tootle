/*------------------------------------------------------

	Basic interface to the reflection library. Not much for the external 
	projects to do, just provide a window to append the reflection menu onto
	(this is a little redundant in OSX as it just adds it to the main menu 
	anyway, but for portability, provide a window to attach the menu to.)

-------------------------------------------------------*/
#pragma once

#include <TootleCore/TLTypes.h>
#include <TootleCore/TRef.h>
#include <TootleGui/TWindow.h>
#include <TootleGui/TMenu.h>
#include <TootleGame/TMenuController.h>	//	gr: including game lib file :( see about moving the menu handling system to the core?


namespace TLGui
{
	class TWindow;
	class TTree;
}

namespace TLGraph
{
	class TGraphBase;
}


namespace TLReflection
{
	class TMenuController;	//	reflection's menu handler
	class TGraphTree;

	bool			CreateReflectionMenu(TLGui::TWindow& Window);		//	create the master reflection menu for this window
	bool			ShowNodeDataWindow(TRefRef Node,TRefRef Graph,bool NewWindow=false);	//	create a data tree window for this node. if not new-window then we re-use the last one if it exists
	bool			IsNodeDataWindowVisible();							//	is there a node-data window up

	namespace Private
	{
		TGraphTree*	ToggleGraphTree(TLGraph::TGraphBase& Graph);	//	toggle the tree window for this graph.
	}
};


//--------------------------------------------------
//	reflection's menu handler
//--------------------------------------------------
class TLReflection::TMenuController : public TLMenu::TMenuController
{
public:
	TMenuController(TLGui::TWindow& Window);
	
	virtual TPtr<TLAsset::TMenu>	CreateMenu(TRefRef MenuRef);	//	create a menu. default just loads menu definition from assets, overload to create custom menus
	virtual Bool					ExecuteCommand(TRefRef MenuCommand,TBinaryTree& MenuItemData);	//	execute menu item command - gr: new version, provides the data from the menu item as well to do specific stuff - can be null if we are executing a command without using a menu item
	
	virtual void					OnMenuOpen();					//	moved onto new menu
	virtual void					OnMenuClose()					{	OnMenuCloseAll();	}
	virtual void					OnMenuCloseAll();				//	closed all menus

protected:
	TLGui::TWindow*				m_pWindow;			//	window to attach to
	TPtr<TLGui::TMenuWrapper>	m_pMenuWrapper;		//	menu wrapper
};

