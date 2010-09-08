/*
 *  TWxWindow.h
 *  TootleGui
 *
 *  Created by Graham Reeves on 14/01/2010.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */
#if !defined(TL_ENABLE_QT)
#error Should only be built in QT only build
#endif // TL_ENABLE_QT

#pragma once
#include "TLQt.h"
#include "../TWindow.h"
#include <QtGui/QVBoxLayout>
#include <QtGui/QMouseEvent>


namespace Qt
{
	//	cannot use Window as the typename as it's a QT symbol. We can wrap all the
	//	qt stuff with configure, but I'd like QT to jsut work with a plain install of the sdk
	class TWindow;
}


//-------------------------------------------------------
//	
//-------------------------------------------------------
class Qt::TWindow : public QMainWindow, public TLGui::TWindow, public Qt::TWidgetWrapper, public Qt::TMenuHandler
{
	Q_OBJECT
public:
	TWindow(TRefRef WindowRef);

	virtual Bool			IsVisible() const				{	return QWidget::isVisible();	}
	virtual void			Show()							{	QWidget::show();	}
	
	virtual void			SetSize(const Type2<u16>& Size)		{	QWidget::resize( Size.x, Size.y );	}	//	size is internal, FrameSize includes bordr
	virtual Type2<u16>		GetSize()							{	return Type2<u16>( QWidget::size().width(), QWidget::size().height() );	}
	virtual void			SetPosition(const Type2<u16>& Pos)	{	QWidget::move( Pos.x, Pos.y );	}
	virtual Type2<u16>		GetPosition() const					{	return Type2<u16>( QWidget::pos().x(), QWidget::pos().y() );	}

	FORCEINLINE QBoxLayout&	GetLayout()							{	return m_Layout;	}

	virtual QMenu*			AllocMenu(const TLAsset::TMenu& Menu);	

#if defined(TL_TARGET_PC)
	virtual void*			GetHandle() const					{	return QMainWindow::winId();	}
#endif

protected:
	virtual QWidget&					GetWidget()				{	return *this;	}
	virtual TLMessaging::TPublisher&	GetPublisher()			{	return *this;	}
	virtual void						BindAction(QAction& Action);
	
	virtual void			OnAddedChild(TLGui::TControl& Control);	//	add child to layout
	virtual TMenuHandler*	GetMenuHandler()					{	return this;	}	//	get the menu handler for the window.

	virtual void			mousePressEvent(QMouseEvent *event);	//	catch mouse movement on the window
	virtual void			mouseReleaseEvent(QMouseEvent *event);	//	catch mouse movement on the window
	virtual void			mouseMoveEvent(QMouseEvent *event);		//	catch mouse movement on the window

	bool					OnMouseInput(Qt::MouseButton Button,QMouseEvent& Event,SyncBool ButtonDown);	//	handle mouse input

private slots:
	void					OnAction()				{	TMenuHandler::Slot_OnAction(static_cast<QAction*>( sender() ) );	}
	
private:
	QVBoxLayout					m_Layout;			//	layout doesn't apply to a main window
	TKeyArray<TRef,QMenu*>		m_Menus;			//	root menu items we've added
	TKeyArray<TRef,QAction*>	m_MenuItemActions;	//	action for each menu item - need to store which menu the item is from?
};


