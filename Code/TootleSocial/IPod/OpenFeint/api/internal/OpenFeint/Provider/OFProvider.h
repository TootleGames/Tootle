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

#import "OFPointer.h"
#import "OFProviderProtocol.h"

class OFDelegate;
class OFReachabilityObserver;
class OFHttpService; 
template <typename _T> class onSmartPointer;
@protocol MPOAuthAPIDelegate;
@class MPOAuthAPIRequestLoader;
@class MPOAuthAPI;

@interface OFProvider : NSObject<OFCallbackable, OFProviderProtocol>
{
@private
	MPOAuthAPI* mOAuthApi;
	OFPointer<OFReachabilityObserver> mReachabilityObserver;
}

+ (id) providerWithProductKey:(NSString*)productKey andSecret:(NSString*)productSecret;
- (id) initWithProductKey:(NSString*)productKey andSecret:(NSString*)productSecret;

+ (OFPointer<OFHttpService>)createHttpService;

- (void)setAccessToken:(NSString*)token andSecret:(NSString*)secret;

- (void) retrieveAccessToken;
- (void) retrieveRequestToken;
- (NSString*) getRequestToken;

- (bool) isAuthenticated;
- (void) loginAndBootstrap:(const OFDelegate&)success onFailure:(const OFDelegate&)failure;
- (void) destroyLocalCredentials;

- (MPOAuthAPIRequestLoader*)getRequestForAction:(NSString*)action 
		withParameters:(NSArray*)parameters 
		withHttpMethod:(NSString*)method 
		withSuccess:(const OFDelegate&)success 
		withFailure:(const OFDelegate&)failure
		withRequestType:(OFActionRequestType)requestType
		withNotice:(NSString*)noticeText
		requiringAuthentication:(bool)requiringAuthentication;

- (void)performAction:(NSString*)action 
		withParameters:(NSArray*)parameters 
		withHttpMethod:(NSString*)method 
		withSuccess:(const OFDelegate&)success 
		withFailure:(const OFDelegate&)failure
		withRequestType:(OFActionRequestType)requestType
		withNotice:(NSString*)noticeText;

- (void)performAction:(NSString*)action 
		withParameters:(NSArray*)parameters 
		withHttpMethod:(NSString*)method 
		withSuccess:(const OFDelegate&)success 
		withFailure:(const OFDelegate&)failure
		withRequestType:(OFActionRequestType)requestType
		withNotice:(NSString*)noticeText
		requiringAuthentication:(bool)requiringAuthentication;
@end
