

#pragma once

#include <TootleCore/TManager.h>
#include <TootleInput/TUser.h>

//#define TEST_WIDGET_ONE_ACTION

namespace TLGui
{
	class TWidgetManager;

	extern TPtr<TWidgetManager>	g_pWidgetManager;
}


class TLGui::TWidgetManager : public TManager
{

public:
	TWidgetManager(TRefRef ManagerRef) :
		TManager(ManagerRef)
	{
	}

protected:

	virtual void	OnEventChannelAdded(TRefRef refPublisherID, TRefRef refChannelID);
	virtual void	ProcessMessage(TLMessaging::TMessage& Message);	

	void			OnInputDeviceAdded(TRefRef refDeviceID, TRefRef refDeviceType);
	void			OnInputDeviceRemoved(TRefRef refDeviceID, TRefRef refDeviceType);

	// Action mapping
#ifdef TL_TARGET_IPOD
	void			MapDeviceActions_TouchPad(TRefRef refDeviceID, TPtr<TLUser::TUser> pUser);
#else
	void			MapDeviceActions_Mouse(TRefRef refDeviceID, TPtr<TLUser::TUser> pUser);
#endif

	void			MapDeviceActions_Keyboard(TRefRef refDeviceID, TPtr<TLUser::TUser> pUser);

};
