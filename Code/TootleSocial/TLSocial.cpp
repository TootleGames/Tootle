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
		void BeginSession(TRefRef SessionTypeRef);
		void EndSession(TRefRef SessionTypeRef);
		
		void OpenDashboard(TRefRef SessionTypeRef);
		
		void SubmitScore(TRefRef SessionTypeRef, const s32& Score, const TString& Format, const s32& LeaderboardID);
	}				
}


using namespace TLSocial;

// Begins a social networking session - initiates login dialogue and session object for a given user
Bool TSocialNetworkingManager::BeginSession(TRefRef SessionRef, TRefRef UserRef, TRefRef SessionTypeRef)
{
	if(m_SessionTypeRef.IsValid())
	{
		// Session already active
		return FALSE;
	}
	
	if(SessionTypeRef != TLSocial::FacebookRef && 
		SessionTypeRef != TLSocial::OpenFeintRef && 
		SessionTypeRef != TLSocial::AGONOnlineRef)
	{
		// Invalid type
		return FALSE;
	}

	m_SessionTypeRef = SessionTypeRef;
		
	Platform::BeginSession(m_SessionTypeRef);
	
	return TRUE;
}

// Ends a session for a user - logs out of social network
Bool TSocialNetworkingManager::EndSession(TRefRef SessionRef)
{
	if(!m_SessionTypeRef.IsValid())
		return FALSE;

	Platform::EndSession(m_SessionTypeRef);
	
	m_SessionTypeRef.SetInvalid();
	
	return TRUE;
}


// Opens social network platform dashboard
Bool TSocialNetworkingManager::OpenDashboard()
{
	if(!m_SessionTypeRef.IsValid())
		return FALSE;

	Platform::OpenDashboard(m_SessionTypeRef);
	
	return TRUE;
}


Bool TSocialNetworkingManager::SubmitScore(const s32& Score, const TString& Format, TRefRef LeaderboardRef)
{
	if(!m_SessionTypeRef.IsValid())
		return FALSE;
	
	// Get the ID of the leaderboard from the tref
	s32* pLeaderboardID = m_Leaderboards.Find(LeaderboardRef);

	// LeaderboardID exists?
	if(!pLeaderboardID)
		return FALSE;
	
	return SubmitScore(Score, Format, *pLeaderboardID);
}


Bool TSocialNetworkingManager::SubmitScore(const s32& Score, const TString& Format, const s32& LeaderboardID)
{
	
	Platform::SubmitScore(m_SessionTypeRef, Score, Format, LeaderboardID);
	
	return TRUE;
}
