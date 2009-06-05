/*
 *  TLSocial.h
 *  TootleSocial
 *
 *  Created by Duane Bradbury on 04/06/2009.
 *  Copyright 2009 Tootle. All rights reserved.
 *
 */

#pragma once

#include <TootleCore/TManager.h>
#include <TootleCore/TPtr.h>
#include <TootleCore/TArray.h>

namespace TLSocial
{
	class TSocialNetworkingManager;
		
	extern TPtr<TSocialNetworkingManager> g_pSocialNetworkingManager;
	
}


class TLSocial::TSocialNetworkingManager : public TManager
{
public:
	TSocialNetworkingManager(TRefRef ManagerRef) :
		TManager(ManagerRef)
	{
	}
	
	// Session management
	Bool		BeginSession(TRefRef SessionRef, TRefRef UserRef);
	Bool		EndSession(TRefRef SessionRef);
	
private:
	
	class TSession;
	
private:
	TArray<TSession> m_Sessions;	// Array of sessions - multiple user login? May only want one?
};



class TLSocial::TSocialNetworkingManager::TSession
{
public:
	TSession() {}
	
	TSession(TRefRef SessionRef, TRefRef UserRef) :
		m_SessionRef(SessionRef),
		m_UserRef(UserRef)
	{}
	
private:
	TRef		m_SessionRef;
	TRef		m_UserRef;		// user associated with the session
};