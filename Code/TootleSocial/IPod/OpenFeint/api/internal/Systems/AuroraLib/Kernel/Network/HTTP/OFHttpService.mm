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

#include "OFHttpService.h"
#include "OFHttpRequest.h"

OFImplementRTTI(OFIHttpRequestUserData, OFSmartObject);

OFHttpService::OFHttpService(OFRetainedPtr<NSString> baseUrl, bool cookies)
: mBaseUrl(baseUrl)
, mHandleCookies(cookies)
{
}

void OFHttpService::startRequest(NSString* path, NSString* httpMethod, NSData* httpBody, OFHttpServiceObserver* requestSpecificObserver)
{
	startRequest(path, httpMethod, httpBody, nil, nil, NULL, nil, requestSpecificObserver);
}

void OFHttpService::startRequest(NSString* path, NSString* httpMethod, NSData* httpBody, NSString* email, NSString* password, OFPointer<OFIHttpRequestUserData> userData, NSString* multiPartBoundary, OFHttpServiceObserver* requestSpecificObserver)
{
	OFRetainedPtr<OFHttpRequest> request = [OFHttpRequest httpRequestWithBase:mBaseUrl withObserver:this withCookies:mHandleCookies];
		
	OFHttpServiceRequestContainer container(request, userData, requestSpecificObserver);
	mRequests.push_back(container);
	
	[request.get() 
		startRequestWithPath:path
		withMethod:httpMethod
		withBody:httpBody
		withEmail:email
		withPassword:password
		multiPartBoundary:multiPartBoundary];
}

void OFHttpService::onFinishedDownloading(OFHttpRequest* info)
{
	RequestContainerSeries::iterator sit = std::find(mRequests.begin(), mRequests.end(), info);
	if(sit == mRequests.end())
	{
		return;
	}
	
	if(sit->getObserver())
	{
		sit->getObserver()->onFinishedDownloading(&(*sit));
	}	
	
	sit = std::find(mRequests.begin(), mRequests.end(), info);
	mRequests.erase(sit);
}

void OFHttpService::onFailedDownloading(OFHttpRequest* info)
{
	RequestContainerSeries::iterator sit = std::find(mRequests.begin(), mRequests.end(), info);
	if(sit == mRequests.end())
	{
		return;
	}
	
	if(sit->getObserver())
	{
		sit->getObserver()->onFailedDownloading(&(*sit));
	}	
	
	sit = std::find(mRequests.begin(), mRequests.end(), info);
	mRequests.erase(sit);
}

bool OFHttpService::hasCookies() const
{
	return [OFHttpRequest hasCookies:mBaseUrl];
}

int OFHttpService::countCookies() const
{
	return [OFHttpRequest countCookies:mBaseUrl];
}

NSArray* OFHttpService::getCookies() const
{
	return [OFHttpRequest getCookies:mBaseUrl];
}

void OFHttpService::addCookies(NSArray* cookies)
{
	return [OFHttpRequest addCookies:cookies withBase:mBaseUrl];
}

void OFHttpService::cancelAllRequests()
{
	RequestContainerSeries::iterator it = mRequests.begin();
	RequestContainerSeries::iterator itEnd = mRequests.end();	
	for(; it != itEnd; ++it)
	{
		OFHttpServiceRequestContainer& request = *it;
		request.cancelImmediately();
	}
}