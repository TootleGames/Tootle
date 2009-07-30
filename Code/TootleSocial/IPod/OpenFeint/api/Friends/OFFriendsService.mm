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
#import "OFFriendsService.h"
#import "OFHttpNestedQueryStringWriter.h"
#import "OFService+Private.h"
#import "OFUser.h"
#import "OFGamePlayer.h"
#import "OFRetainedPtr.h"
#import "OFUsersCredential.h"

OPENFEINT_DEFINE_SERVICE_INSTANCE(OFFriendsService)

@implementation OFFriendsService

OPENFEINT_DEFINE_SERVICE(OFFriendsService);

- (void)populateKnownResources:(OFResourceNameMap*)namedResources
{
	namedResources->addResource([OFUser getResourceName], [OFUser class]);
	namedResources->addResource([OFGamePlayer getResourceName], [OFGamePlayer class]);
	namedResources->addResource([OFUsersCredential getResourceName], [OFUsersCredential class]);
	
}

+ (void)getFriends:(NSString*)ownersUserId pageIndex:(NSInteger)pageIndex onSuccess:(const OFDelegate&)onSuccess onFailure:(const OFDelegate&)onFailure
{
	OFPointer<OFHttpNestedQueryStringWriter> params = new OFHttpNestedQueryStringWriter;
	bool displayAsFriends = true;
	
	params->io("display_as_friends", displayAsFriends);
	params->io("page", pageIndex);
	params->io("user_id", ownersUserId ? ownersUserId : @"me");
	
	
	[[self sharedInstance] 
	 getAction:@"followings.xml"
	 withParameters:params
	 withSuccess:onSuccess
	 withFailure:onFailure
	 withRequestType:OFActionRequestForeground
	 withNotice:@"Downloading Friends"];
}

+ (void)getFriendsPlayingApp:(NSString*)applicationId forUser:(NSString*)ownersUserId pageIndex:(NSInteger)pageIndex onSuccess:(const OFDelegate&)onSuccess onFailure:(const OFDelegate&)onFailure
{
	OFPointer<OFHttpNestedQueryStringWriter> params = new OFHttpNestedQueryStringWriter;
	bool displayAsFriends = true;
	
	params->io("display_as_friends", displayAsFriends);
	params->io("page", pageIndex);
	params->io("user_id", ownersUserId ? ownersUserId : @"me");
	params->io("by_app", applicationId);

	[[self sharedInstance] 
	 getAction:@"followings.xml"
	 withParameters:params
	 withSuccess:onSuccess
	 withFailure:onFailure
	 withRequestType:OFActionRequestForeground
	 withNotice:@"Downloading Friends"];
}

@end