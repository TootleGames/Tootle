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
#import "OFAchievementListController.h"
#import "OFResourceControllerMap.h"
#import "OFControllerLoader.h"
#import "OFProfileController.h"
#import "OFAchievementService.h"
#import "OFAchievement.h"
#import "OFAchievementDetailController.h"
#import "OFPlayedGame.h"
#import "OFUserGameStat.h"
#import "OpenFeint+Settings.h"
#import "OpenFeint+UserOptions.h"
#import "OFComparisonLeadingCell.h"
#import "OFUser.h"
#import "OFApplicationDescriptionController.h"

@implementation OFAchievementListController

@synthesize userId, applicationName, applicationId, applicationIconUrl, doesUserHaveApplication;

- (IBAction)onGetGame
{
	OFApplicationDescriptionController* iPromoteController = [OFApplicationDescriptionController applicationDescriptionForId:applicationId];
	[self.navigationController pushViewController:iPromoteController animated:YES];
}

- (void)dealloc
{
	self.applicationName = nil;
	self.applicationId = nil;
	self.applicationIconUrl = nil;
	self.userId = nil;
	OFSafeRelease(mUser);
	[super dealloc];
}

- (void)populateResourceMap:(OFResourceControllerMap*)resourceMap
{
	resourceMap->addResource([OFAchievement class], @"AchievementList");
}

- (OFService*)getService
{
	return [OFAchievementService sharedInstance];
}

- (void)onCellWasClicked:(OFResource*)cellResource indexPathInTable:(NSIndexPath*)indexPath
{
	// OF2.0UI
//	OFAchievementDetailController* detailController = (OFAchievementDetailController*)OFControllerLoader::load(@"AchievementDetail");
//	detailController.userId = userId;
//	detailController.achievement = (OFAchievement*)cellResource;
//	[[self navigationController] pushViewController:detailController animated:YES];
}

- (NSString*)getNoDataFoundMessage
{
	return [NSString stringWithFormat:@"There are no achievements for %@", applicationName];
}

- (void)doIndexActionWithPage:(unsigned int)oneBasedPageNumber onSuccess:(const OFDelegate&)success onFailure:(const OFDelegate&)failure
{
	[OFAchievementService getAchievementsForApplication:applicationId 
										 comparedToUser:userId 
												   page:oneBasedPageNumber
											  onSuccess:success 
											  onFailure:failure];	
}

- (void)doIndexActionOnSuccess:(const OFDelegate&)success onFailure:(const OFDelegate&)failure;
{
	[self doIndexActionWithPage:1 onSuccess:success onFailure:failure];
}

- (void)populateContextualDataFromPlayedGame:(OFPlayedGame*)playedGame
{
	self.applicationName = playedGame.name;
	self.applicationId = [NSString stringWithFormat:@"%d", playedGame.applicationId];
	self.applicationIconUrl = playedGame.iconUrl;
	for (OFUserGameStat* gameStat in playedGame.userGameStats)
	{
		if ([gameStat.userId isEqualToString:[OpenFeint lastLoggedInUserId]])
		{
			self.doesUserHaveApplication = gameStat.userHasGame;
		}
	}
}

- (NSString*)getProfileUserId
{
	return userId;
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
		OFComparisonLeadingCell* comparisonCell = (OFComparisonLeadingCell*)leadingCell;
		NSString* header = [NSString stringWithFormat:@"%@ Achievements", applicationName];
		[comparisonCell populate:mUser header:header leftIconUrl:applicationIconUrl leftIconDefaultImage:@"OFDefaultApplicationIcon.png"];
	}
}

- (NSString*)getLeadingCellControllerNameForSection:(OFTableSectionDescription*)section
{
	return @"ComparisonLeading";
}

- (NSString*)getTableHeaderControllerName
{
	return (doesUserHaveApplication || !self.applicationIconUrl)  ? nil : @"AchievementListGetGameHeader";
}

@end