/*
 *  TLGui.cpp
 *  TootleGui
 *
 *  Created by Graham Reeves on 14/01/2010.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */
#include "TLReflection.h"
#include <TootleAsset/TMenu.h>
#include <TootleGui/TWindow.h>
#include <TootleCore/TCoreManager.h>
#include "TGraphTree.h"
#include "TDataTree.h"
#include "TAssetTree.h"


namespace TLReflection
{
	TPtr<TMenuController>	g_pMenu;
	TPtr<TNodeDataTree>		g_pNodeDataWindow;	//	one persistent node tree where required
}
	

//-------------------------------------------------
//	create the master reflection menu for this window
//-------------------------------------------------
bool TLReflection::CreateReflectionMenu(TLGui::TWindow& Window)
{
	//	already exists
	if ( g_pMenu )
		return true;
	
	//	create new menu controller
	g_pMenu = new TLReflection::TMenuController(Window);
	g_pMenu->OpenMenu("Reflection");

	return true;
}



TLReflection::TMenuController::TMenuController(TLGui::TWindow& Window) :
	m_pWindow	( &Window )
{
}

//---------------------------------------------------
//	create a menu. default just loads menu definition from assets, overload to create custom menus
//---------------------------------------------------
TPtr<TLAsset::TMenu> TLReflection::TMenuController::CreateMenu(TRefRef MenuRef)			
{
	//	make up the reflection menu
	if ( MenuRef == "Reflection" )
	{
		TPtr<TLAsset::TMenu> pMenu = new TLAsset::TMenu("Reflection");
		
		//	add show-graph menu items
		TFixedArray<TRef,10> Graphs;
		Graphs << "Render" << "Scene" << "Physics" << "Audio";

		for ( u32 g=0;	g<Graphs.GetSize();	g++ )
		{
			TRef GraphRef = Graphs[g];

			//	add data with the menu item to identify the graph
			TLAsset::TMenu::TMenuItem* pMenuItem = pMenu->AddMenuItem();
			if ( !pMenuItem )
				continue;
			
			//	toggle visibility of graph
			pMenuItem->SetCommand("TogGraph");
			pMenuItem->GetData().ExportData("Graph", GraphRef );

			//	set string
			TTempString ItemString;
			ItemString << "Show " << GraphRef << " graph";
			pMenuItem->SetString( ItemString );
		}

		if ( TLAsset::g_pManager )
		{
			TLAsset::TMenu::TMenuItem* pMenuItem = pMenu->AddMenuItem();
			if ( pMenuItem )
			{
				//	toggle visibility of graph
				pMenuItem->SetCommand("ShAsset");

				//	set string
				TTempString ItemString;
				ItemString << "Show Assets";
				pMenuItem->SetString( ItemString );
			}
		}
		
		return pMenu;
	}
	
	return TLMenu::TMenuController::CreateMenu( MenuRef );
}	


//---------------------------------------------------
//	
//---------------------------------------------------
Bool TLReflection::TMenuController::ExecuteCommand(TRefRef MenuCommand,TBinaryTree& MenuItemData)	
{
	switch ( MenuCommand.GetData() )
	{
		case TRef_Static(S,h,A,s,s):
		{
			//	create new asset tree
			TAssetTree* pTree = new TAssetTree();
			if ( !pTree->IsValid() )
			{
				delete pTree;
				pTree = NULL;
			}
			return true;
		}
		break;

		case TRef_Static(T,o,g,G,r):
		{
			TRef GraphRef;
			TLGraph::TGraphBase* pGraph = NULL;
			if ( MenuItemData.ImportData("Graph", GraphRef ) )
				pGraph = TLCore::g_pCoreManager->GetManager<TLGraph::TGraphBase>( GraphRef );
			
			if ( !pGraph )
			{
				TDebugString Debug_String;
				Debug_String << "Failed to find graph " << GraphRef;
				TLDebug_Break( Debug_String );
				return false;
			}
			
			TLReflection::Private::ToggleGraphTree( *pGraph );
			return true;
		}
		break;
	}
	
	return TLMenu::TMenuController::ExecuteCommand( MenuCommand, MenuItemData );	
}


//---------------------------------------------------
//	moved onto new menu
//---------------------------------------------------
void TLReflection::TMenuController::OnMenuOpen()
{
	//	replace old menu
	TLGui::TMenuHandler* pMenuHandler = m_pWindow->GetMenuHandler();
	if ( pMenuHandler )
	{
		m_pMenuWrapper = new TLGui::TMenuWrapper( *this, *pMenuHandler, false );
	}
	else
	{
		TLDebug_Break("trying to open menu on a window that doesn't support menus. Change reflection code to catch this earlier?");
	}
	
	TLMenu::TMenuController::OnMenuOpen();
}


//---------------------------------------------------
//	closed all menus
//---------------------------------------------------
void TLReflection::TMenuController::OnMenuCloseAll()
{
	m_pMenuWrapper = NULL;
	TLMenu::TMenuController::OnMenuOpen();
}


//---------------------------------------------------
//	toggle the tree window for this graph.
//---------------------------------------------------
TLReflection::TGraphTree* TLReflection::Private::ToggleGraphTree(TLGraph::TGraphBase& Graph)
{
	//	if tree exists
	//		delete tree
	//		return null
	TGraphTree* pGraphTree = new TGraphTree( Graph );
	if ( !pGraphTree->IsValid() )
	{
		delete pGraphTree;
		pGraphTree = NULL;
	}
	
	return pGraphTree;
}


//---------------------------------------------------
//	is there a node-data window up
//---------------------------------------------------
bool TLReflection::IsNodeDataWindowVisible()
{
	if ( !g_pNodeDataWindow )
		return false;
	
	return true;
}


//---------------------------------------------------
//	create a data tree window for this node. if not new-window then we re-use the last one if it exists
//---------------------------------------------------
bool TLReflection::ShowNodeDataWindow(TRefRef Node,TRefRef Graph,bool NewWindow)
{
	TNodeDataTree* pTree = new TNodeDataTree( Node, Graph );
	if ( !pTree->IsValid() )
	{
		delete pTree;
		pTree = NULL;
		return false;
	}

	//	gr: this is a bit of a hack, better support for changing the root node in the tree would be better
	//	if not a new window then put this one in it's place and delete the old one
	if ( !NewWindow )
	{
		//	copy old window's dimensions
		if ( g_pNodeDataWindow )
		{
			pTree->GetWindow().SetPosition( g_pNodeDataWindow->GetWindow().GetPosition() );
			pTree->GetWindow().SetSize( g_pNodeDataWindow->GetWindow().GetSize() );
		}
	
		//	delete the old one and store this one
		g_pNodeDataWindow = pTree;
	}
	
	return true;
}

