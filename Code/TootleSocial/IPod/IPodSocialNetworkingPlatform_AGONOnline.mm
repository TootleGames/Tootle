/*
 *  IPodSocialNetworkingPlatform_AGONOnline.mm
 *  TootleSocial
 *
 *  Created by Duane Bradbury on 28/08/2009.
 *  Copyright 2009 Tootle. All rights reserved.
 *
 */

#import "IPodSocialNetworkingPlatform_AGONOnline.h"
#import <TootleCore/IPod/IPodString.h>

#import "TAGONOnline.h"

#include "AGON.h"

void TLSocial::Platform::SocialNetworkingPlatform_AGONOnline::BeginSession(const TString& APIKey, const TString& APISecret)
{
	// Enable logging from inside of AGON.
#ifdef _DEBUG	
	AgonShowLogs(YES);
#endif
	
	//NSString* pAPIKey = [NSString stringWithUTF8String:APIKey.GetData()];
	NSString* pAPISecret = TLString::ConvertToUnicharString(APISecret);
	
	// Test App Secret - @"F6F99887E71B210A7DE512050EAD22894B72952A"
	
	AgonCreate(pAPISecret); // App secret that matches the DevDB environment.
	
	[pAPISecret release];
	
	// Tint AGON backgrounds to match app
	// NOTE: Seaman Count specific atm
	AgonSetStartBackgroundTint([UIColor colorWithRed:125/255.0 green:208/255.0 blue:1 alpha:1]);
	AgonSetEndBackgroundTint([UIColor colorWithRed:0/255.0 green:76/255.0 blue:120/255.0 alpha:1]);
	
}


void TLSocial::Platform::SocialNetworkingPlatform_AGONOnline::EndSession()
{
	AgonDestroy();
}

void TLSocial::Platform::SocialNetworkingPlatform_AGONOnline::OpenDashboard()
{
	AgonShow(nil, nil, AgonBladeProfile);
}

void TLSocial::Platform::SocialNetworkingPlatform_AGONOnline::OpenLeaderboard()												
{
	AgonShow(nil, nil, AgonBladeLeaderboards);	
}

void TLSocial::Platform::SocialNetworkingPlatform_AGONOnline::SubmitScore(const s32& Score, const TString& Format, const s32& LeaderboardID)	
{
	// Covert our TString into an NSString
	NSString* pFormatString = TLString::ConvertToUnicharString(Format);
	
	AgonSubmitScore(Score, pFormatString, LeaderboardID);
	
	[pFormatString release];
}
