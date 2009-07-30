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
#import "OFPlayedGameController.h"
#import "OFResourceControllerMap.h"
#import "OFPlayedGame.h"
#import "OFProfileService.h"
#import "OFTableSequenceControllerHelper+Overridables.h"
#import "OFAchievementListController.h"
#import "OFControllerLoader.h"
#import "OpenFeint+UserOptions.h"
#import "OFUserGameStat.h"
#import "OFComparisonLeadingCell.h"
#import "OFUser.h"

@implementation OFPlayedGameController

@synthesize userId;

- (void)viewWillAppear:(BOOL)animated
{
	[super viewWillAppear:animated];	
}

- (void)populateResourceMap:(OFResourceControllerMap*)resourceMap
{
	resourceMap->addResource([OFPlayedGame class], @"PlayedGame");
}

- (OFService*)getService
{
	return [OFProfileService sharedInstance];
}

- (NSString*)getNoDataFoundMessage
{
	return @"Failed to download games list";
}

- (void)doIndexActionOnSuccess:(const OFDelegate&)success onFailure:(const OFDelegate&)failure
{
	[self doIndexActionWithPage:1 onSuccess:success onFailure:failure];
}

- (void)doIndexActionWithPage:(unsigned int)oneBasedPageNumber onSuccess:(const OFDelegate&)success onFailure:(const OFDelegate&)failure
{
	[OFProfileService getPlayedGamesForUser:userId withPage:oneBasedPageNumber andCountPerPage:10 onSuccess:success onFailure:failure];	
}

- (void)onCellWasClicked:(OFResource*)cellResource indexPathInTable:(NSIndexPath*)indexPath
{
	if ([cellResource isKindOfClass:[OFPlayedGame class]])
	{
		OFPlayedGame* playedGameResource = (OFPlayedGame*)cellResource;
		OFAchievementListController* achievementsController = (OFAchievementListController*)OFControllerLoader::load(@"AchievementList");
		achievementsController.userId = [self.userId isEqualToString:[OpenFeint lastLoggedInUserId]] ? nil : self.userId;
		[achievementsController populateContextualDataFromPlayedGame:playedGameResource];
		[self.navigationController pushViewController:achievementsController animated:YES];
	}
}

- (NSString*)getProfileUserId
{
	return self.userId;
}

- (void)populateLeadingCell:(OFComparisonLeadingCell*)leadingCell withUser:(OFUser*)user
{
	bool myPage = !user || [user.resourceId isEqualToString:[OpenFeint lastLoggedInUserId]];
	if(myPage)
	{
		self.title = @"My Games";
	}
	NSString* header = myPage ? @"My Games" : [NSString stringWithFormat:@"%@'s Games", user.name];
	[leadingCell populate:user header:header leftIconUrl:nil leftIconDefaultImage:nil];
}

- (void)onProfileUserLoaded:(OFUser*)user
{
	OFSafeRelease(mUser);
	mUser = [user retain];
	[self.tableView reloadData];
}

- (void)onLeadingCellWasLoaded:(OFTableCellHelper*)leadingCell forSection:(OFTableSectionDescription*)section
{
	if (mUser)
	{
		[self populateLeadingCell:(OFComparisonLeadingCell*)leadingCell withUser:mUser];
	}
}

- (NSString*)getLeadingCellControllerNameForSection:(OFTableSectionDescription*)section
{
	return @"ComparisonLeading";
}

- (void)dealloc
{
	self.userId = nil;
	OFSafeRelease(mUser);
	[super dealloc];
}

@end
