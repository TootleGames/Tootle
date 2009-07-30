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

#include "OFOutputSerializer.h"
#include "OFBinarySink.h"

class OFBinaryKeyedWriter : public OFOutputSerializer
{
OFDeclareRTTI;
public:
	OFBinaryKeyedWriter(const char* filePath, bool serializeResourcesExternally = true);
	OFBinaryKeyedWriter(OFPointer<OFBinarySink> dataSink, bool serializeResourcesExternally = true);

	~OFBinaryKeyedWriter();

	bool supportsKeys() const;

	OFPointer<OFBinarySink> getDataSink() const;

protected:
	OFBinaryKeyedWriter(bool serializeResourcesExternally);
	void initialize(OFPointer<OFBinarySink> dataSink);
	
private:
	void nviIo(OFISerializerKey* keyName, bool& value)			{ writeData(keyName, sizeof(bool), &value); } 
	void nviIo(OFISerializerKey* keyName, int& value)				{ writeData(keyName, sizeof(int), &value); }
	void nviIo(OFISerializerKey* keyName, unsigned int& value)	{ writeData(keyName, sizeof(unsigned int), &value); }
	void nviIo(OFISerializerKey* keyName, int64_t& value)			{ writeData(keyName, sizeof(int64_t), &value); }
	void nviIo(OFISerializerKey* keyName, float& value)			{ writeData(keyName, sizeof(float), &value); }
	void nviIo(OFISerializerKey* keyName, double& value)			{ writeData(keyName, sizeof(double), &value); }
	void nviIo(OFISerializerKey* keyName, std::string& value)
	{
		uint32_t len = value.length();
		char const* cStr = value.c_str();
		writeData(keyName, len, cStr);
	}
	void nviIo(OFISerializerKey* keyName, OFRetainedPtr<NSString>& value)
	{
		NSData* data = [value.get() dataUsingEncoding:NSStringEncodingConversionExternalRepresentation];
		writeData(keyName, [data length], [data bytes]);
	}
	void nviIo(OFISerializerKey* keyName, OFRetainedPtr<NSData>& value)
	{
		OFAssert(false, @"Writing NSData Not Implemented In OFBinaryKeyedWrited");
	}
	
	void doCommonConstruction(OFPointer<OFBinarySink> dataSink, bool serializeResourcesExternally);

	void writeData(OFISerializerKey* keyName, unsigned int dataSize, const void* data);
	virtual void writeKey(OFISerializerKey* scopeName);
	
	void onScopePushed(OFISerializerKey* scopeName);
	void onScopePopped(OFISerializerKey* scopeName);
		
	const OFRTTI* beginDecodeType();
	void endDecodeType();	
	void beginEncodeType(const OFRTTI* typeToEncode);
	void endEncodeType();
	
	OFPointer<OFBinarySink> mDataSink;
};