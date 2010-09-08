/*
 *  IPodFileSys.h
 *  TootleFileSys
 *
 *  Created by Duane Bradbury on 23/09/2009.
 *  Copyright 2009 Tootle. All rights reserved.
 *
 */

#pragma once

#include <TootleCore/TLTypes.h>
#include <TootleCore/TString.h>

namespace TLFileSys
{
	
	namespace Platform
	{
		Bool GetAssetDirectory(TString& AssetDir);
		Bool GetUserDirectory(TString& UserDir);
		Bool GetApplicationURL(TString& url);
	}
}
