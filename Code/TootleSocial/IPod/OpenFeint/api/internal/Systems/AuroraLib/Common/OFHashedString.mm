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

#include "OFHashedString.h"

namespace
{
	// http://www.cse.yorku.ca/~oz/hash.html
	unsigned int Sdbm(const char *cStr)
	{
		unsigned int hashVal = 0;
		if (cStr)
		{
			unsigned char c;
			while ((c = *cStr++) != 0)
			{
				hashVal = (hashVal<<6) + (hashVal<<16) - hashVal + c;
			}
		}
		return hashVal;
	}
}

const OFSdbmHashedString OFSdbmHashedString::sNullHash;

OFHashedString::OFHashedString(unsigned int OFHashedString, const char* str)
: mHashedString(OFHashedString)
{
}	

OFSdbmHashedString::OFSdbmHashedString(const char* str)
: OFHashedString(Sdbm(str), str)
{
}

OFSdbmHashedString::OFSdbmHashedString()
: OFHashedString(0, "")
{
}
	
bool OFSdbmHashedString::operator ==(const OFSdbmHashedString& rhv) const
{
	return mHashedString == rhv.mHashedString;
}

bool OFSdbmHashedString::operator <(const OFSdbmHashedString& rhv) const
{
	return mHashedString < rhv.mHashedString;
}

bool OFSdbmHashedString::operator !=(const OFSdbmHashedString& rhv) const
{
	return mHashedString != rhv.mHashedString;
}