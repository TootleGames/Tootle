#pragma once
#include "TLGui.h"
#include <TootleCore/TSubscriber.h>


namespace TLAsset
{
	class TMenu;
}

namespace TLMenu
{
	class TMenuController;
}



//------------------------------------------------------
//	manages link between a menu controller and a menu attached to a window
//------------------------------------------------------
class TLGui::TMenuWrapper : public TLMessaging::TSubscriber
{
public:
	TMenuWrapper(TLMenu::TMenuController& MenuController,TLGui::TMenuHandler& MenuHandler,bool PopupMenu);
	~TMenuWrapper();
	
	TMenuHandler&				GetMenuHandler()	{	return *m_pMenuHandler;	}
	TLMenu::TMenuController&	GetMenuController()	{	return *m_pMenuController;	}
	
protected:
	virtual TRefRef				GetSubscriberRef() const			{	static TRef Ref("MenuWrapper");	return Ref;	}
	virtual void				ProcessMessage(TLMessaging::TMessage& Message);
	
private:
	TRef						m_MenuRef;			//	ref for the window
	TMenuHandler*				m_pMenuHandler;		//	window  the menu is on, never null
	TLMenu::TMenuController*	m_pMenuController;	//	owner menu controller, never null
};


class TLGui::TMenuHandler
{
public:
	virtual TRef						CreateMenu(const TLAsset::TMenu& MenuAsset,bool PopupMenu)=0;	//	create a menu from this menu asset
	virtual void						DestroyMenu(TRefRef MenuRef)=0;					//	delete a menu	
	virtual TLMessaging::TPublisher&	GetPublisher()=0;
};	

