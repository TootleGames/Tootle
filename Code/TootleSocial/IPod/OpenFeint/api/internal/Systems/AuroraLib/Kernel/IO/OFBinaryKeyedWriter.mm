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

#include "OFBinaryKeyedWriter.h"
#include "OFBinaryKeyed.h"

OFImplementRTTI(OFBinaryKeyedWriter, OFOutputSerializer);

OFBinaryKeyedWriter::OFBinaryKeyedWriter(bool serializeResourcesExternally)
{
	setSerializeResourcesExternally(serializeResourcesExternally);
}

OFBinaryKeyedWriter::OFBinaryKeyedWriter(const char* filePath, bool serializeResourcesExternally)
{
	doCommonConstruction(new OFBinaryFileSink(filePath), serializeResourcesExternally);
}
	
OFBinaryKeyedWriter::OFBinaryKeyedWriter(OFPointer<OFBinarySink> dataSink, bool serializeResourcesExternally)
{
	doCommonConstruction(dataSink, serializeResourcesExternally);
}

void OFBinaryKeyedWriter::doCommonConstruction(OFPointer<OFBinarySink> dataSink, bool serializeResourcesExternally)
{
	setSerializeResourcesExternally(serializeResourcesExternally);	
	initialize(dataSink);
}

void OFBinaryKeyedWriter::initialize(OFPointer<OFBinarySink> dataSink)
{
	mDataSink = dataSink;
	onScopePushed(createKey("root"));
}
	
OFBinaryKeyedWriter::~OFBinaryKeyedWriter()
{
	onScopePopped(createKey("root"));
}

const OFRTTI* OFBinaryKeyedWriter::beginDecodeType()
{
	OFAssert(0, "Internal error. An output serializer should not be reading.");
	return 0;
}

void OFBinaryKeyedWriter::endDecodeType()
{
}

void OFBinaryKeyedWriter::beginEncodeType(const OFRTTI* typeToEncode)
{
	OFSdbmHashedString typeId = typeToEncode->GetTypeId();
	io("___type", typeId);
}

void OFBinaryKeyedWriter::endEncodeType()
{
}

bool OFBinaryKeyedWriter::supportsKeys() const
{
	return true;
}

void OFBinaryKeyedWriter::writeData(OFISerializerKey* keyName, unsigned int dataSize, const void* data)
{
	mDataSink->write(&OFBinaryKeyed::DataMarker, sizeof(OFBinaryKeyed::DataMarker));	

	writeKey(keyName);
	
	mDataSink->write(&dataSize, sizeof(unsigned int));
	mDataSink->write(data, dataSize);
}

void OFBinaryKeyedWriter::onScopePushed(OFISerializerKey* scopeName)
{	
	mDataSink->write(&OFBinaryKeyed::ScopeMarkerBegin, sizeof(OFBinaryKeyed::ScopeMarkerBegin));
	writeKey(scopeName);
}

void OFBinaryKeyedWriter::writeKey(OFISerializerKey* scopeName)
{
	const unsigned char keyLength = strlen(scopeName->getAsString());
	mDataSink->write(&keyLength, sizeof(keyLength));
	mDataSink->write(scopeName->getAsString(), keyLength);
}

void OFBinaryKeyedWriter::onScopePopped(OFISerializerKey* scopeName)
{
	mDataSink->write(&OFBinaryKeyed::ScopeMarkerEnd, sizeof(OFBinaryKeyed::ScopeMarkerEnd));
}

OFPointer<OFBinarySink> OFBinaryKeyedWriter::getDataSink() const
{
	return mDataSink;
}