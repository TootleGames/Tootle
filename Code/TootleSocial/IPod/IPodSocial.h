/*
 *  IPodSession.h
 *  TootleSocial
 *
 *  Created by Duane Bradbury on 04/06/2009.
 *  Copyright 2009 Tootle. All rights reserved.
 *
 */

#pragma once

#include <TootleCore/TRef.h>


namespace TLSocial
{
	namespace Platform
	{
		void BeginSession(TRefRef SessionTypeRef, const TString& APIKey, const TString& APISecret);
		void EndSession(TRefRef SessionTypeRef);

		void OpenDashboard(TRefRef SessionTypeRef);
		void OpenLeaderboard(TRefRef SessionTypeRef);

		
		void SubmitScore(TRefRef SessionTypeRef, const s32& Score, const TString& Format, const s32& LeaderboardID);
	}
}