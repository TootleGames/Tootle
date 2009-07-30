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

#include "OFSmartObject.h"

class OFXmlElement : public OFSmartObject
{
public: 
	OFXmlElement(NSString* _name);

	bool getValueWithName(const char* name, NSString*& outString, bool markChildAsRead = false);
	OFPointer<OFXmlElement> getChildWithName(NSString* name, bool getNextUnreadChild = false);
	OFPointer<OFXmlElement> getChildWithName(const char* name, bool getNextUnreadChild = false);

	OFPointer<OFXmlElement> dequeueNextUnreadChild();
	OFPointer<OFXmlElement> dequeueNextUnreadChild(NSString* nameToFind);
	OFPointer<OFXmlElement> dequeueNextUnreadChild(const char* nameToFind);	
	
	void addChild(OFXmlElement* childNode)			{ mChildren.push_back(childNode); }
	OFXmlElement* getChildAt(unsigned int index) const{ return mChildren.at(index); }
	bool hasChildren() const						{ return !mChildren.empty(); }
	
	void setName(NSString* name)					{ mName = name; }
	NSString* getName() const						{ return mName; }
	
	void setAttributes(NSDictionary* attributes)	{ mAttributes = attributes; }
	NSString* getAttributeNamed(NSString* name)		{ return [mAttributes.get() valueForKey:name]; }
	
	void setValue(NSString* value)					{ mValue = value; }
	NSString* getValue() const						{ return mValue; } 
	bool hasNilValue() const						{ return mValue.get() == nil; }
	bool hasValue() const							{ return !hasNilValue(); }

private:	
	OFRetainedPtr<NSString> mValue;
	OFRetainedPtr<NSString> mName;
	std::vector<OFPointer<OFXmlElement> > mChildren;
	OFRetainedPtr<NSDictionary> mAttributes;
	bool mHasBeenRead;	
};