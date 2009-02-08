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

namespace TLScheme
{
	class TSchemeManager;
	
	extern TPtr<TSchemeManager> g_pSchemeManager;
}


class TLScheme::TSchemeManager : public TManager
{
	typedef enum TUpdateType
	{
		Load = 0,
		UnLoad,
	};
	
public:	
	
	Bool IsSchemeLoaded(TRefRef SchemeRef);
	
	Bool RequestLoadScheme(TRefRef SchemeRef); 
	Bool RequestUnloadScheme(TRefRef SchemeRef);
	
protected:
	virtual SyncBool Initialise(); 
	virtual SyncBool Shutdown();
	virtual SyncBool Update(float /*fTimeStep*/);		
	
	Bool RequestUpdateScheme(TRefRef SchemeRef, TUpdateType UpdateRequestType);
	
	// TUpdateRequest class
	class TUpdateRequest
	{
		TUpdateType m_Type;
	};

private:

	TArray<TRef>				m_Schemes;		// List of instanced schemes
	TArray<TUpdateRequest>		m_UpdateRequests;
};