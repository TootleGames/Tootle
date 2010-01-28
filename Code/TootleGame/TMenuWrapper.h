/*
 *  TMenuWrapper.h
 *  TootleGame
 *
 *  Created by Duane Bradbury on 17/12/2009.
 *  Copyright 2009 Tootle. All rights reserved.
 *
 */

#pragma once


#include <TootleCore/TSubscriber.h>

#include "TMenuController.h"


namespace TLGame
{
	class TMenuWrapper;
	class TMenuWrapperScheme;
	class TMenuWrapperText;
}


class TLGame::TMenuWrapper
{
public:
	TMenuWrapper(TLMenu::TMenuController& MenuController, TRefRef RenderTargetRef);
	virtual ~TMenuWrapper();
	
	Bool						IsValid()			{	return m_MenuRef.IsValid();	}
	void						SetInvalid()		{	m_MenuRef.SetInvalid();	}
	
protected:
	virtual void				OnWidgetsRemoved(TArray<TRef>& Widgets)	{	}	//	callback so we can do extra widget-removed code
	
protected:
	TRef						m_MenuRef;
	TRef						m_RenderTargetRef;
	TRef						m_RenderNode;		//	root render node added to the parent specified in the constructor
	TArray<TRef>				m_Widgets;			//	widget storage
	TLMenu::TMenuController*	m_pMenuController;		//	owner menu controller
};



//----------------------------------------------
//	gr: this class puts a menu and a scheme together to create clickable menu items.
//	todo: rename this and sort all these classes out into one simple, but overloadable system
//	this class will probably get renamed too
//----------------------------------------------
class TLGame::TMenuWrapperScheme : public TLGame::TMenuWrapper, public TLMessaging::TSubscriber
{
public:
	TMenuWrapperScheme(TLMenu::TMenuController& MenuController,TRefRef SchemeRef,TRefRef ParentRenderNodeRef,TRefRef RenderTargetRef);	//	create menu/render nodes etc
	
protected:
	virtual void					ProcessMessage(TLMessaging::TMessage& Message);	//	catch widget's messages and turn them into menu item execution for our owner menu controller
	
	void							CreateButtonWidget(TRefRef RenderTargetRef, TBinaryTree& WidgetData);
	
	virtual void					OnSchemeInstanced();
};


//----------------------------------------------
//	This class creates text render nodes to make 
//	up a menu that looks like a popup menu
//----------------------------------------------
class TLGame::TMenuWrapperText : public TLGame::TMenuWrapper, public TLMessaging::TSubscriber
{
public:
	TMenuWrapperText(TLMenu::TMenuController& MenuController,TRefRef FontRef,float FontScale,TRefRef ParentRenderNodeRef,TRefRef RenderTargetRef,TRef ParentRenderNodeDatum=TRef());	//	create menu/render nodes etc
	
protected:
	virtual void					ProcessMessage(TLMessaging::TMessage& Message);	//	catch widget's messages and turn them into menu item execution for our owner menu controller
};


