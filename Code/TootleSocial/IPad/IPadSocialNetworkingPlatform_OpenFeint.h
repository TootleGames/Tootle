/*
 *  IPadSocialNetworkingPlatform_OpenFeint.h
 *  TootleSocial
 *
 *  Created by Duane Bradbury on 28/08/2009.
 *  Copyright 2009 Tootle. All rights reserved.
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
		// Social Networking platform policy OpenFeint	
		class SocialNetworkingPlatform_OpenFeint
		{
		public:
			Bool CreateSession(TLMessaging::TMessage& InitMessage);
			void DestroySession();
			
			void BeginSession();
			void EndSession();
			
			void OpenDashboard();
			void OpenLeaderboard();
			
			void SubmitScore(const s32& Score, const TString& Format, const s32& LeaderboardID);
			
		protected:
			SocialNetworkingPlatform_OpenFeint()	{}
		};
		
	}
}




