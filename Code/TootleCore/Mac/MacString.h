/*
 *  MacString.h
 *  TootleCore
 *
 *  Created by Duane Bradbury on 15/12/2009.
 *  Copyright 2009 Tootle. All rights reserved.
 *
 */

#pragma once

#include "../TString.h"
#include <Foundation/NSString.h>

namespace TLString
{	
	NSString* ConvertToUnicharString(const TString& String);
}
