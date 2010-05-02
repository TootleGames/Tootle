/*
 *  TSocialNetworkingManager.h
 *  TootleSocial
 *
 *  Created by Duane Bradbury on 28/08/2009.
 *  Copyright 2009 Tootle. All rights reserved.
 *
 */

#pragma once

#include <TootleCore/TManager.h>
#include <TootleCore/TPtr.h>
#include <TootleCore/TKeyArray.h>

#include "TLSocial.h"

namespace TLSocial
{
	template<class SocialNetworkingPlatform>
	class TSocialNetworkingManager;
	
	namespace Platform
	{
		void BeginSession(TRefRef SessionTypeRef, const TString& APIKey, const TString& APISecret);
		void EndSession(TRefRef SessionTypeRef);
		
		void OpenDashboard(TRefRef SessionTypeRef);
		void OpenLeaderboard(TRefRef SessionTypeRef);
		
		void SubmitScore(TRefRef SessionTypeRef, const s32& Score, const TString& Format, const s32& LeaderboardID);
	}				
	
}


//	include the platform specific header
#if defined(_MSC_EXTENSIONS)&&defined(TL_TARGET_PC)	//	dont include PC stuff when doing an ANSI build
#include "PC/PCSocialnetworkingPlatform.h"
#endif

#if defined(TL_TARGET_IPOD)
#include "IPod/IPodSocialnetworkingPlatform.h"
#include "IPod/IPodSocialnetworkingPlatform_Facebook.h"
#include "IPod/IPodSocialNetworkingPlatform_OpenFeint.h"
#include "IPod/IPodSocialNetworkingPlatform_AGONOnline.h"
#endif

#if defined(TL_TARGET_MAC)	
#include "Mac/MacSocialnetworkingPlatform.h"
#endif





template<class SocialNetworkingPlatform>
class TLSocial::TSocialNetworkingManager : public TLCore::TManager, public SocialNetworkingPlatform
{
public:
	TSocialNetworkingManager(TRefRef ManagerRef) :
	TLCore::TManager		(ManagerRef)
	{
	}
	
	// Session management
	Bool		BeginSession(TRefRef SessionRef, TRefRef UserRef, TRefRef SessionTypeRef, const TString& APIKey, const TString& APISecret);
	Bool		EndSession(TRefRef SessionRef);
	
	Bool		OpenDashboard();
	Bool		OpenLeaderboard();
	
	// Leaderboard processing
	FORCEINLINE Bool		RegisterLeaderboard(TRefRef LeaderboardRef, const s32& LeaderboardID)	{ return m_Leaderboards.Add(LeaderboardRef, LeaderboardID)!=NULL; }
	
	Bool		SubmitScore(const s32& Score, const TString& Format, TRefRef LeaderboardRef);
	
	// Achievement processing
	
	// Accessors
	FORCEINLINE const TString&		GetKey()					const	{ return m_Key; }	
	FORCEINLINE const TString&		GetSecret()					const	{ return m_Secret; }
	
private:
	
	
	Bool		SubmitScore(const s32& Score, const TString& Format, const s32& LeaderboardID);
	
	
	//class TSession;
	
public:
	typedef TSocialNetworkingManager<SocialNetworkingPlatform> SocialNetworkingManager;
	
private:
	
	//TArray<TSession>	m_Sessions;	// Array of sessions - multiple user login? May only want one?
	
	TKeyArray<TRef, s32>	m_Leaderboards;
	
	TRef				m_SessionTypeRef;	// Type of session
	
	// Common social networking platform data
	TString				m_Key;
	TString				m_Secret;
};



// Begins a social networking session - initiates login dialogue and session object for a given user
template<class SocialNetworkingPlatform>
Bool TLSocial::TSocialNetworkingManager<SocialNetworkingPlatform>::BeginSession(TRefRef SessionRef, TRefRef UserRef, TRefRef SessionTypeRef, const TString& APIKey, const TString& APISecret)
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
	
	m_Key = APIKey;
	m_Secret = APISecret;
	
	// Social Networking platform policy begin session
	SocialNetworkingPlatform::BeginSession(m_Key, m_Secret);
	
	//TLSocial::Platform::BeginSession(m_SessionTypeRef, m_Key, m_Secret);
	
	return TRUE;
}

// Ends a session for a user - logs out of social network
template<class SocialNetworkingPlatform>
Bool TLSocial::TSocialNetworkingManager<SocialNetworkingPlatform>::EndSession(TRefRef SessionRef)
{
	if(!m_SessionTypeRef.IsValid())
		return FALSE;
		
	// Social Networking platform policy begin session
	SocialNetworkingPlatform::EndSession();

	//Platform::EndSession(m_SessionTypeRef);
	
	m_SessionTypeRef.SetInvalid();
	
	return TRUE;
}


// Opens social network platform dashboard
template<class SocialNetworkingPlatform>
Bool TLSocial::TSocialNetworkingManager<SocialNetworkingPlatform>::OpenDashboard()
{
	if(!m_SessionTypeRef.IsValid())
		return FALSE;
	
	SocialNetworkingPlatform::OpenDashboard();
	//Platform::OpenDashboard(m_SessionTypeRef);
	
	return TRUE;
}


// Opens social network platform dashboard
template<class SocialNetworkingPlatform>
Bool TLSocial::TSocialNetworkingManager<SocialNetworkingPlatform>::OpenLeaderboard()
{
	if(!m_SessionTypeRef.IsValid())
		return FALSE;
	
	SocialNetworkingPlatform::OpenLeaderboard();
	//Platform::OpenLeaderboard(m_SessionTypeRef);
	
	return TRUE;
}


template <class SocialNetworkingPlatform>
Bool TLSocial::TSocialNetworkingManager<SocialNetworkingPlatform>::SubmitScore(const s32& Score, const TString& Format, TRefRef LeaderboardRef)
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

template<class SocialNetworkingPlatform>
Bool TLSocial::TSocialNetworkingManager<SocialNetworkingPlatform>::SubmitScore(const s32& Score, const TString& Format, const s32& LeaderboardID)
{
	
	SocialNetworkingPlatform::SubmitScore(Score, Format, LeaderboardID);
	//Platform::SubmitScore(m_SessionTypeRef, Score, Format, LeaderboardID);
	
	return TRUE;
}

/*
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
 */

namespace TLSocial
{
	//extern 	TPtr<SocialNetworkingManager> g_pSocialNetworkingManager;
	
}