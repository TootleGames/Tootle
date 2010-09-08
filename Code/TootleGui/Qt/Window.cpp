/*
 *  TWxWindow.cpp
 *  TootleGui
 *
 *  Created by Graham Reeves on 14/01/2010.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */
#include "Window.h"
#include <QtGui/QMenubar>
#include <TootleAsset/TMenu.h>



Qt::TWindow::TWindow(TRefRef WindowRef) : 
	TLGui::TWindow	( WindowRef ),
	QMainWindow		( NULL, Qt::Window ),
	m_Layout		( this )
{
	//	set window title
	TTempString WindowTitle;
	WindowTitle << WindowRef;
	setWindowTitle( Qt::GetString(WindowTitle) );

	//	configure and assign the layout
	GetLayout().setSpacing(0);
	GetLayout().setMargin(0);
//	setLayout( &GetLayout() );
	
	//	capture mouse-move. Need to do this here AND for children we wan't to capture mouse-move on (eg. opengl canvas')
	setMouseTracking(true);
}


QMenu* Qt::TWindow::AllocMenu(const TLAsset::TMenu& Menu)
{
	//	make up menu title from menu ref	
	TTempString MenuTitle;
	MenuTitle << Menu.GetAssetRef();

	//	create the root menu
	QMenu* pWindowMenu = menuBar()->addMenu( Qt::GetString(MenuTitle) );
	return pWindowMenu;
}	


//-------------------------------------------------------
//	add child to layout
//-------------------------------------------------------
void Qt::TWindow::OnAddedChild(TLGui::TControl& Control)
{
	//	need to cast :/
	//	as the control will be multiply inherited, make sure we do a safe cast.
	Qt::TWidgetWrapper* pWidgetControl = dynamic_cast<Qt::TWidgetWrapper*>( &Control );
	if ( !pWidgetControl )
	{
		TLDebug_Break("Failed to cast control to a Qt::TWidgetWrapper - all Qt types must be derived from this!.");
		return;
	}
	
	//	get widget
	QWidget& Widget = pWidgetControl->GetWidget();
	
	/*
	//	temporary until I find the right solution, instead of a layout, just make this widget the central widget.
	if ( m_Controls.GetSize() > 1 )
		TLDebug_Break("Can't handle more than one child atm");
	setCentralWidget( &Widget );
*/
/*	centralWidget()->setLayout( 
	
	QVBoxLayout *vboxLayout = new QVBoxLayout;
	vboxLayout->addWidget( myTextEdit );
	// MainWindow central Widget
	// if central widget already has a layout then:
	centralWidget()->layout()->addLayout( vboxLayout ); 
	// otherwise
	centralWidget()->setLayout( vboxLayout );
*/
	if ( !centralWidget() )
	{
		//	create a dummy central widget
		QWidget* pDummy = new QWidget(this);
		setCentralWidget( pDummy );
		centralWidget()->setLayout( &GetLayout() );
		centralWidget()->setMouseTracking( true );
	}

	//	add widget to the layout
	GetLayout().addWidget( &Widget );
	
	//	the stretch factor is a proportion to cover. 2 gets double the space of 1, 4 is double 2 and 4x more than 1
	//	0 doesn't stretch at all (default)
	GetLayout().setStretchFactor( &Widget, 1 );
}


//-------------------------------------------------------
//	handle mouse input
//-------------------------------------------------------
bool Qt::TWindow::OnMouseInput(Qt::MouseButton Button,QMouseEvent& Event,SyncBool ButtonDown)
{
	//	calc mouse pos
	Type2<u16> MousePos;
	MousePos.x = Event.pos().x();
	MousePos.y = Event.pos().y();

	//	the mouse pos is in window space, we need to get it in client space as in windows this is offset by the window
	//	doesn't seem to be a proper way to do this (aside from mapping to the canvas object, but that's outside of the window scope -
	//	make this input stuff should move to the Screen class? - but the input detection still needs to come from this class
	QWidget* pCentralWidget = this->centralWidget();
	if ( pCentralWidget )
	{
		QRect GeometryRect = pCentralWidget->geometry();
		MousePos.x -= GeometryRect.left();
		MousePos.y -= GeometryRect.top();
	}

	u32 ButtonIndex = 0;
	switch ( Button )
	{
		default:
		case Qt::LeftButton:	ButtonIndex = 0;	break;
		case Qt::RightButton:	ButtonIndex = 1;	break;
		case Qt::MidButton:		ButtonIndex = 2;	break;
	};

	return TLGui::TWindow::OnMouseInput( ButtonIndex, MousePos, ButtonDown ? SyncTrue : SyncFalse );
}



//-------------------------------------------------------
//	catch mouse movement on the window
//-------------------------------------------------------
void Qt::TWindow::mousePressEvent(QMouseEvent *event)
{
	//	do inherited behaviour
	QMainWindow::mousePressEvent( event );

	OnMouseInput( event->button(), *event, SyncTrue );
}


//-------------------------------------------------------
//	catch mouse movement on the window
//-------------------------------------------------------
void Qt::TWindow::mouseReleaseEvent(QMouseEvent *event)
{
	//	do inherited behaviour
	QMainWindow::mouseReleaseEvent( event );
	
	OnMouseInput( event->button(), *event, SyncFalse );
}

//-------------------------------------------------------
//	catch mouse movement on the window
//-------------------------------------------------------
void Qt::TWindow::mouseMoveEvent(QMouseEvent *event)
{
	//	do inherited behaviour
	QMainWindow::mouseMoveEvent( event );

	OnMouseInput( Qt::NoButton, *event, SyncWait );
}


//-------------------------------------------------------
//	bind this action - must be done by widget based object
//-------------------------------------------------------
void Qt::TWindow::BindAction(QAction& Action)
{
	if ( !connect( &Action, SIGNAL(triggered()), this, SLOT(OnAction())) )
	{
		TLDebug_Break("Failed to setup menu handler");
	}
}

