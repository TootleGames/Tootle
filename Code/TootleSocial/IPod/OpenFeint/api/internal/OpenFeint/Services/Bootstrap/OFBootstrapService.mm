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
#import "OFBootstrapService.h"
#import "OFHttpNestedQueryStringWriter.h"
#import "OFService+Private.h"
#import "OFBootstrap.h"
#import "OFPoller.h"
#import "OpenFeint+Private.h"
#import "OFDelegateChained.h"
#import "OFResourceNameMap.h"
#import "MPOAuthAPIRequestLoader.h"
#import "OpenFeint+Settings.h"
#import "OpenFeint+UserOptions.h"
#import "OFXmlDocument.h"
#import "OFProvider.h"

OPENFEINT_DEFINE_SERVICE_INSTANCE(OFBootstrapService);

@implementation OFBootstrapService

OPENFEINT_DEFINE_SERVICE(OFBootstrapService);

- (id) init
{
	if(self = [super init])
	{
	}
	return self;
}

- (void)dealloc
{
	[super dealloc];
}

- (void)registerPolledResources:(OFPoller*)poller
{
}

- (void) populateKnownResources:(OFResourceNameMap*)namedResources
{
	namedResources->addResource([OFBootstrap getResourceName], [OFBootstrap class]);
}

+ (void) doBootstrap:(const OFDelegate&)onSuccess onFailedLoggingIn:(const OFDelegate&)onFailure
{
	OFPointer<OFHttpNestedQueryStringWriter> params = new OFHttpNestedQueryStringWriter;
	OFRetainedPtr<NSString> udid = [UIDevice currentDevice].uniqueIdentifier;
	params->io("udid", udid);	
	

	[[self sharedInstance]
		_performAction:@"bootstrap.xml"
		 withParameters:params
		 withHttpMethod:@"POST"
		 withSuccess:OFDelegate([self sharedInstance], @selector(_onReceivedConfiguration:nextCall:), onSuccess)
		 withFailure:onFailure
		 withRequestType:OFActionRequestSilent
		 withNotice:nil 
		 requiringAuthentication:false];
}

- (void)_onReceivedConfiguration:(NSArray*)resources nextCall:(OFDelegateChained*)chainedDelegate
{
	if([resources count] == 0)
	{
		return;
	}
	
	OFBootstrap* bootstrap = (OFBootstrap*)[resources objectAtIndex:0];
	[OpenFeint storePollingFrequencyDefault:bootstrap.pollingFrequencyDefault];
	[OpenFeint storePollingFrequencyInChat:bootstrap.pollingFrequencyInChat];
	[OpenFeint setLastLoggedInUserId:bootstrap.loggedInUserId];
	[OpenFeint setLastLoggedInUserName:bootstrap.loggedInUserName];	
	[OpenFeint setLastLoggedInUserProfilePictureUrl:bootstrap.loggedInUserProfilePictureUrl];
	[OpenFeint setLastLoggedInUserUsesFacebookProfilePicture:bootstrap.loggedInUserUsesFacebookProfilePicture];
	[[OpenFeint provider] setAccessToken:bootstrap.accessToken andSecret:bootstrap.accessTokenSecret];
	[OpenFeint setLoggedInUserHasSetName:bootstrap.loggedInUserHasSetName];
	[OpenFeint setLoggedInUserHadFriendsOnBootup:bootstrap.loggedInUserHadFriendsOnBootup];
	[OpenFeint setLoggedInUserHasNonDeviceCredential:bootstrap.loggedInUserHasNonDeviceCredential];
	[OpenFeint setLoggedInUserIsNewUser:bootstrap.loggedInUserIsNewUser];
	[OpenFeint setClientApplicationId:bootstrap.clientApplicationId];
	[OpenFeint setClientApplicationIconUrl:bootstrap.clientApplicationIconUrl];
	
	[chainedDelegate invoke];
}

@end
