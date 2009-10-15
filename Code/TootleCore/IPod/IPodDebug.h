/*
 *  IPodDebug.h
 *  TootleCore
 *
 *  Created by Duane Bradbury on 14/10/2009.
 *  Copyright 2009 Tootle. All rights reserved.
 *
 */

#pragma once

#include "../TLTypes.h"

//	forward declarations
class TString;


namespace TLDebug
{
	namespace Platform
	{
		
		SyncBool		Initialise();
		SyncBool		Shutdown();
		
		
		void		PrintToBuffer(const TString& String);	//	platform specific debug output - buffered
		
		Bool		Break(const TString& String);	//	return FALSE to stop app, TRUE and will attempt to continue
	}
};
