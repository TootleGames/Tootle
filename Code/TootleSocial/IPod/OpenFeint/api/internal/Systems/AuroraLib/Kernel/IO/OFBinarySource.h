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

class OFBinarySource : public OFSmartObject
{
OFDeclareRTTI;
OFDeclareNonCopyable(OFBinarySource);
public:
	OFBinarySource() {}
	
	virtual void read(void* data, unsigned int dataSize) = 0;
	virtual bool isEmpty() const = 0;
};

class OFBinaryFileSource : public OFBinarySource
{
OFDeclareRTTI;
public:
	OFBinaryFileSource(const char* filePath);
	~OFBinaryFileSource();
	
	void read(void* data, unsigned int dataSize);
	bool isEmpty() const;
		
private:
	FILE* mFileStream;
};

class OFBinaryMemorySource : public OFBinarySource
{
OFDeclareRTTI;
public:
	OFBinaryMemorySource(const char* data, unsigned int dataSize);
	~OFBinaryMemorySource();
	
	void read(void* data, unsigned int dataSize);
	bool isEmpty() const;
	
private:
	const char* const mData;
	const unsigned int mNumBytes;
	unsigned int mNextByte;
};