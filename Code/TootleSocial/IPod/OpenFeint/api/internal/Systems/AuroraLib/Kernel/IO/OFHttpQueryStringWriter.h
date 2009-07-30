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
#include <list>
#include "OFStringUtility.h"

class OFHttpQueryStringWriter : public OFOutputSerializer
{
OFDeclareRTTI;
public:
	OFHttpQueryStringWriter();

	bool supportsKeys() const;
			
	NSString* getQueryString();
	NSData* getQueryStringAsData();
	
private:
	void nviIo(OFISerializerKey* keyName, bool& value)							{ mParameters.push_back([NSString stringWithFormat:@"%s=%d", keyName->getAsString(), value ? 1 : 0]); }	
	void nviIo(OFISerializerKey* keyName, int& value)								{ mParameters.push_back([NSString stringWithFormat:@"%s=%d", keyName->getAsString(), value]); }	
	void nviIo(OFISerializerKey* keyName, unsigned int& value)					{ mParameters.push_back([NSString stringWithFormat:@"%s=%d", keyName->getAsString(), value]); }
	void nviIo(OFISerializerKey* keyName, float& value)							{ mParameters.push_back([NSString stringWithFormat:@"%s=%f", keyName->getAsString(), value]); }
	void nviIo(OFISerializerKey* keyName, double& value)							{ mParameters.push_back([NSString stringWithFormat:@"%s=%f", keyName->getAsString(), value]); }
	void nviIo(OFISerializerKey* keyName, std::string& value)
	{
		OFRetainedPtr<NSString> str([NSString stringWithCString:value.c_str()]);
		nviIo(keyName, str);
	}
	
	void nviIo(OFISerializerKey* keyName, OFRetainedPtr<NSString>& value)	
	{
		mParameters.push_back([NSString stringWithFormat:@"%s=%@", keyName->getAsString(), OFStringUtility::convertToValidParameter(value).get()]);		
	}

	const OFRTTI* beginDecodeType();
	void endDecodeType();	
	void beginEncodeType(const OFRTTI* typeToEncode);
	void endEncodeType();
	
	typedef std::list<OFRetainedPtr<NSString> > ParameterList;
	ParameterList mParameters;
};