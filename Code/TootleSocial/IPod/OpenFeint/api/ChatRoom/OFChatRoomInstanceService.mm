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
#import "OFChatRoomInstanceService.h"
#import "OFHttpNestedQueryStringWriter.h"
#import "OFService+Private.h"
#import "OFChatRoomDefinition.h"
#import "OFChatRoomInstance.h"
#import "OFChatMessageService.h"
#import "OFDelegateChained.h"

OPENFEINT_DEFINE_SERVICE_INSTANCE(OFChatRoomInstanceService);

@implementation OFChatRoomInstanceService

@synthesize lastRoom = mLastRoom;
@synthesize roomJoining = mRoomJoining;
@synthesize rejoiningRoom = mRejoiningRoom;

OPENFEINT_DEFINE_SERVICE(OFChatRoomInstanceService);

- (void) populateKnownResources:(OFResourceNameMap*)namedResources
{
	namedResources->addResource([OFChatRoomInstance getResourceName], [OFChatRoomInstance class]);
}

- (CFAbsoluteTime)getTimeSinceLastRoomUpdated
{
	return CFAbsoluteTimeGetCurrent() - mLastUpdateOfLastRoom;
}

+ (void) getPage:(NSInteger)pageIndex forChatRoomDefinition:(OFChatRoomDefinition*)roomDefinition onSuccess:(const OFDelegate&)onSuccess onFailure:(const OFDelegate&)onFailure
{
	OFPointer<OFHttpNestedQueryStringWriter> params = new OFHttpNestedQueryStringWriter;
	OFRetainedPtr<NSString> resourceId = roomDefinition.resourceId;
	params->io("chat_room_definition_id", resourceId);
	params->io("page", pageIndex);
	
	
	[[self sharedInstance] 
	 getAction:@"chat_room_instances.xml"
	 withParameters:params
	 withSuccess:onSuccess
	 withFailure:onFailure
	 withRequestType:OFActionRequestForeground
	 withNotice:@"Downloading Room Instances"];
}

+ (void) attemptToJoinRoom:(OFChatRoomInstance*)roomToJoin 
				 rejoining:(BOOL)rejoining  
				 onSuccess:(const OFDelegate&)onSuccess 
				 onFailure:(const OFDelegate&)onFailure
{
	OFDelegate chainedSuccessDelegate([self sharedInstance], @selector(_onJoinedChatRoom:nextCall:), onSuccess);
	OFPointer<OFHttpNestedQueryStringWriter> params = new OFHttpNestedQueryStringWriter;
	OFRetainedPtr<NSString> resourceId = roomToJoin.resourceId;
	params->io("chat_room_instance_id", resourceId);
	
	[self sharedInstance].roomJoining = roomToJoin;
	[self sharedInstance].rejoiningRoom = rejoining;
	
	[[self sharedInstance] 
	 getAction:@"chat_room_instances/join"
	 withParameters:params
	 withSuccess:chainedSuccessDelegate
	 withFailure:onFailure
	 withRequestType:OFActionRequestForeground
	 withNotice:@"Joining Chat Room"];
}

+ (void) loadLastRoomJoined:(const OFDelegate&)onSuccess onFailure:(const OFDelegate&)onFailure
{
	CFTimeInterval timeSinceUpdated = [[self sharedInstance]getTimeSinceLastRoomUpdated];
	if (timeSinceUpdated < 10.f)
	{
		onSuccess.invoke([self sharedInstance].lastRoom);
	}
	else
	{
		OFDelegate chainedSuccessDelegate([self sharedInstance], @selector(_onLoadedLastJoinedChatRoom:nextCall:), onSuccess);
		
		[[self sharedInstance] 
		 getAction:@"chat_room_instances/show"
		 withParameters:nil
		 withSuccess:chainedSuccessDelegate
		 withFailure:onFailure
		 withRequestType:OFActionRequestSilent
		 withNotice:@"Loading Last Room"];
	}
}

+ (OFChatRoomInstance*) getCachedLastRoomJoined
{
	return [self sharedInstance].lastRoom;
}

- (void)_onJoinedChatRoom:(NSObject*)param nextCall:(OFDelegateChained*)nextCall
{
	if (!self.rejoiningRoom)
	{
		[OFChatMessageService clearCacheAndPollNow];
	}
	self.lastRoom = self.roomJoining;
	self.roomJoining = nil;
	self.rejoiningRoom = NO;
	mLastUpdateOfLastRoom = CFAbsoluteTimeGetCurrent();
	[nextCall invokeWith:param];
}

- (void)_onLoadedLastJoinedChatRoom:(NSArray*)roomArray nextCall:(OFDelegateChained*)nextCall
{
	mLastUpdateOfLastRoom = CFAbsoluteTimeGetCurrent();
	if ([roomArray count] == 1)
	{
		self.lastRoom = [roomArray objectAtIndex:0];
	}
	else
	{
		self.lastRoom = nil;
	}
	[nextCall invokeWith:self.lastRoom];
}




@end
