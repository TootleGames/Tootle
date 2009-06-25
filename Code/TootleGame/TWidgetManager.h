

#pragma once

#include <TootleCore/TManager.h>
#include <TootleInput/TUser.h>

namespace TLGui
{
	class TWidgetManager;

	extern TPtr<TWidgetManager>	g_pWidgetManager;
}


class TLGui::TWidgetManager : public TManager
{
private:
	
	class TActionRefData
	{
	public:
		TRef	m_ClickActionRef;
		TRef	m_MoveActionRef;
	};

public:
	TWidgetManager(TRefRef ManagerRef) :
		TManager(ManagerRef)
	{
	}
	
	
	Bool			IsClickActionRef(TRefRef ClickActionRef)
	{
		for(u32 uIndex = 0; uIndex < m_WidgetActionRefs.GetSize(); uIndex++)
		{
			if(m_WidgetActionRefs.ElementAt(uIndex).m_ClickActionRef == ClickActionRef)
				return TRUE;
		}
		return FALSE;
	}
	
	Bool			IsMoveActionRef(TRefRef MoveActionRef)
	{
		for(u32 uIndex = 0; uIndex < m_WidgetActionRefs.GetSize(); uIndex++)
		{
			if(m_WidgetActionRefs.ElementAt(uIndex).m_MoveActionRef == MoveActionRef)
				return TRUE;
		}
		return FALSE;
	}
	
	Bool			IsActionPair(TRefRef ClickActionRef, TRefRef MoveActionRef)
	{
		for(u32 uIndex = 0; uIndex < m_WidgetActionRefs.GetSize(); uIndex++)
		{
			if(m_WidgetActionRefs.ElementAt(uIndex).m_ClickActionRef == ClickActionRef)
			{
				if(m_WidgetActionRefs.ElementAt(uIndex).m_MoveActionRef == MoveActionRef)
					return TRUE;

				// Actions will be unique so if we find the click ref 
				// but move is different then we should not find any further click with the same ref
				// therefore we can return
				return FALSE;
			}
		}
		return FALSE;
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
	
private:
	
	TArray<TActionRefData>	m_WidgetActionRefs;
};
