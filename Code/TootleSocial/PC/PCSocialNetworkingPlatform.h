/*
 *  PCSocialNetworkingPlatform.h
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
		// Social Networking platform policies

		class SocialNetworkingPlatform_None
		{
		public:
			FORCEINLINE Bool CreateSession(TLMessaging::TMessage& InitMessage)									{ return TRUE; }
			FORCEINLINE void DestroySession()																	{}
			
			FORCEINLINE void BeginSession()																		{}
			FORCEINLINE void EndSession()																		{}
			
			FORCEINLINE void OpenDashboard()																	{}
			FORCEINLINE void OpenLeaderboard()																	{}
			
			FORCEINLINE void SubmitScore(const s32& Score, const TString& Format, const s32& LeaderboardID)		{}
			
		protected:
			~SocialNetworkingPlatform_None()	{}
		};

		// On the PC we don't use any social networking platforms yet as there aren't generally any SDK's for them
		class SocialNetworkingPlatform_Facebook : public SocialNetworkingPlatform_None
		{
		};

		class SocialNetworkingPlatform_OpenFeint : public SocialNetworkingPlatform_None
		{
		};

		class SocialNetworkingPlatform_AGONOnline : public SocialNetworkingPlatform_None
		{
		};
	}
}