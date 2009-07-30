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

#import "OFXmlReader.h"
#import "OFXmlDocument.h"
#include "OFRTTIRepository.h"

OFImplementRTTI(OFXmlReader, OFInputSerializer);

OFXmlReader::OFXmlReader(const char* fileName, StringDecoder decoder)
: mStringDecoder(decoder)
{
	NSString* filePath = [[NSBundle mainBundle] pathForResource:[NSString stringWithCString:fileName] ofType:@"xml"];
	if(![[NSFileManager defaultManager] fileExistsAtPath:filePath])
	{
		// OFLog(@"OFXmlReader: Expected xml file at path %@. Not Parsing.", filePath);
		return;
	}
	
	mDocument.reset([OFXmlDocument xmlDocumentWithData:[NSData dataWithContentsOfFile:filePath]]);
}

OFXmlReader::OFXmlReader(NSData* data, StringDecoder decoder)
: mStringDecoder(decoder)
{
	mDocument.reset([OFXmlDocument xmlDocumentWithData:data]);
}

OFXmlReader::OFXmlReader(NSString* fullPathName, StringDecoder decoder)
: mStringDecoder(decoder)
{
	mDocument.reset([OFXmlDocument xmlDocumentWithData:[NSData dataWithContentsOfFile:fullPathName]]);
}

bool OFXmlReader::getNextValueAtCurrentScope(OFISerializerKey* keyName, NSString*& outString)
{
	return [mDocument.get() nextValueAtCurrentScopeWithKey:keyName->getAsString() outValue:outString];
}

void OFXmlReader::nviIo(OFISerializerKey* keyName, bool& value)
{
	NSString* outString;
	const bool isValid = getNextValueAtCurrentScope(keyName, outString);
	if(isValid)
	{
		value = [outString boolValue];
	}
}

void OFXmlReader::nviIo(OFISerializerKey* keyName, int& value)
{
	NSString* outString;
	const bool isValid = getNextValueAtCurrentScope(keyName, outString);
	if(isValid)
	{
		value = [outString intValue];
	}
}

void OFXmlReader::nviIo(OFISerializerKey* keyName, std::string& value)
{
	NSString* outString;
	const bool isValid = getNextValueAtCurrentScope(keyName, outString);
	if(isValid)
	{
		value = [decodeString(outString).get() UTF8String];
	}
}

void OFXmlReader::nviIo(OFISerializerKey* keyName, unsigned int& value)
{
	NSString* outString;
	const bool isValid = getNextValueAtCurrentScope(keyName, outString);
	if(isValid)
	{
		value = [outString intValue];
	}
}

void OFXmlReader::nviIo(OFISerializerKey* keyName, int64_t& value)
{
	NSString* outString;
	const bool isValid = getNextValueAtCurrentScope(keyName, outString);
	if(isValid)
	{
		value = [outString longLongValue];
	}
}

void OFXmlReader::nviIo(OFISerializerKey* keyName, double& value)
{
	NSString* outString;
	const bool isValid = getNextValueAtCurrentScope(keyName, outString);
	if(isValid)
	{
		value = [outString doubleValue];
	}
}

void OFXmlReader::nviIo(OFISerializerKey* keyName, float& value)
{
	NSString* outString;
	const bool isValid = getNextValueAtCurrentScope(keyName, outString);
	if(isValid)
	{
		value = [outString floatValue];
	}
}

void OFXmlReader::nviIo(OFISerializerKey* keyName, OFRetainedPtr<NSString>& value)
{
	NSString* outString;
	const bool isValid = getNextValueAtCurrentScope(keyName, outString);
	if(isValid)
	{
		value.reset(decodeString(outString));
	}
}

void OFXmlReader::onScopePushed(OFISerializerKey* scopeName)
{
	[mDocument.get() pushNextScope:scopeName->getAsString()];
}

void OFXmlReader::onScopePopped(OFISerializerKey* scopeName)
{
	[mDocument.get() popScope];
}

const OFRTTI* OFXmlReader::beginDecodeType()
{
	[mDocument.get() pushNextUnreadScope];
	return OFRTTIRepository::Instance()->getType([[mDocument.get() getCurrentScopeShortName] UTF8String]);
}

void OFXmlReader::endDecodeType()
{
	onScopePopped(NULL);
}

void OFXmlReader::beginEncodeType(const OFRTTI* typeToEncode)
{
	OFAssert(0, "Internal error. An input serializer should not be writing.");
}

void OFXmlReader::endEncodeType()
{
	OFAssert(0, "Internal error. An input serializer should not be writing.");
}

bool OFXmlReader::supportsKeys() const
{
	return true;
}

OFRetainedPtr<NSString> OFXmlReader::decodeString(NSString* stringToDecode)
{
	if(mStringDecoder == NULL)
	{
		return stringToDecode;
	}
	
	return mStringDecoder(stringToDecode);
}