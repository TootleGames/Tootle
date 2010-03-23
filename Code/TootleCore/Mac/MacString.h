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
	NSString*	ConvertToUnicharString(const TString& String);
	void		Append(TString& String,const NSString* pNSString);				//	append cocoa string to TString
}


//---------------------------------------------------
//	append cocoa string to TString
//---------------------------------------------------
template<>					
FORCEINLINE TString& operator<<(TString& String,const NSString* pNSString)
{
	TLString::Append( String, pNSString );
	return String;
}

//---------------------------------------------------
//	gr: I don't know why but sometimes the compiler won't auto convert an
//		NSString* pString; to a const NSString* pString;
//		so the normal templated << operator above doesn't get called (It does call the base template though?!)
//		so this is done to workaround it so we don't have to put const NSString* everywhere
//	note: this is not one of the templated string << operators, it just exists globally. (all the templated stirng ones take const params)
//---------------------------------------------------
FORCEINLINE TString& operator<<(TString& String,NSString* pNSString)
{
	const NSString* pConstString = pNSString;
	return String << pConstString;
}



