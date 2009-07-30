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

#include "OFSmartObject.h"
#include "OFHttpServiceObserver.h"
#include "OFHttpRequestObserver.h"
#include "OFHttpServiceRequestContainer.h"

class OFHttpService : public OFSmartObject, public OFHttpRequestObserver
{
public:
	OFHttpService(OFRetainedPtr<NSString> baseUrl, bool cookies = false);

	virtual void startRequest(NSString* path, NSString* httpMethod, NSData* httpBody, NSString* email, NSString* password, OFPointer<OFIHttpRequestUserData> userData, NSString* multiPartBoundary, OFHttpServiceObserver* requestSpecificObserver);
	virtual void startRequest(NSString* path, NSString* httpMethod, NSData* httpBody, OFHttpServiceObserver* requestSpecificObserver);

	// Note: No callbacks are invoked on any observers. All outstanding request are just closed
	virtual void cancelAllRequests();

	void onFinishedDownloading(OFHttpRequest* info);
	void onFailedDownloading(OFHttpRequest* info);	
	
	bool handlesCookies() const { return mHandleCookies; }
	void setHandlesCookies(bool cookies) { mHandleCookies = cookies; }
	bool hasCookies() const;
	int countCookies() const;
	NSArray* getCookies() const;
	void addCookies(NSArray* cookies);
		
private:
	OFRetainedPtr<NSString> mBaseUrl;
	bool mHandleCookies;
	
	typedef std::vector<OFHttpServiceRequestContainer> RequestContainerSeries;
	RequestContainerSeries mRequests;
};