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

#include "OFBinarySink.h"

OFImplementRTTI(OFBinarySink, OFSmartObject);
OFImplementRTTI(OFBinaryFileSink, OFBinaryMemorySink);
OFImplementRTTI(OFBinaryMemorySink, OFBinaryMemorySink);

OFBinaryFileSink::OFBinaryFileSink(const char* filePath)
{
	mFileStream = fopen(filePath, "wb+");		
}

OFBinaryFileSink::~OFBinaryFileSink()
{
	fclose(mFileStream);
	mFileStream = NULL;
}
	
void OFBinaryFileSink::write(const void* data, unsigned int dataSize)
{
	fwrite(data, dataSize, 1, mFileStream);
}	

OFBinaryMemorySink::OFBinaryMemorySink()
{
}

OFBinaryMemorySink::OFBinaryMemorySink(NSData* data)
: mData(data)
{
}

OFBinaryMemorySink::~OFBinaryMemorySink()
{
}
	
const void* OFBinaryMemorySink::getDataBuffer() const
{
	return [mData.get() bytes];
}

NSData* OFBinaryMemorySink::getNSData() const
{
	return mData.get();
}
	
void OFBinaryMemorySink::write(const void* data, unsigned int dataSize)
{
	mData = [NSData dataWithBytes:data length:dataSize];
}

unsigned int OFBinaryMemorySink::getDataSize() const
{
	return [mData.get() length];
}