/*
 *  IPodSocialNetworkingPlatform.h
 *  TootleSocial
 *
 *  Created by Duane Bradbury on 28/08/2009.
 *  Copyright 2009 Tootle. All rights reserved.
 *
 */

#pragma once

namespace TLSocial
{
	namespace Platform
	{
		// Social Networking platform policy None - no social networking	
		class SocialNetworkingPlatform_None
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
				~SocialNetworkingPlatform_None()	{}
			};
	}
}