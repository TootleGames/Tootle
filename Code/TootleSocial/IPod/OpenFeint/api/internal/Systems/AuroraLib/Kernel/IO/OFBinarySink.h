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

class OFBinarySink : public OFSmartObject
{
OFDeclareRTTI;
OFDeclareNonCopyable(OFBinarySink);
public:
	OFBinarySink() {}
	
	virtual void write(const void* data, unsigned int dataSize) = 0;
};

class OFBinaryFileSink : public OFBinarySink
{
OFDeclareRTTI;
public:
	OFBinaryFileSink(const char* filePath);
	~OFBinaryFileSink();
	
	void write(const void* data, unsigned int dataSize);
	
private:
	FILE* mFileStream;
};

class OFBinaryMemorySink : public OFBinarySink
{
OFDeclareRTTI;
public:
	OFBinaryMemorySink();
	OFBinaryMemorySink(NSData* data);
	~OFBinaryMemorySink();
	
	const void* getDataBuffer() const;
	NSData* getNSData() const;
	unsigned int getDataSize() const;
	
	void write(const void* data, unsigned int dataSize);

private:
	OFRetainedPtr<NSData> mData;
};