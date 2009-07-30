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

#include "OFBinarySource.h"

OFImplementRTTI(OFBinarySource, OFSmartObject);
OFImplementRTTI(OFBinaryFileSource, OFBinaryMemorySource);
OFImplementRTTI(OFBinaryMemorySource, OFBinaryMemorySource);

OFBinaryFileSource::OFBinaryFileSource(const char* filePath)
{
	mFileStream = fopen(filePath, "rb+");
}

OFBinaryFileSource::~OFBinaryFileSource()
{
	fclose(mFileStream);
	mFileStream = NULL;
}

bool OFBinaryFileSource::isEmpty() const
{
	return feof(mFileStream);
}
	
void OFBinaryFileSource::read(void* data, unsigned int dataSize)
{
	fread(data, dataSize, 1, mFileStream);
}

// ------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------

OFBinaryMemorySource::OFBinaryMemorySource(const char* data, unsigned int dataSize)
: mData(data)
, mNumBytes(dataSize)
, mNextByte(0)
{
}

OFBinaryMemorySource::~OFBinaryMemorySource()
{
}

void OFBinaryMemorySource::read(void* data, unsigned int dataSize)
{
	unsigned int sizeToRead = dataSize;
	if(mNextByte + sizeToRead >= mNumBytes)
	{
		sizeToRead = mNumBytes - mNextByte;
	}
	
	memcpy(data, mData + mNextByte, sizeToRead);
	mNextByte += dataSize;
}

bool OFBinaryMemorySource::isEmpty() const
{
	return mNextByte >= mNumBytes;
}