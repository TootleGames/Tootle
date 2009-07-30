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
#include <TootleCore/TKeyArray.h>

namespace TLSocial
{
	class TSocialNetworkingManager;
		
	extern TPtr<TSocialNetworkingManager> g_pSocialNetworkingManager;
	
	// Supported social networking platforms
	const TRef FacebookRef			= TRef_Static(F,a,c,e,b);	//"Facebook";
//	const TRef TwitterRef			= TRef_Static(T,w,i,t,t);	//"Twitter";
	const TRef OpenFeintRef			= TRef_Static(O,p,e,n,F);	//"OpenFeint";
	const TRef AGONOnlineRef		= TRef_Static(A,G,O,N,O);	//"AGONOnline";
	
	
}


class TLSocial::TSocialNetworkingManager : public TLCore::TManager
{
public:
	TSocialNetworkingManager(TRefRef ManagerRef) :
		TLCore::TManager		(ManagerRef)
	{
	}
	
	// Session management
	Bool		BeginSession(TRefRef SessionRef, TRefRef UserRef, TRefRef SessionTypeRef);
	Bool		EndSession(TRefRef SessionRef);
	
	Bool		OpenDashboard();

	// Leaderboard processing
	FORCEINLINE Bool		RegisterLeaderboard(TRefRef LeaderboardRef, const s32& LeaderboardID)	{ return m_Leaderboards.Add(LeaderboardRef, LeaderboardID); }
	
	Bool		SubmitScore(const s32& Score, const TString& Format, TRefRef LeaderboardRef);

	// Achievement processing
	
	// Accessors
	FORCEINLINE const TString&		GetKey()					const	{ return m_Key; }
	FORCEINLINE void				SetKey(const TString& key)			{ m_Key = key; }
	
	FORCEINLINE const TString&		GetSecret()					const	{ return m_Secret; }
	FORCEINLINE void				SetSecret(const TString& secret)	{ m_Secret = secret; }

private:
	
	
	Bool		SubmitScore(const s32& Score, const TString& Format, const s32& LeaderboardID);
	
	
	class TSession;
	
private:
	TArray<TSession>	m_Sessions;	// Array of sessions - multiple user login? May only want one?
	
	TKeyArray<TRef, s32>	m_Leaderboards;
	
	TRef				m_SessionTypeRef;	// Type of session

	// Common social networking platform data
	TString				m_Key;
	TString				m_Secret;
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