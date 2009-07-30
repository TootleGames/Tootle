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
#import "OFProfileController.h"
#import "OpenFeint.h"
#import "OFViewHelper.h"
#import "OFControllerLoader.h"
#import "OFProfileService.h"
#import "OFPlayedGamesProfileHeaderView.h"
#import "OFPlayedGame.h"
#import "OFPlayedGameController.h"
#import "OFFriendsController.h"
#import "OFUser.h"
#import "OFImageView.h"
#import "OpenFeint+UserOptions.h"
#import "OpenFeint+Private.h"
#import "OFReportAbuseController.h"
#import "OFSingleLabelCell.h"
#import "OFTableSectionDescription.h"
#import "OFAchievementListController.h"
#import "OFPaginatedSeries.h"
#import "OFPaginatedSeriesHeader.h"
#import "OFColors.h"
#import "OFProfileNoFriendsHeaderView.h"
#import "OFUsersCredential.h"

@implementation OFProfileController

@synthesize userId = mUserId;

+ (void)showProfileForUser:(NSString*)userId
{
	UINavigationController* currentNavController = [OpenFeint getActiveNavigationController];
	if (currentNavController)
	{
		BOOL shouldOpenProfile = YES;

		if ([currentNavController.visibleViewController isKindOfClass:[OFProfileController class]])
		{
			OFProfileController* currentProfile = (OFProfileController*)currentNavController.visibleViewController;
			if ([userId isEqualToString:currentProfile.userId])
				shouldOpenProfile = NO;
		}
		
		if (shouldOpenProfile)
		{
			OFProfileController* newProfile = (OFProfileController*)OFControllerLoader::load(@"Profile");
			newProfile.userId = userId;
			[currentNavController pushViewController:newProfile animated:YES];
		}
	}	
}

- (IBAction) onFlag
{
	UIViewController* reportAbuseController = [OFReportAbuseController OFReportAbuseControllerForUserId:self.userId];
	
	NSString* backgroundImageName = [OpenFeint isInLandscapeMode] ? @"OpenFeintBackgroundWhiteLandscape.png" : @"OpenFeintBackgroundWhite.png";
	UIView* backgroundView = [[UIImageView alloc] initWithImage:[UIImage imageNamed:backgroundImageName]];
	[reportAbuseController.view addSubview:backgroundView];
	[reportAbuseController.view sendSubviewToBack:backgroundView];
	
	[self presentModalViewController:reportAbuseController animated:YES];
}

- (NSString*)getProfileUserId
{
	return mUserId;
}

- (NSString*)getTableHeaderControllerName
{
	return (mUserId && ![mUserId isEqualToString:[OpenFeint lastLoggedInUserId]]) ? @"ProfileControllerHeader" : nil;
}

- (void)populateResourceMap:(OFResourceControllerMap*)resourceMap
{
	resourceMap->addResource([OFUser class], @"User");
	resourceMap->addResource([OFPlayedGame class], @"PlayedGame");
}

- (void)updatePlayedGamesHeader
{
	if (mUser)
	{
		OFTableSectionDescription* gamesSection = [self getSectionWithIdentifier:@"games"];
		if (gamesSection)
		{
			OFPlayedGamesProfileHeaderView* sectionHeader = (OFPlayedGamesProfileHeaderView*)OFControllerLoader::loadView(@"PlayedGamesProfileHeader");
			[sectionHeader setUser:mUser];
			gamesSection.headerView = sectionHeader;
			[self.tableView reloadData];
		}
	}
}

- (void)onImportFriends
{
	alwaysRefresh = true;
	[self.navigationController pushViewController:OFControllerLoader::load(@"ImportFriends") animated:YES];
}

- (bool)isDisplayingLocalUser
{
	return mUserId == nil || [mUserId isEqualToString:@""] || [mUserId isEqualToString:[OpenFeint lastLoggedInUserId]];
}

- (void)updateNoFriendsHeader
{
	if ([self isDisplayingLocalUser])
	{
		OFTableSectionDescription* gamesSection = [self getSectionWithIdentifier:@"friends"];
		if (gamesSection && [gamesSection.page count] == 0)
		{
			NSMutableArray* missingCredentials = [self getMetaDataOfType:[OFUsersCredential class]];
			if ([missingCredentials count] > 0)
			{
				NSString* networkNames = nil;
				if ([missingCredentials count] > 1)
				{
					networkNames = [NSString stringWithFormat:@"%@ or %@", 
									[OFUsersCredential getDisplayNameForCredentialType:[[missingCredentials objectAtIndex:0] credentialType]],
									[OFUsersCredential getDisplayNameForCredentialType:[[missingCredentials objectAtIndex:1] credentialType]]];
				}
				else
				{
					networkNames = [OFUsersCredential getDisplayNameForCredentialType:[[missingCredentials objectAtIndex:0] credentialType]];
				}
				OFProfileNoFriendsHeaderView* noFriendsView = (OFProfileNoFriendsHeaderView*)OFControllerLoader::loadView(@"ProfileNoFriendsHeader");
				noFriendsView.target = self;
				noFriendsView.importFriendsCallback = @selector(onImportFriends);
				noFriendsView.messageLabel.text = [NSString stringWithFormat:@"Import your friends from %@ to see what OpenFeint games they're playing, compare progress, and more.", networkNames];
				gamesSection.headerView = noFriendsView;
			}
		}
	}
}

- (void)onSectionsCreated
{
	[self updatePlayedGamesHeader];
	[self updateNoFriendsHeader];
}

- (void)onProfileUserLoaded:(OFUser*)profileUser
{
	OFSafeRelease(mUser);
	mUser = [profileUser retain];
	[self updatePlayedGamesHeader];
}

- (OFService*)getService
{
	return [OFProfileService sharedInstance];
}

- (void)doIndexActionOnSuccess:(const OFDelegate&)success onFailure:(const OFDelegate&)failure
{
	[OFProfileService 
	 getProfileForUser:mUserId
	 onSuccess:success
	 onFailure:failure];
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
	else if ([cellResource isKindOfClass:[OFPlayedGame class]])
	{
		OFPlayedGame* playedGameResource = (OFPlayedGame*)cellResource;
		OFAchievementListController* achievementsController = (OFAchievementListController*)OFControllerLoader::load(@"AchievementList");
		achievementsController.userId = [self.userId isEqualToString:[OpenFeint lastLoggedInUserId]] ? nil : self.userId;
		[achievementsController populateContextualDataFromPlayedGame:playedGameResource];
		[self.navigationController pushViewController:achievementsController animated:YES];
	}
}

- (bool)allowPagination
{
	return false;
}

- (bool)shouldAlwaysRefreshWhenShown
{
	return alwaysRefresh;
}

- (NSString*)getNoDataFoundMessage
{
	return @"Failed downloading data.";
}

- (NSString*)getTrailingCellControllerNameForSection:(OFTableSectionDescription*)section
{
	return section.page.header.totalPages > 1 ? @"SingleLabel" : nil;
}

- (void)onTrailingCellWasLoaded:(OFTableCellHelper*)trailingCell forSection:(OFTableSectionDescription*)section
{
	if ([trailingCell isKindOfClass:[OFSingleLabelCell class]])
	{
		OFSingleLabelCell* singleLabelCell = (OFSingleLabelCell*)trailingCell;
		singleLabelCell.accessoryType = UITableViewCellAccessoryDisclosureIndicator;
		singleLabelCell.label.textAlignment = UITextAlignmentCenter;
		singleLabelCell.label.textColor = OFColors::lightBlue;
		singleLabelCell.label.text = [section.identifier isEqualToString:@"games"] ? @"All Games Played" : @"All Friends";
	}
	else
	{
		OFLog(@"Wrong kind of trailing cell for OFProfileController");
	}
}

- (void)onTrailingCellWasClickedForSection:(OFTableSectionDescription*)section
{
	if ([section.identifier isEqualToString:@"games"])
	{
		OFPlayedGameController* playedGameController = (OFPlayedGameController*)OFControllerLoader::load(@"PlayedGame");
		playedGameController.userId = self.userId;
		[self.navigationController pushViewController:playedGameController animated:YES];
	}
	else if ([section.identifier isEqualToString:@"friends"])
	{
		OFFriendsController* friendsController = (OFFriendsController*)OFControllerLoader::load(@"Friends");
		friendsController.userId = self.userId;
		[self.navigationController pushViewController:friendsController animated:YES];
	}
}

- (void)dealloc
{
	OFSafeRelease(mUserId);
	OFSafeRelease(mUser);
	[super dealloc];
}

@end