

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
	TManager(TRef refManagerID) :
	  m_refManagerID(refManagerID),
	  m_ManagerState(TLManager::S_None)
	{
	}

	virtual ~TManager()
	{
	}

	inline TRef						GetManagerID()			const	{ return m_refManagerID; }
	inline TLManager::ManagerState	GetState()				const	{ return m_ManagerState; }
	FORCEINLINE Bool				operator<(const TManager& Manager)const	{	return (m_refManagerID < Manager.m_refManagerID);	}

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
	
	virtual void	OnEventChannelAdded(TRef& refPublisherID, TRef& refChannelID);

private:
	TRef					m_refManagerID;	// Manager unique ID
	TLManager::ManagerState	m_ManagerState; // Simple manager state handling (state machine would be OTT and involve calling an update every frame
};