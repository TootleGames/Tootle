/*
 *  IPodSocialNetworkingPlatform_OpenFeint.mm
 *  TootleSocial
 *
 *  Created by Duane Bradbury on 28/08/2009.
 *  Copyright 2009 Tootle. All rights reserved.
 *
 */

#import "IPodSocialNetworkingPlatform_OpenFeint.h"

#import "TOpenFeint.h"

#import "OFHighScoreService.h"
#import "OpenFeint.h"
#import "OFViewHelper.h"

void TLSocial::Platform::SocialNetworkingPlatform_OpenFeint::BeginSession(const TString& APIKey, const TString& APISecret)
{
	NSString* pAPIKey = [NSString stringWithUTF8String:APIKey.GetData()];
	NSString* pAPISecret = [NSString stringWithUTF8String:APISecret.GetData()];
	
	[OpenFeint initializeWithProductKey: pAPIKey
							  andSecret: pAPISecret
						 andDisplayName:@"TestGame" 
							andSettings:nil 
							andDelegate:nil];
		
}


void TLSocial::Platform::SocialNetworkingPlatform_OpenFeint::EndSession()
{
	[OpenFeint shutdown];
}

void TLSocial::Platform::SocialNetworkingPlatform_OpenFeint::OpenDashboard()
{
	[OpenFeint launchDashboard];
}

void TLSocial::Platform::SocialNetworkingPlatform_OpenFeint::OpenLeaderboard()												
{
}

void TLSocial::Platform::SocialNetworkingPlatform_OpenFeint::SubmitScore(const s32& Score, const TString& Format, const s32& LeaderboardID)	
{
	
	NSString* pFormattedLeaderboard = [NSString stringWithFormat:@"%d", LeaderboardID];
	
	[OFHighScoreService	setHighScore:Score 
					  forLeaderboard:pFormattedLeaderboard
						   onSuccess:OFDelegate() 
						   onFailure:OFDelegate()];
		
}
