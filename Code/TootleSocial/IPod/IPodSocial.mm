/*
 *  IPodSession.mm
 *  TootleSocial
 *
 *  Created by Duane Bradbury on 04/06/2009.
 *  Copyright 2009 Tootle. All rights reserved.
 *
 */

#include "IPodSocial.h"

#include "TFacebookConnect.h"
#include "TAGONOnline.h"
#include "TOpenFeint.h"

#include "../TLSocial.h"


void TLSocial::Platform::BeginSession(TRefRef SessionTypeRef, const TString& APIKey, const TString& APISecret)
{
	if(SessionTypeRef == TLSocial::FacebookRef)
	{
		IPod::Facebook::BeginSession(APIKey, APISecret);
	}
	else if(SessionTypeRef == TLSocial::AGONOnlineRef)
	{
		IPod::AGONOnline::BeginSession(APIKey, APISecret);
	}
	else if(SessionTypeRef == TLSocial::OpenFeintRef)
	{
		IPod::OpenFeint::BeginSession(APIKey, APISecret);
	}

}

void TLSocial::Platform::EndSession(TRefRef SessionTypeRef)
{
	if(SessionTypeRef == TLSocial::FacebookRef)
	{
		IPod::Facebook::EndSession();
	}
	else if(SessionTypeRef == TLSocial::AGONOnlineRef)
	{
		IPod::AGONOnline::EndSession();
	}
	else if(SessionTypeRef == TLSocial::OpenFeintRef)
	{
		IPod::OpenFeint::EndSession();
	}

}

void TLSocial::Platform::OpenDashboard(TRefRef SessionTypeRef)
{
	if(SessionTypeRef == TLSocial::FacebookRef)
	{
		IPod::Facebook::OpenDashboard();
	}
	else if(SessionTypeRef == TLSocial::AGONOnlineRef)
	{
		IPod::AGONOnline::OpenDashboard();
	}
	else if(SessionTypeRef == TLSocial::OpenFeintRef)
	{
		IPod::OpenFeint::OpenDashboard();
	}

}

void TLSocial::Platform::OpenLeaderboard(TRefRef SessionTypeRef)
{
	if(SessionTypeRef == TLSocial::AGONOnlineRef)
	{
		IPod::AGONOnline::OpenLeaderboard();
	}
	else if(SessionTypeRef == TLSocial::OpenFeintRef)
	{
		IPod::OpenFeint::OpenLeaderboard();
	}
}



void TLSocial::Platform::SubmitScore(TRefRef SessionTypeRef, const s32& Score, const TString& Format, const s32& LeaderboardID)
{
	if(SessionTypeRef == TLSocial::AGONOnlineRef)
	{
		IPod::AGONOnline::SubmitScore(Score, Format, LeaderboardID);
	}
	else if(SessionTypeRef == TLSocial::OpenFeintRef)
	{
		IPod::OpenFeint::SubmitScore(Score, Format, LeaderboardID);
	}
}


