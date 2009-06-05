/*
 *  TLSocial.cpp
 *  TootleSocial
 *
 *  Created by Duane Bradbury on 04/06/2009.
 *  Copyright 2009 Tootle. All rights reserved.
 *
 */

#include "TLSocial.h"

namespace TLSocial
{
	TPtr<TSocialNetworkingManager> g_pSocialNetworkingManager = NULL;	
	
	namespace Platform
	{
		void BeginSession();
		void EndSession();
	}				
}


using namespace TLSocial;

// Begins a social networking session - initiates login dialogue and session object for a given user
Bool TSocialNetworkingManager::BeginSession(TRefRef SessionRef, TRefRef UserRef)
{
	Platform::BeginSession();
	
	return FALSE;
}

// Ends a session for a user - logs out of social network
Bool TSocialNetworkingManager::EndSession(TRefRef SessionRef)
{
	Platform::EndSession();
	
	return FALSE;
}
