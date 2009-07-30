////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///
///  This is beta software and is subject to changes without notice.
///
///  Do not distribute.
///
///  Copyright (c) 2009 Aurora Feint Inc. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#import "OFDependencies.h"
#import "OFSelectChatRoomDefinitionController.h"
#import "OFSelectChatRoomInstanceController.h"
#import "OFControllerLoader.h"
#import "OFResourceControllerMap.h"
#import "OFChatRoomDefinition.h"
#import "OFChatRoomDefinitionService.h"
#import "OFChatRoomInstanceService.h"
#import "OFDeadEndErrorController.h"
#import "OFChatRoomDefinitionHeaderController.h"

@implementation OFSelectChatRoomDefinitionController

@synthesize includeGlobalRooms;
@synthesize includeApplicationRooms;
@synthesize includeDeveloperRooms;
- (void)populateResourceMap:(OFResourceControllerMap*)resourceMap
{
	resourceMap->addResource([OFChatRoomDefinition class], @"ChatRoomDefinition");
}

- (OFService*)getService
{
	return [OFChatRoomDefinitionService sharedInstance];
}

- (BOOL)hideLoadingScreenIfInHiddenTab
{
	if ([self isInHiddenTab])
	{
		[self hideLoadingScreen];
		return YES;
	}
	else
	{
		return NO;
	}
}

- (void)onFailedLoadingInstances
{
	[self hideLoadingScreen];
	OFDeadEndErrorController* errorScreen = (OFDeadEndErrorController*)OFControllerLoader::load(@"DeadEndError");
	errorScreen.message = @"An error occurred when trying to download available rooms. Please try again.";
	[[self navigationController] pushViewController:errorScreen animated:YES];
}

- (void)attemptToJoinRoom:(OFChatRoomInstance*)room
{
	if ([self hideLoadingScreenIfInHiddenTab])
	{
		return;
	}
	OFDelegate success(self, @selector(onJoinedRoom));
	OFDelegate failure(self, @selector(onFailedToJoinRoom));
	[OFChatRoomInstanceService attemptToJoinRoom:room rejoining:NO onSuccess:success onFailure:failure];
}

- (void)onLoadedInstances:(OFResource*)loadedInstances
{
	if ([self hideLoadingScreenIfInHiddenTab])
	{
		return;
	}
	NSArray* resourceArray = (NSArray*)loadedInstances;
	
	if ([resourceArray count] == 0)
	{
		[self onFailedLoadingInstances];
	}
	else if ([resourceArray count] == 1)
	{
		[self attemptToJoinRoom:[resourceArray objectAtIndex:0]];	
	}
	else
	{
		[self hideLoadingScreen];
		OFSelectChatRoomInstanceController* roomInstanceController = (OFSelectChatRoomInstanceController*)OFControllerLoader::load(@"SelectChatRoomInstance");
		roomInstanceController.preLoadedChatRoomInstances = resourceArray;
		[[self navigationController] pushViewController:roomInstanceController animated:YES];
	}
	
}

- (IBAction)onClickedButton
{
	OFChatRoomInstance* lastRoom = [OFChatRoomInstanceService getCachedLastRoomJoined];
	if (lastRoom)
	{
		[self showLoadingScreen];
		[self attemptToJoinRoom:lastRoom];
	}
}

- (void)onCellWasClicked:(OFResource*)cellResource indexPathInTable:(NSIndexPath*)indexPath
{
//	if (!mLoadingScreen)
	{
		
	[self showLoadingScreen];
	OFDelegate success(self, @selector(onLoadedInstances:));
	OFDelegate failure(self, @selector(onFailedLoadingInstances));
	[OFChatRoomInstanceService getPage:1 forChatRoomDefinition:(OFChatRoomDefinition*)cellResource onSuccess:success onFailure:failure];		
	}
}

- (void)onJoinedRoom
{
	if ([self hideLoadingScreenIfInHiddenTab])
	{
		return;
	}
	[self hideLoadingScreen];
	[OFSelectChatRoomInstanceController pushChatRoom:[OFChatRoomInstanceService getCachedLastRoomJoined] navController:[self navigationController]];
}

- (void)onFailedToJoinRoom
{
	[self hideLoadingScreen];
	[OFSelectChatRoomInstanceController pushRoomFullScreen:[self navigationController]];
}

- (NSString*)getTableHeaderControllerName
{
	return @"ChatRoomDefinitionHeader";
}

- (NSString*)getNoDataFoundMessage
{
	return @"There are no available chat rooms";
}

- (void)onTableHeaderCreated:(UIViewController*)tableHeader
{	
	if (!includeGlobalRooms && includeApplicationRooms && includeDeveloperRooms)
	{
		[(OFChatRoomDefinitionHeaderController*)tableHeader hideFeintLogo];
	}
}

- (void)doIndexActionOnSuccess:(const OFDelegate&)success onFailure:(const OFDelegate&)failure
{
	[OFChatRoomDefinitionService getPage:1 
							 includeGlobalRooms:self.includeGlobalRooms 
						  includeDeveloperRooms:self.includeDeveloperRooms 
						includeApplicationRooms:self.includeApplicationRooms 
									  onSuccess:success 
									  onFailure:failure];
}

- (void)viewWillAppear:(BOOL)animated
{
	[super viewWillAppear:animated];
	OFChatRoomDefinitionHeaderController* header = (OFChatRoomDefinitionHeaderController*)mTableHeaderController;
	if (header)
	{
		[header updateBeforeDisplaying];
	}
}

- (void)dealloc
{
	[super dealloc];
}
@end
