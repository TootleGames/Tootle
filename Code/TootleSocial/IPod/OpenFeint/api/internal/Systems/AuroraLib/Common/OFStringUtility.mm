////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///
///  Copyright (c) 2009 Aurora Feint Inc.
///
///  This library is free software; you can redistribute it and/or
///  modify it under the terms of the GNU Lesser General Public
///  License as published by the Free Software Foundation; either
///  
///  version 3 of the License, or (at your option) any later version.
///  
///  This library is distributed in the hope that it will be useful,
///  
///  but WITHOUT ANY WARRANTY; without even the implied warranty of
///  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
///  Lesser General Public License for more details.
///  
///  
///  You should have received a copy of the GNU Lesser General Public
///  License along with this library; if not, write to the Free Software
///  
///  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
///
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "OFStringUtility.h"

namespace
{
	CFStringRef illegalCharacters = CFSTR("!@#$%^&*()+=\\][{}|';:\"/.,<>?`~ ");		
}

namespace OFStringUtility
{
	OFRetainedPtr<NSString> convertToValidParameter(OFRetainedPtr<NSString> str)
	{
		return [(NSString*)CFURLCreateStringByAddingPercentEscapes(NULL, (CFStringRef)str.get(), NULL, illegalCharacters, kCFStringEncodingUTF8) autorelease];
	}

	OFRetainedPtr<NSString> convertFromValidParameter(NSString* str)
	{
		return [(NSString*)CFURLCreateStringByReplacingPercentEscapes(NULL, (CFStringRef)str, CFSTR("")) autorelease];
	}
}