/*
 *  IPodSocialNetworkingPlatform_OpenFeint.mm
 *  TootleSocial
 *
 *  Created by Duane Bradbury on 28/08/2009.
 *  Copyright 2009 Tootle. All rights reserved.
 *
 */

#import "IPodSocialNetworkingPlatform_OpenFeint.h"

#import <TootleCore/IPod/IPodString.h>

#import "OFHighScoreService.h"
#import "OpenFeint.h"
#import "OFViewHelper.h"

#import "../TLSocial.h"

Bool TLSocial::Platform::SocialNetworkingPlatform_OpenFeint::CreateSession(TLMessaging::TMessage& InitMessage)
{
	TTempString APIKey;
	
	if(!InitMessage.ImportDataString(TLSocial::APIKeyRef, APIKey))
	{
		TLDebug_Break("Failed to import API key");
		return FALSE;
	}
	
	TTempString APISecret;
	
	if(!InitMessage.ImportDataString(TLSocial::APISecretRef, APISecret))
	{
		TLDebug_Break("Failed to import API secret");
		return FALSE;
	}
	
	TTempString AppName;
	
	if(!InitMessage.ImportDataString("AppName", AppName))
	{
		TLDebug_Break("Failed to import app name");
		return FALSE;
	}
	
	
	
	NSString* pAPIKey = TLString::ConvertToUnicharString(APIKey);
	NSString* pAPISecret = TLString::ConvertToUnicharString(APISecret);
	NSString* pAppName = TLString::ConvertToUnicharString(AppName);
	
	[OpenFeint initializeWithProductKey: pAPIKey
							  andSecret: pAPISecret
						 andDisplayName: pAppName 
							andSettings:nil 
							andDelegate:nil];
		
	[pAPISecret release];
	[pAPIKey release];
	[pAppName release];
	
	return TRUE;
}

void TLSocial::Platform::SocialNetworkingPlatform_OpenFeint::BeginSession()
{
}

void TLSocial::Platform::SocialNetworkingPlatform_OpenFeint::EndSession()
{
}

void TLSocial::Platform::SocialNetworkingPlatform_OpenFeint::DestroySession()
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
