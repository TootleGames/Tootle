/*
 *  PCFileSys.h
 *  TootleFilesys
 *
 *  Created by Duane Bradbury on 22/09/2009.
 *  Copyright 2009 Tootle. All rights reserved.
 *
 */

#pragma once
#include "../TLFileSys.h"

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
