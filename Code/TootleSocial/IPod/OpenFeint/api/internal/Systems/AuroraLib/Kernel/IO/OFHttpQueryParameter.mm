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

#include "OFHttpQueryParameter.h"
#include "OFOutputSerializer.h"
#import "MPURLRequestParameter.h"

OFHttpQueryParameter::OFHttpQueryParameter(NSString* name)
: mName(name)
{
}

NSString* OFHttpQueryParameter::getName() const
{
	return mName;
}

// ----------------------------------------------------------------------
// ----------------------------------------------------------------------
// ----------------------------------------------------------------------
// ----------------------------------------------------------------------

OFHttpAsciiParameter::OFHttpAsciiParameter(NSString* parameterName, NSString* parameterValue)
: OFHttpQueryParameter(parameterName)
, mValue(parameterValue)
{
}

NSString* OFHttpAsciiParameter::getAsUrlParameter() const
{
	return [NSString stringWithFormat:@"%@=%@", mName.get(), mValue.get()];
}

MPURLRequestParameter* OFHttpAsciiParameter::getAsMPURLRequestParameter() const
{
	NSString* value = mValue.get() ? mValue.get() : @"";
	return [[[MPURLRequestParameter alloc] initWithName:getName() andValue:value] autorelease];
}

void OFHttpAsciiParameter::appendToMultipartFormData(NSMutableData* multipartStream) const
{
	const NSString* headerContentDisposition = [NSString stringWithFormat:@"Content-Disposition: form-data; name=\"%@\"\r\n", mName.get()];
	const NSString* headerSpacer = @"\r\n";
	
	[multipartStream appendData:[headerContentDisposition dataUsingEncoding:NSUTF8StringEncoding]];
	[multipartStream appendData:[headerSpacer dataUsingEncoding:NSUTF8StringEncoding]];	
	[multipartStream appendData:[mValue.get() dataUsingEncoding:NSUTF8StringEncoding]];	
	[multipartStream appendData:[headerSpacer dataUsingEncoding:NSUTF8StringEncoding]];	
}

NSString* OFHttpAsciiParameter::getValueAsString() const
{
	return mValue.get();
}

// ----------------------------------------------------------------------
// ----------------------------------------------------------------------
// ----------------------------------------------------------------------
// ----------------------------------------------------------------------
#pragma mark OFHttpBinaryParameter

OFHttpBinaryParameter::OFHttpBinaryParameter(NSString* parameterName, OFPointer<OFBinaryMemorySink> binaryData, NSString* dataType)
: OFHttpQueryParameter(parameterName)
, mData(binaryData)
, mDataType(dataType)
{
}

NSString* OFHttpBinaryParameter::getAsUrlParameter() const
{
	OFAssert(0, "Something proably went wrong. Attempting to get a binary data parameter for use in a URL query string.");
	return @"";
}

MPURLRequestParameter* OFHttpBinaryParameter::getAsMPURLRequestParameter() const
{
	return [[[MPURLRequestParameter alloc] initWithName:getName() andBlob:mData->getNSData() andDataType:mDataType.get()] autorelease];
}

void OFHttpBinaryParameter::appendToMultipartFormData(NSMutableData* multipartStream) const
{
	const NSString* headerContentDisposition = [NSString stringWithFormat:@"Content-Disposition: form-data; name=\"%@\"; filename=\"%@.%@\"\r\n", mName.get(), mName.get(), mDataType.get()];
	const NSString* headerContentType =  [NSString stringWithFormat:@"Content-Type: %@\r\n", mDataType.get()];
	const NSString* headerContentEncoding =  [NSString stringWithFormat:@"Content-Transfer-Encoding: binary\r\n"];	
	const NSString* headerSpacer = @"\r\n";
	
	[multipartStream appendData:[headerContentDisposition dataUsingEncoding:NSUTF8StringEncoding]];
	[multipartStream appendData:[headerContentType dataUsingEncoding:NSUTF8StringEncoding]];
	[multipartStream appendData:[headerContentEncoding dataUsingEncoding:NSUTF8StringEncoding]];
	[multipartStream appendData:[headerSpacer dataUsingEncoding:NSUTF8StringEncoding]];
	[multipartStream appendBytes:mData->getDataBuffer() length:mData->getDataSize()];
	[multipartStream appendData:[headerSpacer dataUsingEncoding:NSUTF8StringEncoding]];	
}
	
NSString* OFHttpBinaryParameter::getValueAsString() const
{
	OFAssert(0, "Something proably went wrong. Attempting to get a binary data parameter for use in a URL query string.");
	return @"";
}