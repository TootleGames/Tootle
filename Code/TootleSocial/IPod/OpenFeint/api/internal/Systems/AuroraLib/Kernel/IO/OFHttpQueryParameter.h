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

#include "OFBinarySink.h"

class OFOutputSerializer;
@class MPURLRequestParameter;
class OFHttpQueryParameter : public OFSmartObject
{
public:
	OFHttpQueryParameter(NSString* name);

	virtual NSString* getAsUrlParameter() const = 0;
	virtual void appendToMultipartFormData(NSMutableData* multipartStream) const = 0;
	virtual MPURLRequestParameter* getAsMPURLRequestParameter() const = 0;

	virtual NSString* getValueAsString() const = 0;
	NSString* getName() const;

protected:
	OFRetainedPtr<NSString> mName;
};

class OFHttpAsciiParameter : public OFHttpQueryParameter
{
public:
	OFHttpAsciiParameter(NSString* parameterName, NSString* parameterValue);

	NSString* getValueAsString() const;
	NSString* getAsUrlParameter() const;
	MPURLRequestParameter* getAsMPURLRequestParameter() const;
	void appendToMultipartFormData(NSMutableData* multipartStream) const;

private:
	OFRetainedPtr<NSString> mValue;	
};

class OFHttpBinaryParameter : public OFHttpQueryParameter
{
public:
	OFHttpBinaryParameter(NSString* parameterName, OFPointer<OFBinaryMemorySink> binaryData, NSString* dataType);

	NSString* getValueAsString() const;
	NSString* getAsUrlParameter() const;
	MPURLRequestParameter* getAsMPURLRequestParameter() const;
	void appendToMultipartFormData(NSMutableData* multipartStream) const;

private:
	OFRetainedPtr<NSString> mDataType;
	OFPointer<OFBinaryMemorySink> mData;
};