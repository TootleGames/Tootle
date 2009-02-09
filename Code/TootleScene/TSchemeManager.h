/*
 *  TSchemeManager.h
 *  TootleScene
 *
 *	The scheme manager is for loading/unloading schemes an keeping track of what primary schemes are loaded.
 *
 *
 *  Created by Duane Bradbury on 06/02/2009.
 *  Copyright 2009 Tootle. All rights reserved.
 *
 */

#pragma once

#include <TootleCore/TManager.h>
#include <TootleCore/TStateMachine.h>

namespace TLScheme
{
	class TSchemeManager;
	
	extern TPtr<TSchemeManager> g_pSchemeManager;
}


class TLScheme::TSchemeManager : public TManager
{
	enum TSchemeUpdateType
	{
		Load = 0,
		UnLoad,
	};
	
	enum TSchemeState
	{
		Loading = 0,
		Loaded,
		UnLoading,
		UnLoaded,
	};
	
	
public:	
	
	TSchemeManager(TRefRef ManagerRef) :
		TManager( ManagerRef )
	{
	}

	
	FORCEINLINE Bool IsSchemeLoaded(TRefRef SchemeRef)			 { return IsSchemeInState(SchemeRef, Loaded); }
	FORCEINLINE Bool IsSchemeLoading(TRefRef SchemeRef)			 { return IsSchemeInState(SchemeRef, Loading); }
	FORCEINLINE Bool IsSchemeUnloading(TRefRef SchemeRef)		 { return IsSchemeInState(SchemeRef, UnLoading); }
	FORCEINLINE Bool IsSchemeUnLoaded(TRefRef SchemeRef)		 { return IsSchemeInState(SchemeRef, UnLoaded); }
	
	FORCEINLINE Bool RequestLoadScheme(TRefRef SchemeRef, TRefRef SchemeAssetRef)		 { return RequestUpdateScheme(SchemeRef, SchemeAssetRef, Load); }
	FORCEINLINE Bool RequestUnloadScheme(TRefRef SchemeRef, TRefRef SchemeAssetRef)		 { return RequestUpdateScheme(SchemeRef, SchemeAssetRef, UnLoad); }
	
protected:
	virtual SyncBool Initialise(); 
	virtual SyncBool Shutdown();
	virtual SyncBool Update(float /*fTimeStep*/);	
	
	virtual void	ProcessMessage(TPtr<TLMessaging::TMessage>& pMessage);

	virtual void	OnEventChannelAdded(TRefRef refPublisherID, TRefRef refChannelID);

	void		UnloadAllSchemes();
	
	// Scheme state checking
	Bool		IsSchemeInState(TRefRef SchemeRef, TSchemeState SchemeState);

	// Scheme update request
	Bool		RequestUpdateScheme(TRefRef SchemeRef, TRefRef SchemeAssetRef, TSchemeUpdateType UpdateRequestType);

	// inernal TSchemeUpdateRequest class
	class TSchemeUpdateRequest;

private:

	TArray<TRef>						m_Schemes;					// List of instanced schemes
	TPtrArray<TSchemeUpdateRequest>		m_SchemeUpdateRequests;		// Scheme load/unload requests
	TArray<TRef>						m_SchemeUpdateRemove;		// List of update requests to remove
};

// TSchemeUpdateRequest class
class TLScheme::TSchemeManager::TSchemeUpdateRequest : public TStateMachine, public TPublisher
{	
public:
	TSchemeUpdateRequest(TRefRef SchemeRef, TRefRef SchemeAssetRef, TSchemeUpdateType UpdateType);
	
	TRefRef				GetSchemeRef()			const	{ return m_SchemeRef;}
	TRefRef				GetSchemeAssetRef()		const	{ return m_SchemeAssetRef;}
	TSchemeUpdateType	GetUpdateType()			const	{ return m_Type; }
		
	inline Bool		operator==(const TRefRef SchemeRef) const	{	return (m_SchemeRef == SchemeRef);	}

private:
	TRef				m_SchemeRef;			// SchemeRef
	TRef				m_SchemeAssetRef;		// Asset of scheme
	TSchemeUpdateType	m_Type;					// Load or unload
	
	// Scheme update state modes
	class TSchemeState_Init;		
	class TSchemeState_Loading;		
	class TSchemeState_UnLoading;		
	class TSchemeState_Finished;		
	class TSchemeState_Cancel;
	
};


// Scheme update state modes
class TLScheme::TSchemeManager::TSchemeUpdateRequest::TSchemeState_Init : public TStateMode
{
	virtual TRef			Update();			
};

// Handles loading a scheme
class TLScheme::TSchemeManager::TSchemeUpdateRequest::TSchemeState_Loading : public TStateMode
{
	virtual TRef			Update();			
};

// Handles unloading a scheme
class TLScheme::TSchemeManager::TSchemeUpdateRequest::TSchemeState_UnLoading : public TStateMode
{
	virtual TRef			Update();
};

// Holding state mode for when the scheme update is finished
class TLScheme::TSchemeManager::TSchemeUpdateRequest::TSchemeState_Finished : public TStateMode
{
};

// Handles cancelling a load or unload request
class TLScheme::TSchemeManager::TSchemeUpdateRequest::TSchemeState_Cancel : public TStateMode
{
	virtual TRef			Update();			
};