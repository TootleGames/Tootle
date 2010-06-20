/*------------------------------------------------------
	
	A manager is a component of the runtime game, it is
	owned by the App (the engine)

-------------------------------------------------------*/
#pragma once

#include "TRelay.h"


namespace TLManager
{
	// Manager states
	typedef enum ManagerState
	{
		S_None = 0,
		S_Initialising,
		S_Ready,
		S_ShuttingDown,
		S_Shutdown,
		S_Error,
	};
}

namespace TLCore
{
	class TManager;
}


// Generic manager class
class TLCore::TManager : public TLMessaging::TRelay
{
public:
	TManager(TRefRef ManagerRef) :
	  m_ManagerRef		( ManagerRef ),
	  m_ManagerState	( TLManager::S_None )
	{
	}

	virtual ~TManager()
	{
	}

	virtual TRefRef						GetSubscriberRef() const				{	return GetManagerRef();	}
	virtual TRefRef						GetPublisherRef() const					{	return GetManagerRef();	}
	FORCEINLINE TRefRef					GetManagerRef() const					{	return m_ManagerRef;	}
	FORCEINLINE TLManager::ManagerState	GetState() const						{	return m_ManagerState;	}

	// Messaging
	virtual Bool				SendMessageTo(TRefRef RecipientRef, TLMessaging::TMessage& Message) 
	{ 
		// If the recipient is the graph then process the message
		if(RecipientRef == GetManagerRef())
		{
			ProcessMessage(Message);
			return TRUE;
		}

		return FALSE; 
	}

	FORCEINLINE Bool			operator<(const TManager& Manager)const				{	return (GetManagerRef() < Manager.GetManagerRef());	}
	FORCEINLINE Bool			operator==(const TPtr<TManager>* ppManager)const	{	const TManager* pThat = ppManager ? (*ppManager).GetObjectPointer() : NULL;	return (this == pThat);	}
	FORCEINLINE Bool			operator==(const TRef& Ref) const					{	return (m_ManagerRef == Ref);	}

protected:
	virtual SyncBool Initialise()					{	return SyncTrue; 	}
	virtual SyncBool Shutdown()						{ 	return SyncTrue;	}
	virtual SyncBool Update(float /*fTimeStep*/)	{	return SyncTrue;	}

	virtual void	ProcessMessage(TLMessaging::TMessage& Message);

	void			SetState(TLManager::ManagerState NewState);
	
	virtual void	OnEventChannelAdded(TRefRef PublisherRef, TRefRef ChannelRef);

	virtual void					SetProperty(TLMessaging::TMessage& Message) {}			//	base manager SetProperty that can be used without having a pointer to the manager by using the messaging system via the coremanager
	virtual void					GetProperty(TLMessaging::TMessage& Message, TLMessaging::TMessage& Response) {}	//	base manager GetProperty message handler

private:
	TRef					m_ManagerRef;	// Manager unique ID
	TLManager::ManagerState	m_ManagerState; // Simple manager state handling (state machine would be OTT and involve calling an update every frame
};