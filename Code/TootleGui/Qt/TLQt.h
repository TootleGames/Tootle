/*------------------------------------------------------
 
	wrapper to include wx
 
 -------------------------------------------------------*/
#pragma once
#if !defined(TL_ENABLE_QT)
#error Should only be built in QT only build
#endif // TL_ENABLE_QT

#undef QT_FATAL_ASSERT

#include <QtGui/QApplication>
#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QHeaderView>
#include <QtGui/QMainWindow>
#include <QtGui/QMenuBar>
#include <QtGui/QStatusBar>
#include <QtGui/QToolBar>
#include <QtGui/QWidget>
#include <QtGui/QDesktopWidget>
#include <TootleCore/TKeyArray.h>
#include "../TWindow.h"
#include "../TMenu.h"

TLCompileAssert( sizeof(TChar) == sizeof(QChar), "Expected QT char and T char to be the same size. If not, the string conversion code needs changing." )

namespace TLGui
{
	namespace Platform
	{
		SyncBool		Init();
		SyncBool		Shutdown();

		void			GetDesktopSize(Type4<s32>& DesktopSize);	//	get the desktop dimensions. note: need a window so we can decide which desktop?	
	}
}


namespace Qt
{
	FORCEINLINE QString		GetString(const TString& String);								//	Get QT string from our own string
	bool					GetData(TBinary& Data,const QVariant& Value,TRefRef DataType);	//	convert QT variant data to our own format. if DataType specified then explicitly convert to that type
	bool					InitMenu(QMenu& Menu,TKeyArray<TRef,QAction*>& MenuActionItems,const TLAsset::TMenu& MenuAsset);	//	set-up a QMenu item's from a menu asset
	int						Main(const TString& Params);		//	common main-like entry

	class TWidgetWrapper;	//	all controls must inherit from this so we can access their QWidget
	class TMenuHandler;		//	inherit from one of these to add generic menu functionality to your widget
}


//---------------------------------------------------
//	all controls must inherit from this so we can access their QWidget
//---------------------------------------------------
class Qt::TWidgetWrapper
{
public:
	virtual QWidget&		GetWidget()=0;		//	access the widget
};

//---------------------------------------------------
//	Qt menu handler - works for popup menus and menu bars (QMenu is all of these)
//---------------------------------------------------
class Qt::TMenuHandler : public TLGui::TMenuHandler
{
protected:
	virtual TRef				CreateMenu(const TLAsset::TMenu& MenuAsset,bool PopupMenu);	//	create a menu from this menu asset
	virtual void				DestroyMenu(TRefRef MenuRef);							//	delete a menu
	//QMenu*					GetMenu(TRefRef MenuRef)								{	QMenu** ppMenu = m_Menus.Find( MenuRef );	return ppMenu ? *ppMenu : NULL;	}
	
	QMenu*								AllocPopupMenu();								//	create a qt popup menu owned by this widget
	
protected:
	virtual QWidget&					GetWidget()=0;
//	virtual TLMessaging::TPublisher&	GetPublisher()=0;
	virtual void						BindAction(QAction& Action)=0;	//	bind this action - must be done by widget based object
	virtual QMenu*						AllocMenu(const TLAsset::TMenu& Menu)=0;
	
protected:// slots:
	virtual void				Slot_OnAction(QAction* pAction);		//	for derived types to use for a slot
	
private:
	TKeyArray<TRef,QMenu*>		m_Menus;			//	root menu's for the handler
	TKeyArray<TRef,QAction*>	m_MenuItemActions;	//	action for each menu item - need to store which menu the item is from?
};

//---------------------------------------------------
//	convert TString to QString
//---------------------------------------------------
FORCEINLINE QString Qt::GetString(const TString& String)
{
	//	construct as unicode string
	return QString( (QChar*)String.GetData(), String.GetLength() );
}


//---------------------------------------------------
//	append QString to TString
//---------------------------------------------------
template<>					
FORCEINLINE TString& operator<<(TString& String,const QString& That)
{
	String << (TChar*)That.constData();
	return String;
}

