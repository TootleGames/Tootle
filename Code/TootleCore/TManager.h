

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


// Generic manager class
class TManager : public TLMessaging::TRelay
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

	FORCEINLINE TRefRef					GetManagerRef() const					{	return m_ManagerRef;	}
	FORCEINLINE TLManager::ManagerState	GetState() const						{	return m_ManagerState;	}
	FORCEINLINE Bool					operator<(const TManager& Manager)const	{	return (GetManagerRef() < Manager.GetManagerRef());	}

protected:
	virtual SyncBool Initialise() 
	{	
		return SyncTrue; 
	}

	virtual SyncBool Shutdown()
	{ 
		return SyncTrue; 
	}

	virtual SyncBool Update(float /*fTimeStep*/)		
	{
		return SyncTrue;
	}

	virtual void	ProcessMessage(TPtr<TLMessaging::TMessage>& pMessage);

	void			SetState(TLManager::ManagerState NewState);
	
	virtual void	OnEventChannelAdded(TRefRef refPublisherID, TRefRef refChannelID);

private:
	TRef					m_ManagerRef;	// Manager unique ID
	TLManager::ManagerState	m_ManagerState; // Simple manager state handling (state machine would be OTT and involve calling an update every frame
};