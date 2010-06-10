/*
 *  TSocialNetworkingManager.h
 *  TootleSocial
 *
 *	The Social Networking Manager handles requests to the social networking systems
 *	such as facebook, twitter, AGON, OpenFeint etc.
 *
 *	At the moment this uses one exclusive system dictated by the social platform
 *	type in use.
 *
 *	Long term we should be able to have multiple social netwoking platforms working together
 *	where possible.  We should be able to have twitter and facebook sessions at the same time for instance.
 *	We may not be able to mix some social networking platforms however eg. AGON and OpenFeint.
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
}


//	include the platform specific header
#include PLATFORMHEADER(SocialnetworkingPlatform.h)

// Etra headers for the IPod and IPad
#if defined(TL_TARGET_IPOD)
	#include "IPod/IPodSocialnetworkingPlatform_Facebook.h"
	#include "IPod/IPodSocialNetworkingPlatform_OpenFeint.h"
	#include "IPod/IPodSocialNetworkingPlatform_AGONOnline.h"
	#include "IPod/IPodSocialnetworkingPlatform_GameCenter.h"
#elif defined(TL_TARGET_IPAD)
	#include "IPad/IPadSocialnetworkingPlatform_Facebook.h"
	#include "IPad/IPadSocialNetworkingPlatform_OpenFeint.h"
	#include "IPad/IPadSocialNetworkingPlatform_AGONOnline.h"
	#include "IPad/IPadSocialnetworkingPlatform_GameCenter.h"
#endif




template<class SocialNetworkingPlatform>
class TLSocial::TSocialNetworkingManager : public TLCore::TManager, public SocialNetworkingPlatform
{
public:
	TSocialNetworkingManager(TRefRef ManagerRef);
	~TSocialNetworkingManager();
	
	// Session management
	Bool		CreateSession(TRefRef SessionRef, TRefRef UserRef, TRefRef SessionTypeRef, TLMessaging::TMessage& InitMessage);
	Bool		DestroySession(TRefRef SessionRef);

	Bool		BeginSession(TRefRef SessionRef);
	Bool		EndSession(TRefRef SessionRef);
	
	Bool		OpenDashboard();
	Bool		OpenLeaderboard();
	
	// Leaderboard processing
	FORCEINLINE Bool		RegisterLeaderboard(TRefRef LeaderboardRef, const s32& LeaderboardID)	{ return m_Leaderboards.Add(LeaderboardRef, LeaderboardID)!=NULL; }
	
	Bool		SubmitScore(const s32& Score, const TString& Format, TRefRef LeaderboardRef);
	
	// Achievement processing
	
	// Accessors
//	FORCEINLINE const TString&		GetKey()					const	{ return m_Key; }	
//	FORCEINLINE const TString&		GetSecret()					const	{ return m_Secret; }
	
protected:

	virtual SyncBool	Shutdown();
	
private:
	
	
	Bool		SubmitScore(const s32& Score, const TString& Format, const s32& LeaderboardID);
	
	
	//class TSession;
	
public:
	typedef TSocialNetworkingManager<SocialNetworkingPlatform> SocialNetworkingManager;
	
private:
	
	//TArray<TSession>	m_Sessions;	// Array of sessions - multiple user login? May only want one?
	
	TKeyArray<TRef, s32>	m_Leaderboards;
	
	TRef				m_SessionTypeRef;	// Type of session	
};


template<class SocialNetworkingPlatform>
TLSocial::TSocialNetworkingManager<SocialNetworkingPlatform>::TSocialNetworkingManager(TRefRef ManagerRef) :
TLCore::TManager		(ManagerRef)
{
}


template<class SocialNetworkingPlatform>
TLSocial::TSocialNetworkingManager<SocialNetworkingPlatform>::~TSocialNetworkingManager()
{
	// Last resort - destroy the social networking session if still active
	// Shouldn;t really be valid at this point as the session should be destroyed either
	// via the game of via the shutdown of the manager
	if(m_SessionTypeRef.IsValid())
	{
		TLDebug_Break("Social netowrking session still active");
		DestroySession(TRef());
	}
}

template<class SocialNetworkingPlatform>
SyncBool TLSocial::TSocialNetworkingManager<SocialNetworkingPlatform>::Shutdown()
{
	// Destroy the social networking session if still active
	if(m_SessionTypeRef.IsValid())
	{
		DestroySession(TRef());
	}
	
	return TManager::Shutdown();
}


// Begins a social networking session - initiates login dialogue and session object for a given user
template<class SocialNetworkingPlatform>
Bool TLSocial::TSocialNetworkingManager<SocialNetworkingPlatform>::CreateSession(TRefRef SessionRef, TRefRef UserRef, TRefRef SessionTypeRef, TLMessaging::TMessage& InitMessage)
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
	
	
	// Social Networking platform policy begin session
	if(!SocialNetworkingPlatform::CreateSession(InitMessage))
		return FALSE;

	// Session created
	m_SessionTypeRef = SessionTypeRef;
		
	//m_Key = APIKey;
	//m_Secret = APISecret;
	return TRUE;
}


template<class SocialNetworkingPlatform>
Bool TLSocial::TSocialNetworkingManager<SocialNetworkingPlatform>::BeginSession(TRefRef SessionRef)
{
	if(!m_SessionTypeRef.IsValid())
	{
		// Session already active
		return FALSE;
	}
	
	
	// Social Networking platform policy begin session
	SocialNetworkingPlatform::BeginSession();
		
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
	
	return TRUE;
}


template<class SocialNetworkingPlatform>
Bool TLSocial::TSocialNetworkingManager<SocialNetworkingPlatform>::DestroySession(TRefRef SessionRef)
{
	if(!m_SessionTypeRef.IsValid())
		return FALSE;
	
	// Social Networking platform policy begin session
	SocialNetworkingPlatform::DestroySession();
	
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