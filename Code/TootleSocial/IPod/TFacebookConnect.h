/*
 *  TFacebookConnect.h
 *  TootleSocial
 *
 *  Created by Duane Bradbury on 04/06/2009.
 *  Copyright 2009 Tootle. All rights reserved.
 *
 */

#pragma once

#include <TootleCore/TString.h>

namespace TLSocial
{
	namespace Platform
	{
		namespace IPod
		{
			namespace Facebook
			{
				
				void BeginSession(const TString& APIKey, const TString& APISecret);
				void EndSession();
				
				void OpenDashboard();
			}
		}
	}
}