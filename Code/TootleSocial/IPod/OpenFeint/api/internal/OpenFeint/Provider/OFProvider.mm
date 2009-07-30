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

#import "OFProvider.h"
#import "MPOAuthAPI.h"
#import "MPURLRequestParameter.h"
#import "OFHttpService.h"
#import "OFActionRequest.h"
#import "OFSettings.h"
#import "OpenFeint+UserOptions.h"
#import "OFDelegateChained.h"
#import "MPOAuthAPIRequestLoader.h"
#import "OFReachabilityObserver.h"
#import "OpenFeint+Private.h"
#import "OFBootstrapService.h"
#import "FBConnect.h"

@implementation OFProvider

- (void)_onReachabilityStatusChanged:(NSNumber*)statusAsInt
{
	NetworkReachability status = (NetworkReachability)[statusAsInt intValue];
	if(status == NotReachable)
	{
		[OpenFeint displayMustBeOnlineErrorMessage];
	}
}

- (id) initWithProductKey:(NSString*)productKey andSecret:(NSString*)productSecret
{
	if(self = [super init])
	{
		NSDictionary* credentials = [NSDictionary dictionaryWithObjectsAndKeys:
			productKey,		kMPOAuthCredentialConsumerKey,
			productSecret,	kMPOAuthCredentialConsumerSecret,
			nil
		];
		
		NSString* apiUrlString = OFSettings::Instance()->getServerUrl();
		NSURL* apiUrl = [NSURL URLWithString:apiUrlString];

		mOAuthApi = [[MPOAuthAPI alloc] initWithCredentials:credentials	andBaseURL:apiUrl];
		
		mOAuthApi.oauthRequestTokenURL		= [NSURL URLWithString:[NSString stringWithFormat:@"%@oauth/request_token", apiUrlString]];
		mOAuthApi.oauthAuthorizeTokenURL	= [NSURL URLWithString:[NSString stringWithFormat:@"%@oauth/authorize", apiUrlString]];
		mOAuthApi.oauthGetAccessTokenURL	= [NSURL URLWithString:[NSString stringWithFormat:@"%@oauth/access_token", apiUrlString]];

		mReachabilityObserver.reset(new OFReachabilityObserver(OFDelegate(self, @selector(_onReachabilityStatusChanged:))));
	}
	
	return self;
}

- (void) dealloc
{
	[mOAuthApi release];
	[super dealloc];
}

+ (id) providerWithProductKey:(NSString*)productKey andSecret:(NSString*)productSecret
{
	return [[[OFProvider alloc] initWithProductKey:productKey andSecret:productSecret] autorelease];
}

- (void) destroyLocalCredentials
{
	[mOAuthApi removeAllCredentials];
	[[FBSession session] logout];
}

+ (OFPointer<OFHttpService>)createHttpService
{
	return new OFHttpService(OFSettings::Instance()->getServerUrl());
}

- (bool) isAuthenticated
{
	return [mOAuthApi isAuthenticated];
}

- (void)actionRequestWithLoader:(MPOAuthAPIRequestLoader*)loader withRequestType:(OFActionRequestType)requestType withNotice:(NSString*)noticeText requiringAuthentication:(bool)requiringAuthentication
{
	OFActionRequest* ofAction = [OFActionRequest actionRequestWithLoader:loader withRequestType:requestType withNotice:noticeText requiringAuthentication:requiringAuthentication];
	[ofAction performSelectorOnMainThread:@selector(dispatch) withObject:nil waitUntilDone:YES];
}

- (void) retrieveAccessToken
{
	MPOAuthAPIRequestLoader* loader = [mOAuthApi createLoaderForAccessToken];	
	[self actionRequestWithLoader:loader withRequestType:OFActionRequestForeground withNotice:@"Finalizing Authentication" requiringAuthentication:false];
}

- (void) retrieveRequestToken
{
	MPOAuthAPIRequestLoader* loader = [mOAuthApi createLoaderForRequestToken];	
	[self actionRequestWithLoader:loader withRequestType:OFActionRequestForeground withNotice:@"Starting Authentication" requiringAuthentication:false];
}

- (NSString*) getRequestToken
{
	return [mOAuthApi getRequestToken];
}

- (MPOAuthAPIRequestLoader*)getRequestForAction:(NSString*)action 
		withParameters:(NSArray*)parameters 
		withHttpMethod:(NSString*)method 
		withSuccess:(const OFDelegate&)success 
		withFailure:(const OFDelegate&)failure
		withRequestType:(OFActionRequestType)requestType
		withNotice:(NSString*)noticeText
		requiringAuthentication:(bool)requiringAuthentication
{
	MPOAuthAPIRequestLoader* loader = 
		[mOAuthApi createLoaderForMethod:action 
									atURL:[NSURL URLWithString:OFSettings::Instance()->getServerUrl()]
						   withParameters:parameters
						   withHttpMethod:method					   
							  withSuccess:success
							  withFailure:failure];
							  
	return loader;
}

- (void)performAction:(NSString*)action 
		withParameters:(NSArray*)parameters 
		withHttpMethod:(NSString*)method 
		withSuccess:(const OFDelegate&)success 
		withFailure:(const OFDelegate&)failure
		withRequestType:(OFActionRequestType)requestType
		withNotice:(NSString*)noticeText
		requiringAuthentication:(bool)requiringAuthentication
{

	if (![self isAuthenticated] && ![OpenFeint hasUserApprovedFeint])
	{
		return;
	}
	MPOAuthAPIRequestLoader* loader = [self 
		getRequestForAction:action
		withParameters:parameters
		withHttpMethod:method
		withSuccess:success
		withFailure:failure
		withRequestType:requestType
		withNotice:noticeText
		requiringAuthentication:requiringAuthentication];
		
	[self actionRequestWithLoader:loader withRequestType:requestType withNotice:noticeText requiringAuthentication:requiringAuthentication];
}

- (void)performAction:(NSString*)action 
		withParameters:(NSArray*)parameters 
		withHttpMethod:(NSString*)method 
		withSuccess:(const OFDelegate&)success 
		withFailure:(const OFDelegate&)failure
		withRequestType:(OFActionRequestType)requestType
		withNotice:(NSString*)noticeText
{
	[self performAction:action
			withParameters:parameters
			withHttpMethod:method
			withSuccess:success
			withFailure:failure
			withRequestType:requestType
			withNotice:noticeText
			requiringAuthentication:true];
}

- (void)setAccessToken:(NSString*)token andSecret:(NSString*)secret
{
	[mOAuthApi setAccessToken:token andSecret:secret];
}

- (bool)canReceiveCallbacksNow
{
	return true;
}

- (void) loginAndBootstrap:(const OFDelegate&)success onFailure:(const OFDelegate&)failure
{
	[OFBootstrapService doBootstrap:success onFailedLoggingIn:failure];
}

@end
