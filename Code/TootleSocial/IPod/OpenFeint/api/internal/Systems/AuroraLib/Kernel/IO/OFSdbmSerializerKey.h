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

#pragma once

#include "OFISerializerKey.h"
#include "OFHashedString.h"

class OFSdbmSerializerKey : public OFISerializerKey
{
public:
	explicit OFSdbmSerializerKey(const char* keyName) : value(keyName) {}
	explicit OFSdbmSerializerKey(const OFSdbmHashedString& hashedName) : value(hashedName) {}

	bool equals(const OFISerializerKey* rhv) const			{ return value == ((const OFSdbmSerializerKey*)rhv)->value; }
	bool lessthan(const OFISerializerKey* rhv) const		{ return value <	((const OFSdbmSerializerKey*)rhv)->value; }
	const char* getAsString() const							
	{
		if(!mLazyString.get()) 
		{
			mLazyString.reset([NSString stringWithFormat:@"%x", value.valueForSerialization()]);
		}
		
		return [mLazyString.get() UTF8String];
	}

	OFSdbmHashedString value;
	
private:
	mutable OFRetainedPtr<NSString> mLazyString;
};