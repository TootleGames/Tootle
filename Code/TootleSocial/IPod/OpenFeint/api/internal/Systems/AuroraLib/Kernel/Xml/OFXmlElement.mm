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

#include "OFXmlElement.h"

OFXmlElement::OFXmlElement(NSString* _name)
: mName(_name)
, mHasBeenRead(false) 
{
}

OFPointer<OFXmlElement> OFXmlElement::dequeueNextUnreadChild(const char* nameToFind)
{			
	return dequeueNextUnreadChild([NSString stringWithCString:nameToFind]);
}

OFPointer<OFXmlElement> OFXmlElement::dequeueNextUnreadChild(NSString* nameToFind)
{
	const unsigned int numChildren = mChildren.size();
	for(unsigned int i = 0; i < numChildren; ++i)
	{
		OFXmlElement* child = mChildren.at(i).get();
		if(!child->mHasBeenRead && [child->mName.get() isEqualToString:nameToFind])
		{
			child->mHasBeenRead = true;
			return child;
		}
	}
	
	return NULL;		
}
		
OFPointer<OFXmlElement> OFXmlElement::dequeueNextUnreadChild()
{
	const unsigned int numChildren = mChildren.size();
	for(unsigned int i = 0; i < numChildren; ++i)
	{
		OFXmlElement* child = mChildren.at(i).get();
		if(!child->mHasBeenRead)
		{
			child->mHasBeenRead = true;
			return child;
		}
	}
	
	return NULL;		
}

OFPointer<OFXmlElement> OFXmlElement::getChildWithName(const char* name, bool getNextUnreadChild)
{
	NSString* nsName = [NSString stringWithCString:name];
	return getChildWithName(nsName, getNextUnreadChild);
}

OFPointer<OFXmlElement> OFXmlElement::getChildWithName(NSString* name, bool getNextUnreadChild)
{
	const unsigned int numChildren = mChildren.size();
	for(unsigned int i = 0; i < numChildren; ++i)
	{
		OFXmlElement* child = mChildren.at(i).get();
		
		if([child->mName.get() isEqualToString:name])
		{
			if(!getNextUnreadChild || (getNextUnreadChild && !child->mHasBeenRead))
			{
				return child;
			}
		}
	}
	
	return NULL;
}

bool OFXmlElement::getValueWithName(const char* name, NSString*& outString, bool markChildAsRead)
{
	OFPointer<OFXmlElement> child = getChildWithName(name, markChildAsRead);
	if(child.get())
	{
		if(markChildAsRead)
		{
			child->mHasBeenRead = true;
		}
		
		outString = child->mValue.get();
		return true;
	}
	
	return false;
}