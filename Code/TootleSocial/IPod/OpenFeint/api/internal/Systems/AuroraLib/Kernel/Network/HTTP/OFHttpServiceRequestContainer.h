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

#include "OFHttpRequest.h"
#include "OFIHttpRequestUserData.h"

class OFHttpServiceRequestContainer : public OFSmartObject
{
public:
	OFHttpServiceRequestContainer(OFHttpRequest* request_, OFIHttpRequestUserData* userData, OFHttpServiceObserver* requestSpecificObserver_)
	: mUrlPath(request_.urlPath)
	, mHttpMethod(request_.httpMethod)
	, mUserData(userData)
	, mRequestSpecificObserver(requestSpecificObserver_)
	, mRequest(request_)
	{
	}
		
	void forceSetUrlPath(NSString* urlPath)					{ mUrlPath = urlPath; }
	void forceSetHttpMethod(NSString* httpMethod)			{ mHttpMethod = httpMethod; }
	void forceSetContentType(NSString* contentTypeToUse)	{ mContentType = contentTypeToUse; }
	void forceSetData(NSData* dataToUse)					{ mData = dataToUse; }

	OFIHttpRequestUserData* getUserData()			{ return mUserData.get(); }
	OFHttpServiceObserver* getObserver()			{ return mRequestSpecificObserver; }
	NSData* getData() const						{ return mData.get() ? mData.get() : mRequest.get().data; }
	NSString* getContentType() const			{ return mContentType.get() ? mContentType.get() : mRequest.get().contentType; }
	NSString* getContentDisposition() const		{ return mRequest.get().contentDisposition; }
	NSString* getUrlPath() const				{ return mUrlPath.get(); }
	NSString* getHttpMethod() const				{ return mHttpMethod.get(); }
	
	void cancelImmediately()					{ [mRequest.get() cancelImmediately]; }
	
	bool operator==(OFHttpRequest* rhv) const { return mRequest.get() == rhv; }

private:
	OFRetainedPtr<OFHttpRequest> mRequest;	
	OFRetainedPtr<NSString> mContentType;
	OFRetainedPtr<NSData> mData;
	OFRetainedPtr<NSString> mUrlPath;
	OFRetainedPtr<NSString> mHttpMethod;
	OFPointer<OFIHttpRequestUserData> mUserData;
	OFHttpServiceObserver* mRequestSpecificObserver;
};