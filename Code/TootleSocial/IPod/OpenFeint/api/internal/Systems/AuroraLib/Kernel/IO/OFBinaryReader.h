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

#include "OFInputSerializer.h"
#include <cstdio>

class OFBinaryReader : public OFInputSerializer
{
OFDeclareRTTI;
public:
	OFBinaryReader(const char* filePath);
	~OFBinaryReader();

	bool supportsKeys() const;
	
private:
	void nviIo(OFISerializerKey* keyName, bool& value)				{ if(mFileStream) fread(&value, sizeof(value), 1, mFileStream); } 
	void nviIo(OFISerializerKey* keyName, int& value)				{ if(mFileStream) fread(&value, sizeof(value), 1, mFileStream); }
	void nviIo(OFISerializerKey* keyName, unsigned int& value)	{ if(mFileStream) fread(&value, sizeof(value), 1, mFileStream); }
	void nviIo(OFISerializerKey* keyName, float& value)			{ if(mFileStream) fread(&value, sizeof(value), 1, mFileStream); }
	void nviIo(OFISerializerKey* keyName, double& value)			{ if(mFileStream) fread(&value, sizeof(value), 1, mFileStream); }
	void nviIo(OFISerializerKey* keyName, std::string& value)
	{
		if(mFileStream) 
		{
			int stringLength = 0;
			fread(&stringLength, sizeof(stringLength), 1, mFileStream);

			std::auto_ptr<char> byteStream(new char[stringLength + 1]);
			byteStream.get()[stringLength] = '\0';
			fread(byteStream.get(), stringLength, 1, mFileStream);
			
			value = std::string(byteStream.get());
		}	
	}

	void nviIo(OFISerializerKey* keyName, OFRetainedPtr<NSString>& value)
	{ 
		if(mFileStream) 
		{
			int stringLength = 0;
			fread(&stringLength, sizeof(stringLength), 1, mFileStream);

			std::auto_ptr<char> byteStream(new char[stringLength + 1]);
			byteStream.get()[stringLength] = '\0';
			fread(byteStream.get(), stringLength, 1, mFileStream);
			
			value.reset([NSString stringWithCString:byteStream.get() encoding:NSStringEncodingConversionExternalRepresentation]);
		}
	}

	const OFRTTI* beginDecodeType();
	void endDecodeType();	
	void beginEncodeType(const OFRTTI* typeToEncode);
	void endEncodeType();

	FILE* mFileStream;
};