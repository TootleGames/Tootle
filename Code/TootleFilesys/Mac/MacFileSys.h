/*
 *  MacFileSys.h
 *  TootleFilesys
 *
 *  Created by Duane Bradbury on 22/09/2009.
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
		Bool GetAssetDirectory(TTempString& AssetDir);
		Bool GetAssetSubDirectory(TTempString& AssetDir, const TTempString& Subdirectory);
		
		Bool GetUserDirectory(TTempString& UserDir);
		
		Bool GetApplicationURL(TTempString& url);
	}
}
