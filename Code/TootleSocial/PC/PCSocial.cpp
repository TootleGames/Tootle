/*
 *  PCSocial.cpp
 *  TootleSocial
 *
 *  Created by Duane Bradbury on 04/06/2009.
 *  Copyright 2009 Tootle. All rights reserved.
 *
 */

#include "PCSocial.h"

#include <TootleCore/TLTypes.h>
#include <TootleCore/TRef.h>
#include <TootleCore/TString.h>

namespace TLSocial
{
	namespace Platform
	{
		void BeginSession(TRefRef SessionTypeRef)	{}
		void EndSession(TRefRef SessionTypeRef)		{}
		
		void OpenDashboard(TRefRef SessionTypeRef)	{}
		
		void SubmitScore(TRefRef SessionTypeRef, const s32& Score, const TString& Format, const s32& LeaderboardID)	{}
	}
}


