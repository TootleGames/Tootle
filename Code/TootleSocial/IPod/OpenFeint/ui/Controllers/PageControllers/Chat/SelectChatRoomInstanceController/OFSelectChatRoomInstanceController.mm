/*
 *  OFSelectChatRoomInstanceController.mm
 *  OpenFeint
 *
 *  Created by jakobwilkenson on 3/17/09.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */

#import "OFDependencies.h"
#import "OFSelectChatRoomInstanceController.h"
#import "OFControllerLoader.h"
#import "OFResourceControllerMap.h"
#import "OFChatRoomInstance.h"
#import "OFChatRoomInstanceService.h"
#import "OFChatRoomController.h"
#import "OFDeadEndErrorController.h"

@implementation OFSelectChatRoomInstanceController

@synthesize preLoadedChatRoomInstances;

- (void)populateResourceMap:(OFResourceControllerMap*)resourceMap
{
	resourceMap->addResource([OFChatRoomInstance class], @"ChatRoomInstance");
}

- (OFService*)getService
{
	return [OFChatRoomInstanceService sharedInstance];
}

- (void)onCellWasClicked:(OFResource*)cellResource indexPathInTable:(NSIndexPath*)indexPath
{
	[self showLoadingScreen];
	OFDelegate success(self, @selector(onJoinedRoom));
	OFDelegate failure(self, @selector(onFailedToJoinRoom));
	[OFChatRoomInstanceService attemptToJoinRoom:(OFChatRoomInstance*)cellResource rejoining:NO onSuccess:success onFailure:failure];
}

- (void)doIndexActionOnSuccess:(const OFDelegate&)success onFailure:(const OFDelegate&)failure;
{
	success.invoke(self.preLoadedChatRoomInstances);
}

- (void)onJoinedRoom
{
	[self hideLoadingScreen];
	if (![self isInHiddenTab])
	{
		[OFSelectChatRoomInstanceController pushChatRoom:[OFChatRoomInstanceService getCachedLastRoomJoined] navController:[self navigationController]];
	}
}

- (void)onFailedToJoinRoom
{
	[self hideLoadingScreen];
	[OFSelectChatRoomInstanceController pushRoomFullScreen:[self navigationController]];
}

+ (void)pushChatRoom:(OFChatRoomInstance*)chatRoom navController:(UINavigationController*)navController
{
	OFChatRoomController* chatRoomController = (OFChatRoomController*)OFControllerLoader::load(@"ChatRoom");
	chatRoomController.roomInstance = chatRoom;
	[navController pushViewController:chatRoomController animated:YES];
}

+ (void)pushRoomFullScreen:(UINavigationController*)navController
{
	OFDeadEndErrorController* errorScreen = (OFDeadEndErrorController*)OFControllerLoader::load(@"DeadEndError");
	errorScreen.message = @"The room you attempted to join is full. Please try another room.";
	[navController pushViewController:errorScreen animated:YES];
}

- (NSString*)getTableHeaderControllerName
{
	return nil;
}

- (NSString*)getNoDataFoundMessage
{
	return @"There are no available chat room instances";
}

- (void)dealloc
{
	self.preLoadedChatRoomInstances = nil;
	[super dealloc];
}
@end
