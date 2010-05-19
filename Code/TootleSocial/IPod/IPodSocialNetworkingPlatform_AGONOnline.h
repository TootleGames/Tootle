/*
 *  IPodSocialNetworkingPlatform_AGONOnline.h
 *  TootleSocial
 *
 *  Created by Duane Bradbury on 28/08/2009.
 *  Copyright 2009 Tootle. All rights reserved.
 *
 */


// If you use this you will also need to link with the AGON library.
// To do that add the following to your project build settings:
//		Other Linker Flags 
//			Debug	- "-lagond"
//			Release - "-lagon"
//		Library Search Paths
//			All - "../../../Tootle/Code/TootleSocial/IPod/libagon/lib/$(SDK_NAME)"
//
// Additionally you will need to add some bundles to your project resources:
//	AgonData.bundle		- found in the libagon/resources directory
//	AgonPackage.bundle	- project specific and is downloaded from the AGON online developer site.

#pragma once

#include <TootleCore/TLTypes.h>
#include <TootleCore/TString.h>
#include <TootleCore/TLMessaging.h>

namespace TLSocial
{
	namespace Platform
	{
		// Social Networking platform policy AGON Online	
		class SocialNetworkingPlatform_AGONOnline
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
			~SocialNetworkingPlatform_AGONOnline()	{}
		};
	}
}



