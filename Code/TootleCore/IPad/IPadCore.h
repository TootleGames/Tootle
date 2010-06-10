/*
 *  IPadCore.h
 *  TootleCore
 *
 *  Created by Duane Bradbury on 15/12/2009.
 *  Copyright 2009 Tootle. All rights reserved.
 *
 */

#pragma once

#include "../TLTypes.h"

#include <math.h>
#include <stdio.h>
#include <typeinfo>

//	include low level ipod stuff
//#import <Foundation/Foundation.h>


//	forward declarations
class TString;
class TBinaryTree;


namespace TLTime
{
	class TTimestamp;
	
	namespace Platform
	{
		SyncBool			Init();				//	time init
	}
}

namespace TLCore
{
	namespace Platform
	{
		SyncBool			Init();					//	platform init
		SyncBool			Update();				//	platform update
		SyncBool			Shutdown();				//	platform shutdown
		void				Sleep(u32 Millisecs);	//	platform thread/process sleep
		
		//	ipoddy specific funcs
		//void				GetString(TString& String, const NSString* pNSString);	//	append ipod foundation string to TString
		
		void				QueryHardwareInformation(TBinaryTree& Data);
		void				QueryLanguageInformation(TBinaryTree& Data);
	}
}
