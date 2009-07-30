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

#import "OFDependencies.h"
#import "OFService+Private.h"
#import "OFService+Overridables.h"
#import "OFProvider.h"
#import "OpenFeint+Private.h"
#import "OFDelegateChained.h"
#import "MPOAuthAPIRequestLoader.h"
#import "OFResource.h"
#import "OFXmlDocument.h"
#import "OFHttpNestedQueryStringWriter.h"
#import "OFDelegate.h"
#import "OFPaginatedSeries.h"

@implementation OFService ( Private )

- (void)_onActionFailed:(MPOAuthAPIRequestLoader*)request nextCall:(OFDelegateChained*)next
{
	[next invoke];
}

- (void)_onActionSucceeded:(MPOAuthAPIRequestLoader*)request nextCall:(OFDelegateChained*)next
{
	[next invokeWith:[OFResource resourcesFromXml:[OFXmlDocument xmlDocumentWithData:request.data] withMap:[self getKnownResources]]];
}

- (void)_performAction:(NSString*)action
		withParameters:(OFHttpNestedQueryStringWriter*)params
		withHttpMethod:(NSString*)httpMethod
		withSuccess:(const OFDelegate&)onSuccess 
		withFailure:(const OFDelegate&)onFailure
		withRequestType:(OFActionRequestType)requestType
		withNotice:(NSString*)notice
{
	[self _performAction:action
		withParameters:params
		withHttpMethod:httpMethod
		withSuccess:onSuccess
		withFailure:onFailure
		withRequestType:requestType
		withNotice:notice
		requiringAuthentication:true];
}

- (void)_performAction:(NSString*)action
		withParameters:(OFHttpNestedQueryStringWriter*)params
		withHttpMethod:(NSString*)httpMethod
		withSuccess:(const OFDelegate&)onSuccess 
		withFailure:(const OFDelegate&)onFailure
		withRequestType:(OFActionRequestType)requestType
		withNotice:(NSString*)notice
		requiringAuthentication:(bool)requiringAuthentication
{
	[[OpenFeint provider] 
		performAction:action
		withParameters:(params ? params->getQueryParametersAsMPURLRequestParameters() : nil)
		withHttpMethod:httpMethod
		withSuccess:OFDelegate(self, @selector(_onActionSucceeded:nextCall:), onSuccess)
		withFailure:OFDelegate(self, @selector(_onActionFailed:nextCall:), onFailure)
		withRequestType:requestType
		withNotice:notice
		requiringAuthentication:requiringAuthentication];
}


- (void)getAction:(NSString*)action
		withParameters:(OFHttpNestedQueryStringWriter*)params
		withSuccess:(const OFDelegate&)onSuccess 
		withFailure:(const OFDelegate&)onFailure
		withRequestType:(OFActionRequestType)requestType
		withNotice:(NSString*)notice
{
	[self 
		_performAction:action
		withParameters:params
		withHttpMethod:@"GET"
		withSuccess:onSuccess
		withFailure:onFailure
		withRequestType:requestType
		withNotice:notice];
}

- (void)postAction:(NSString*)action
		withParameters:(OFHttpNestedQueryStringWriter*)params
		withSuccess:(const OFDelegate&)onSuccess 
		withFailure:(const OFDelegate&)onFailure
		withRequestType:(OFActionRequestType)requestType
		withNotice:(NSString*)notice
{
	[self 
		_performAction:action
		withParameters:params
		withHttpMethod:@"POST"
		withSuccess:onSuccess
		withFailure:onFailure
		withRequestType:requestType
		withNotice:notice];
}

- (void)putAction:(NSString*)action
		withParameters:(OFHttpNestedQueryStringWriter*)params
		withSuccess:(const OFDelegate&)onSuccess 
		withFailure:(const OFDelegate&)onFailure
		withRequestType:(OFActionRequestType)requestType
		withNotice:(NSString*)notice
{
	OFRetainedPtr<NSString> fakeMethod = @"put";
	params->io("_method", fakeMethod);
	
	[self 
		_performAction:action
		withParameters:params
		withHttpMethod:@"POST"
		withSuccess:onSuccess
		withFailure:onFailure
		withRequestType:requestType
		withNotice:notice];
}

@end
