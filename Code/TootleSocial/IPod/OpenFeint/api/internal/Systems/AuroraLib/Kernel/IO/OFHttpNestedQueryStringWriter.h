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
#include "OFHttpQueryParameter.h"

class OFHttpNestedQueryStringWriter : public OFOutputSerializer
{
OFDeclareRTTI;
public:
	OFHttpNestedQueryStringWriter();

	bool supportsKeys() const;
	
	NSArray* getQueryParameters() const;
	NSArray* getQueryParametersAsMPURLRequestParameters() const;
	NSString* getQueryString() const;
		
private:
	void nviIo(OFISerializerKey* keyName, bool& value)						{ addAsciiParameter(formatScoped(keyName), [NSString stringWithFormat:@"%d", value ? 1 : 0]); }	
	void nviIo(OFISerializerKey* keyName, int& value)							{ addAsciiParameter(formatScoped(keyName), [NSString stringWithFormat:@"%d", value]); }	
	void nviIo(OFISerializerKey* keyName, unsigned int& value)				{ addAsciiParameter(formatScoped(keyName), [NSString stringWithFormat:@"%d", value]); }
	void nviIo(OFISerializerKey* keyName, int64_t& value)						{ addAsciiParameter(formatScoped(keyName), [NSString stringWithFormat:@"%qi", value]); }
	void nviIo(OFISerializerKey* keyName, float& value)						{ addAsciiParameter(formatScoped(keyName), [NSString stringWithFormat:@"%f", value]); }
	void nviIo(OFISerializerKey* keyName, double& value)						{ addAsciiParameter(formatScoped(keyName), [NSString stringWithFormat:@"%f", value]); }
	void nviIo(OFISerializerKey* keyName, std::string& value)
	{
		OFRetainedPtr<NSString> str([NSString stringWithCString:value.c_str()]);
		nviIo(keyName, str);
	}
	
	void nviIo(OFISerializerKey* keyName, OFRetainedPtr<NSData>& value)
	{
		addBlobParameter([NSString stringWithCString:keyName->getAsString()], value);
	}
	
	void nviIo(OFISerializerKey* keyName, OFRetainedPtr<NSString>& value)	
	{
		addAsciiParameter(
			formatScoped(keyName), 
			OFStringUtility::convertToValidParameter(value).get()
		);
	}

	void addAsciiParameter(NSString* name, NSString* value);
	void addBlobParameter(NSString* name, NSData* value);
	void serializeNumElementsInScopeSeries(unsigned int& count);
	
	const OFRTTI* beginDecodeType();
	void endDecodeType();	
	void beginEncodeType(const OFRTTI* typeToEncode);
	void endEncodeType();
	
	NSString* formatScoped(OFISerializerKey* keyName);
	NSMutableString* getCurrentScope();

	OFPointer<OFISerializer> createSerializerForInnerStreamOfType(const OFRTTI* type);
	void storeInnerStreamOfType(OFISerializer* innerResourceStream, const OFRTTI* type);
	
	typedef std::vector< OFPointer<OFHttpQueryParameter> > ParameterList;
	ParameterList mParameters;
};