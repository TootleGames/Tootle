/*
 *  IPodSocialNetworkingPlatform_GameCenter.h
 *  TootleSocial
 *
 *  Created by Duane Bradbury on 18/05/2010.
 *  Copyright 2010 Tootle. All rights reserved.
 *
 */

#pragma once

#include <TootleCore/TLTypes.h>
#include <TootleCore/TString.h>
#include <TootleCore/TLMessaging.h>

namespace TLSocial
{
	namespace Platform
	{
		// Social Networking platform policy GameCenter	
		class SocialNetworkingPlatform_GameCenter
		{
		public:
			FORCEINLINE Bool CreateSession(TLMessaging::TMessage& InitMessage)					{ return TRUE; }
			FORCEINLINE void DestroySession()													{}
			
			FORCEINLINE void BeginSession()														{}
			FORCEINLINE void EndSession()														{}
			
			FORCEINLINE void OpenDashboard()													{}
			FORCEINLINE void OpenLeaderboard()													{}
			
			FORCEINLINE void SubmitScore(const s32& Score, const TString& Format, const s32& LeaderboardID)	{}
			
		protected:
			~SocialNetworkingPlatform_GameCenter()	{}
		};
	}
}