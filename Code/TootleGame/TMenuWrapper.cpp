/*
 *  TMenuWrapper.cpp
 *  TootleGame
 *
 *  Created by Duane Bradbury on 17/12/2009.
 *  Copyright 2009 Tootle. All rights reserved.
 *
 */

#include "TMenuWrapper.h"

#include <TootleAsset/TScheme.h>
#include <TootleRender/TRendergraph.h>
#include <TootleRender/TRenderNodeText.h>
#include <TootleGame/TWidgetManager.h>



TLGame::TMenuWrapper::TMenuWrapper(TLMenu::TMenuController& MenuController, TRefRef RenderTargetRef) :
	m_RenderTargetRef	( RenderTargetRef ),
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
	//m_Widgets.FunctionAll( &TLGui::TWidget::Shutdown );
	
	// Remove widgets from the widget system
	for(u32 uIndex = 0; uIndex < m_Widgets.GetSize(); uIndex++)
	{
		TRef WidgetRef = m_Widgets.ElementAt(uIndex);
		TLGui::g_pWidgetManager->RemoveWidget(m_RenderTargetRef, WidgetRef);
	}
	
	m_Widgets.Empty();
	
}



//---------------------------------------------------
//	create menu/render nodes etc
//---------------------------------------------------
TLGame::TMenuWrapperScheme::TMenuWrapperScheme(TLMenu::TMenuController& MenuController,TRefRef SchemeRef,TRefRef ParentRenderNodeRef,TRefRef RenderTargetRef) :
TMenuWrapper		( MenuController, RenderTargetRef )
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
		
		
		// Subscribe to the render graph so we can be notified when the new node is actually instanced into the graph
		SubscribeTo(TLRender::g_pRendergraph);
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
		
		CreateButtonWidget(RenderTargetRef, *pWidgetData);
	}
	
}

void TLGame::TMenuWrapperScheme::CreateButtonWidget(TRefRef RenderTargetRef, TBinaryTree& WidgetData)
{
	TRef ButtonWidget = TLGui::g_pWidgetManager->CreateWidget(RenderTargetRef, "msbutton", "button");
	
	if(ButtonWidget.IsValid())
	{
		TLMessaging::TMessage InitMessage(TLCore::InitialiseRef);
		
		InitMessage.CopyDataTree(WidgetData, FALSE);

		InitMessage.ExportData("RTarget", RenderTargetRef); // NOTE: should be able to remove this soon
		InitMessage.ExportData("User", TRef_Static(g,l,o,b,a));
		InitMessage.ExportData<bool>("Enable", TRUE);
		
		m_Widgets.Add(ButtonWidget);
		
		TLGui::g_pWidgetManager->SendMessageToWidget(RenderTargetRef, ButtonWidget, InitMessage);
		TLGui::g_pWidgetManager->SubscribeToWidget(RenderTargetRef, ButtonWidget, this );
		
	}	
}


//---------------------------------------------------
//	catch widget's messages and turn them into menu item execution for our owner menu controller
//---------------------------------------------------
void TLGame::TMenuWrapperScheme::ProcessMessage(TLMessaging::TMessage& Message)
{
	TRef MessageRef = Message.GetMessageRef();
	//	action from a gui - match it to a menu item, then invoke the "click" of that menu item
	if ( MessageRef == TRef_Static(A,c,t,i,o) )
	{
		TRef ActionRef;
		if ( Message.Read( ActionRef ) )
		{
			TRef MenuItemRef = ActionRef;
			m_pMenuController->ExecuteMenuItem( MenuItemRef );
		}
	}
	else if(MessageRef == TRef("NodeAdded"))
	{
		TRef NodeRef;
		Message.ResetReadPos();
		if(Message.Read( NodeRef ))
		{
			if(NodeRef == m_RenderNode)
				OnSchemeInstanced();
		}
	}
}


void TLGame::TMenuWrapperScheme::OnSchemeInstanced()
{
	// Unsuscribe from the render graph
	UnsubscribeFrom(TLRender::g_pRendergraph);
}


//---------------------------------------------------
//	create menu/render nodes etc
//---------------------------------------------------
TLGame::TMenuWrapperText::TMenuWrapperText(TLMenu::TMenuController& MenuController,TRefRef FontRef,float FontScale,TRefRef ParentRenderNodeRef,TRefRef RenderTargetRef,TRef ParentRenderNodeDatum) :
TMenuWrapper		( MenuController, RenderTargetRef )
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
		if ( MenuItem.GetString().GetLength() == 0 )
		{
			TTempString RefString;
			MenuItem.GetMenuItemRef().GetString( RefString );
			InitMessage.ExportDataString("string", RefString );
		}
		else
		{
			InitMessage.ExportDataString("string", MenuItem.GetString() );
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
		
				
		//	make the rendernode of this menu item clickable, the action coming out of the TWidget
		//	is the ref of the menu item that was clicked
		TRef ButtonWidget = TLGui::g_pWidgetManager->CreateWidget(RenderTargetRef, "mtbutton", "button");
		
		if(ButtonWidget.IsValid())
		{
			TLMessaging::TMessage InitMessage(TLCore::InitialiseRef);
			
			InitMessage.ExportData("RTarget", RenderTargetRef); // NOTE: should be able to remove this soon
			InitMessage.ExportData( TRef_Static4(N,o,d,e), MenuItemRenderNodeRef );
			
			InitMessage.ExportData("User", TRef_Static(g,l,o,b,a));
			
			//	the action from the widget is the menu item ref
			InitMessage.ExportData("ActDown", MenuItem.GetMenuItemRef() );

			InitMessage.ExportData<bool>("Enable", TRUE);
			
			m_Widgets.Add(ButtonWidget);
			
			TLGui::g_pWidgetManager->SendMessageToWidget(RenderTargetRef, ButtonWidget, InitMessage);
			TLGui::g_pWidgetManager->SubscribeToWidget(RenderTargetRef, ButtonWidget, this );
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
