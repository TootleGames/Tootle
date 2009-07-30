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

#include "OFDictionaryWriter.h"

OFImplementRTTI(OFDictionaryWriter, OFOutputSerializer);

OFDictionaryWriter::OFDictionaryWriter()
{
	mDictionary.reset([NSMutableDictionary dictionary]);
}

OFDictionaryWriter::~OFDictionaryWriter()
{
}

bool OFDictionaryWriter::supportsKeys() const
{
	return true;
}

NSDictionary* OFDictionaryWriter::getDictionary() const
{
	return mDictionary.get();
}

void OFDictionaryWriter::nviIo(OFISerializerKey* keyName, bool& value)	
{ [mDictionary.get() setValue:[NSNumber numberWithBool:value] forKey:[NSString stringWithCString:keyName->getAsString()]]; }

void OFDictionaryWriter::nviIo(OFISerializerKey* keyName, int& value)
{ [mDictionary.get() setValue:[NSNumber numberWithInt:value] forKey:[NSString stringWithCString:keyName->getAsString()]]; }

void OFDictionaryWriter::nviIo(OFISerializerKey* keyName, unsigned int& value)
{ [mDictionary.get() setValue:[NSNumber numberWithUnsignedInt:value] forKey:[NSString stringWithCString:keyName->getAsString()]]; }

void OFDictionaryWriter::nviIo(OFISerializerKey* keyName, int64_t& value)
{ [mDictionary.get() setValue:[NSNumber numberWithLongLong:value] forKey:[NSString stringWithCString:keyName->getAsString()]]; }

void OFDictionaryWriter::nviIo(OFISerializerKey* keyName, float& value)
{ [mDictionary.get() setValue:[NSNumber numberWithFloat:value] forKey:[NSString stringWithCString:keyName->getAsString()]]; }

void OFDictionaryWriter::nviIo(OFISerializerKey* keyName, double& value)
{ [mDictionary.get() setValue:[NSNumber numberWithDouble:value] forKey:[NSString stringWithCString:keyName->getAsString()]]; }

void OFDictionaryWriter::nviIo(OFISerializerKey* keyName, std::string& value)
{ [mDictionary.get() setValue:[NSString stringWithCString:value.c_str()] forKey:[NSString stringWithCString:keyName->getAsString()]]; }

void OFDictionaryWriter::nviIo(OFISerializerKey* keyName, OFRetainedPtr<NSString>& value)
{ [mDictionary.get() setValue:value.get() forKey:[NSString stringWithCString:keyName->getAsString()]]; }
	
const OFRTTI* OFDictionaryWriter::beginDecodeType() { return NULL; }
void OFDictionaryWriter::endDecodeType() {}
void OFDictionaryWriter::beginEncodeType(const OFRTTI* typeToEncode) {}
void OFDictionaryWriter::endEncodeType() {}