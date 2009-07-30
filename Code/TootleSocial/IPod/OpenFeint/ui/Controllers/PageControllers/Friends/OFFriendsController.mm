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
#import "OFFriendsController.h"
#import "OFResourceControllerMap.h"
#import "OFFriendsService.h"
#import "OFUser.h"
#import "OFSingleLabelCell.h"
#import "OpenFeint+UserOptions.h"
#import "OFProfileController.h"
#import "OFControllerLoader.h"
#import "OFUsersCredential.h"
#import "OFEmptyFriendsController.h"


@implementation OFFriendsController

@synthesize userId;

- (void)populateResourceMap:(OFResourceControllerMap*)resourceMap
{
	resourceMap->addResource([OFUser class], @"User");
}

- (OFService*)getService
{
	return [OFFriendsService sharedInstance];
}


- (void)doIndexActionWithPage:(NSUInteger)pageIndex onSuccess:(const OFDelegate&)success onFailure:(const OFDelegate&)failure
{
	[OFFriendsService getFriends:userId pageIndex:pageIndex onSuccess:success onFailure:failure];	
}

- (void)doIndexActionOnSuccess:(const OFDelegate&)success onFailure:(const OFDelegate&)failure;
{
	[self doIndexActionWithPage:1 onSuccess:success onFailure:failure];
}

- (bool)localUsersFriends
{
	return userId == nil || [userId isEqualToString:@""] || [userId isEqualToString:[OpenFeint lastLoggedInUserId]];
}

- (UIViewController*)getNoDataFoundViewController
{
	if ([self localUsersFriends])
	{
		alwaysRefresh = YES;
		OFEmptyFriendsController* controller = (OFEmptyFriendsController*)OFControllerLoader::load(@"EmptyFriends");
		controller.owner = self;
		return controller;
	}
	
	return nil;
}

- (NSString*)getNoDataFoundMessage
{
	return [self localUsersFriends] ? @"You have no friends with OpenFeint accounts." : @"This user has no friends with OpenFeint accounts";
}

- (BOOL)shouldAlwaysRefreshWhenShown
{
	return alwaysRefresh;
}

- (NSString*)getProfileUserId
{
	return self.userId;
}

- (void)onLeadingCellWasLoaded:(OFTableCellHelper*)leadingCell forSection:(OFTableSectionDescription*)section
{
	OFSingleLabelCell* singleLabelCell = (OFSingleLabelCell*)leadingCell;
	if (!self.userId || [self.userId isEqualToString:[OpenFeint lastLoggedInUserId]])
	{
		singleLabelCell.label.text = @"My Friends";
	}
	else if (mUser)
	{
		singleLabelCell.label.text = [NSString stringWithFormat:@"%@'s Friends", mUser.name];
	}
	else
	{
		singleLabelCell.label.text = @"Friends";
	}
}

- (void)onProfileUserLoaded:(OFUser*)user
{
	OFSafeRelease(mUser);
	mUser = [user retain];
	[self.tableView reloadData];
}

- (NSString*)getLeadingCellControllerNameForSection:(OFTableSectionDescription*)section
{
	return @"SingleLabel";
}

- (void)onCellWasClicked:(OFResource*)cellResource indexPathInTable:(NSIndexPath*)indexPath
{
	if ([cellResource isKindOfClass:[OFUser class]])
	{
		OFUser* userResource = (OFUser*)cellResource;
		OFProfileController* friendsProfileController = (OFProfileController*)OFControllerLoader::load(@"Profile");
		friendsProfileController.userId = userResource.resourceId;
		[self.navigationController pushViewController:friendsProfileController animated:YES];
	}
}

- (void)dealloc
{
	self.userId = nil;
	OFSafeRelease(mUser);
	[super dealloc];
}

@end

