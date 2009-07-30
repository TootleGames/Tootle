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

#import <UIKit/UIKit.h>

class OFHttpRequestObserver;

// If these are declared as static const, we get tons of "defined but not used" warnings
#define HttpMethodPost @"POST"
#define HttpMethodGet @"GET"
#define HttpMethodPut @"PUT"
#define HttpMethodDelete @"DELETE"


@interface OFHttpRequest : NSObject
{
	NSString* mRequestPath;
	NSString* mRequestMethod;
	NSHTTPURLResponse* mHttpResponse;
	NSMutableData* mReceivedData;
	NSURLConnection* mURLConnection;
	NSString* mBaseUrl;
	OFRetainedPtr<NSString> mPassword;
	OFRetainedPtr<NSString> mEmail;
	bool mIsRequestInProgress;
	
	OFHttpRequestObserver* mObserver;
	
	bool mHandleCookies;
}

+ (id)httpRequestWithBase:(NSString*)urlBase withObserver:(OFHttpRequestObserver*)observer;
+ (id)httpRequestWithBase:(NSString*)urlBase withObserver:(OFHttpRequestObserver*)observer withCookies:(bool)cookies;
- (id)initWithBaseUrl:(NSString*)url withObserver:(OFHttpRequestObserver*)observer;
- (id)initWithBaseUrl:(NSString*)url withObserver:(OFHttpRequestObserver*)observer withCookies:(bool)cookies;
- (void)startRequestWithPath:(NSString*)path withMethod:(NSString*)httpMethod withBody:(NSData*)httpBody;
- (void)startRequestWithPath:(NSString*)path withMethod:(NSString*)httpMethod withBody:(NSData*)httpBody withEmail:(NSString*)email withPassword:(NSString*)password multiPartBoundary:(NSString*)multiPartBoundary;
- (void)changeObserver:(OFHttpRequestObserver*)newObserver;

- (void)cancelImmediately;

+ (bool)hasCookies:urlBase;
+ (int)countCookies:urlBase;
+ (NSArray*)getCookies:urlBase;
+ (void)addCookies:(NSArray*)cookies withBase:urlBase;
- (bool)hasCookies;
- (int)countCookies;
- (NSArray*)getCookies;
- (void)addCookies:(NSArray*)cookies;

@property(nonatomic, readonly) NSString* urlPath;
@property(nonatomic, readonly) NSString* httpMethod;
@property(nonatomic, readonly) NSURLConnection* url;
@property(nonatomic, readonly) NSData* data;
@property(nonatomic, readonly) NSString* contentType;
@property(nonatomic, readonly) NSString* contentDisposition;

- (void)_releaseConnectionResources;
@end