/*
 *  IPadSocialNetworkingPlatform_AGONOnline.mm
 *  TootleSocial
 *
 *  Created by Duane Bradbury on 28/08/2009.
 *  Copyright 2009 Tootle. All rights reserved.
 *
 */

#import "IPadSocialNetworkingPlatform_AGONOnline.h"
#import <TootleCore/IPad/IPadString.h>
#import "../TLSocial.h"


#include "AGON.h"

Bool TLSocial::Platform::SocialNetworkingPlatform_AGONOnline::CreateSession(TLMessaging::TMessage& InitMessage)
{
	TTempString APISecret;
	
	if(!InitMessage.ImportDataString(TLSocial::APISecretRef, APISecret))
	{
		TLDebug_Break("Failed to import API secret");
		return FALSE;
	}
	
	// Import the server type
	TRef ServerType;
	AgonEnvironment Environment = AgonDeveloperServers;
	
	InitMessage.ImportData("Server", ServerType);
	
	if(ServerType == TRef_Static(P,r,o,d,u))	// Production
		Environment = AgonProductionServers;
	else if(ServerType == TRef_Static(R,e,l,e,a)) // Release
		Environment = AgonReleaseServers;
	//else if(ServerType == TRef_Static(D,e,v,e,l)) // Development (default)
	
	// Enable logging from inside of AGON.
#ifdef _DEBUG	
	AgonShowLogs(YES);
#endif
		
	NSString* pAPISecret = TLString::ConvertToUnicharString(APISecret);
	
	// App secret that matches the DevDB environment.
	// Test App Secret - @"F6F99887E71B210A7DE512050EAD22894B72952A"
	
	//if(AgonCreate(pAPISecret, AgonProductionServers)) 
	if(!AgonCreate(pAPISecret, Environment)) 
	{
		TLDebug_Break("AgonCreate failed");
		return FALSE;
	}
		
			
	[pAPISecret release];
	
	// Tint AGON backgrounds to match app
	// NOTE: Seaman Count specific atm
	AgonSetStartBackgroundTint([UIColor colorWithRed:125.0f/255.0f green:208.0f/255.0f blue:1 alpha:1]);
	AgonSetEndBackgroundTint([UIColor colorWithRed:0.0f/255.0f green:76.0f/255.0f blue:120.0f/255.0f alpha:1]);

	return TRUE;
}

void TLSocial::Platform::SocialNetworkingPlatform_AGONOnline::DestroySession()
{
}


void TLSocial::Platform::SocialNetworkingPlatform_AGONOnline::BeginSession()
{
	AgonStartGameSession();
}


void TLSocial::Platform::SocialNetworkingPlatform_AGONOnline::EndSession()
{
	AgonEndGameSession();
}

void TLSocial::Platform::SocialNetworkingPlatform_AGONOnline::OpenDashboard()
{
	AgonShow(AgonBladeProfile, TRUE );
}

void TLSocial::Platform::SocialNetworkingPlatform_AGONOnline::OpenLeaderboard()												
{
	AgonShow(AgonBladeLeaderboards, TRUE);	
}

void TLSocial::Platform::SocialNetworkingPlatform_AGONOnline::SubmitScore(const s32& Score, const TString& Format, const s32& LeaderboardID)	
{
	// Covert our TString into an NSString
	NSString* pFormatString = TLString::ConvertToUnicharString(Format);
	
	AgonSubmitScore(Score, pFormatString, LeaderboardID);
	
	[pFormatString release];
}
