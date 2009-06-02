
#pragma once

#include "TManager.h"
#include "TLTime.h"
#include "TLMessaging.h"
#include "TKeyArray.h"
#include "TBinaryTree.h"


namespace TLCore
{
	class TCoreManager;

	extern TPtr<TCoreManager>		g_pCoreManager;
};



// Core manager class - monitors all other managers
class TLCore::TCoreManager : public TManager, public TLMessaging::TMessageQueue
{
public:
	TCoreManager(TRefRef ManagerRef);
	virtual ~TCoreManager();

	SyncBool				InitialiseLoop();	//	do initialisation
	SyncBool				UpdateLoop();		// one process of Main update loop
	SyncBool				UpdateShutdown();	// one process of Main shutdown
	
	// TEMP ROUTINE TO FIX ISSUE OF GLITCH ON INIT ON THE IPOD
	FORCEINLINE void				ForceUpdate()	{	PublishUpdateMessage(TRUE);	}
	FORCEINLINE void				ForceRender()	{	PublishRenderMessage();	}

	template<class T> Bool	CreateAndRegisterManager(TPtr<T>& pManager,TRefRef ManagerRef);	// Creates and registers a manager and assigns the new manager to a global pointer 
	template<class T> Bool	CreateAndRegisterManager(TRefRef ManagerRef);	// Creates and registers a manager but doesn't assign the new manager to any global pointer 
	template<class T> Bool	RegisterManager(TPtr<T>& pManager);				// Simply registers the manager by adding to our list - can also be used to add managers that have been created externally
	Bool					UnRegisterManager(TRefRef ManagerRef);

	// Time step access
	FORCEINLINE Bool		IsReadyForUpdate() const								{	return m_TimerUpdateCount > 0;	}
	FORCEINLINE void		SetReadyForUpdate()										{	m_TimerUpdateCount += IsEnabled() ? 1 : 0;	}
	FORCEINLINE u16			Debug_GetFramesPerSecond() const						{	return m_Debug_FramesPerSecond;	}
	FORCEINLINE float		Debug_GetFrameTimePerSecond() const						{	return m_Debug_FrameTimePerSecond;	}

	FORCEINLINE Bool		AddTimeStepModifier(TRefRef ModiferRef, const float& fValue);
	FORCEINLINE Bool		RemoveTimeStepModifier(TRefRef ModiferRef);
	FORCEINLINE void		RemoveAllTimeStepModifiers();
	FORCEINLINE Bool		HasTimeStepModifier(TRefRef ModiferRef);

	Bool					SetTimeStepModifier(TRefRef ModiferRef, const float& fValue);
	Bool					IncrementTimeStepModifier(TRefRef ModiferRef, const float& fValue = 0.1f);
	Bool					DecrementTimeStepModifier(TRefRef ModiferRef, const float& fValue = 0.1f);

	Bool					GetTimeStepModifier(TRefRef ModiferRef, float& fValue);
	float					GetTimeStepModifier();
	
	// Enable/Disable - should only be called by the lower level hardware interrupt routines
	void					Enable(Bool bEnable);
	FORCEINLINE Bool		IsEnabled() const							{	return m_bEnabled;	}

	
	TRef					GetHardwareLanguage();
	
protected:
	virtual SyncBool		Initialise();
	virtual SyncBool		Update(float fTimeStep);
	virtual SyncBool		Shutdown();

	FORCEINLINE float		GetUpdateTimeStepDifference(const TLTime::TTimestampMicro& TimeNow)		{	return GetTimeStepDifference( m_LastUpdateTime, TimeNow );	}	//	work out timestep for update
	float					GetTimeStepDifference(TLTime::TTimestampMicro& LastTimestamp,const TLTime::TTimestampMicro& TimeNow);	//	work out timestep for update

	virtual void			ProcessMessageFromQueue(TLMessaging::TMessage& Message)			{	ProcessMessage( Message );	}
	virtual void			ProcessMessage(TLMessaging::TMessage& Message);
	void					UnregisterAllManagers();				

	FORCEINLINE void		Debug_UpdateDebugCounters();		//	update our per-second counters to detect X fps etc

private:
	void					PublishInitMessage();						//	publish an init
	Bool					PublishUpdateMessage(Bool bForced = FALSE);	//	publish an update - returns FALSE if we didn't do an update (timestep too small etc)
	void					PublishRenderMessage();						//	publish a render - ALWAYS published
	void					PublishShutdownMessage();					//	publish a shutdown

	Bool					CheckManagersInState(TLManager::ManagerState State);		// Checks the managers to see if they are in the specified state
	Bool					CheckManagersInErrorState();					// Checks the managers to see if any are in the error state

	void					SubscribeAllManagers();
	
private:
	u16							m_TimerUpdateCount;					//	incremented every time the OS wants to trigger an update
	TLTime::TTimestampMicro		m_LastUpdateTime;					//	timestamp of last update
	
	TLTime::TTimestamp			m_Debug_LastCountTime;				//	timestamp last time we recorded FPS/UPS/time counter
	u16							m_Debug_FramesPerSecond;			//	fps counter for prev second
	u16							m_Debug_CurrentFramesPerSecond;		//	fps counter
	float						m_Debug_FrameTimePerSecond;			//	time counter for prev second
	float						m_Debug_CurrentFrameTimePerSecond;	//	time counter

	TPtrArray<TManager>			m_Managers;				// Array of TPtr's to managers
	TArray< TPtr<TManager>* >	m_ManagerPointers;		// Array of pointers to the global TPtr's to managers

	TKeyArray<TRef, float>		m_aTimeStepModifiers;	// Game mechanics time step modifier - allows the game to sped up/slowed down/paused etc
	
	TPtr<TBinaryTree>			m_pMachineData;			// Hardware specific data

	Bool						m_ChannelsInitialised;	//	for the init, we do channel initialisation once
	Bool						m_bEnabled;
	Bool						m_bQuit;
};



//---------------------------------------------------------
// Creates and registers a manager and assigns the new manager to a global pointer 
//---------------------------------------------------------
template<class T>
Bool TLCore::TCoreManager::CreateAndRegisterManager(TPtr<T>& pManager,TRefRef ManagerRef)
{
	//	alloc and register manager
	pManager = new T(ManagerRef);

	if ( !RegisterManager(pManager) )
	{
		pManager = NULL;
		return FALSE;
	}

	// Success - now add the TPtr to our list of pointers to TPtrs so we can clear the TPtr 
	// automatically at shutdown
	TPtr<TManager>* pPtr = (reinterpret_cast< TPtr<TManager>* >(&pManager));
	m_ManagerPointers.Add(pPtr);

	return TRUE;
}

//---------------------------------------------------------
// Creates and registers a manager but doesn't assign the new manager to any global pointer 
//---------------------------------------------------------
template<class T>
Bool TLCore::TCoreManager::CreateAndRegisterManager(TRefRef ManagerRef)
{
	//	alloc new manager
	TPtr<T> pNewManager = new T(ManagerRef);
	if ( !pNewManager )
		return FALSE;
	
	//	register
	return RegisterManager(pNewManager);
}

//---------------------------------------------------------
// Simply registers the manager by adding to our list - can also be used to add managers 
// that have been created externally
//---------------------------------------------------------
template <class T>
Bool TLCore::TCoreManager::RegisterManager(TPtr<T>& pManager)
{
	return (m_Managers.Add(pManager) != -1);
}


//---------------------------------------------------------
//
//---------------------------------------------------------
FORCEINLINE Bool TLCore::TCoreManager::AddTimeStepModifier(TRefRef ModiferRef, const float& fValue)
{
	return m_aTimeStepModifiers.Add(ModiferRef, fValue) != NULL;
}

//---------------------------------------------------------
//
//---------------------------------------------------------
FORCEINLINE Bool TLCore::TCoreManager::RemoveTimeStepModifier(TRefRef ModiferRef)
{
	return m_aTimeStepModifiers.Remove(ModiferRef);
}

//---------------------------------------------------------
//
//---------------------------------------------------------
FORCEINLINE void TLCore::TCoreManager::RemoveAllTimeStepModifiers()
{
	m_aTimeStepModifiers.Empty();
}


//---------------------------------------------------------
//
//---------------------------------------------------------
FORCEINLINE Bool TLCore::TCoreManager::HasTimeStepModifier(TRefRef ModiferRef)
{
	return (m_aTimeStepModifiers.Find(ModiferRef) != NULL);
}


//---------------------------------------------------------
//
//---------------------------------------------------------
FORCEINLINE Bool TLCore::TCoreManager::GetTimeStepModifier(TRefRef ModiferRef, float& fValue)
{
	float* pfValue = m_aTimeStepModifiers.Find(ModiferRef);
	if(!pfValue)
		return FALSE;

	fValue = *pfValue;
	return TRUE;
}

