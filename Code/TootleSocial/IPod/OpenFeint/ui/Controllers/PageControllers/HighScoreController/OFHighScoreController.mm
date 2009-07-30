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
#import "OFHighScoreController.h"
#import "OFResourceControllerMap.h"
#import "OFHighScore.h"
#import "OFHighScoreService.h"
#import "OFLeaderboard.h"
#import "OFTableSequenceControllerHelper+Overridables.h"
#import "OFUser.h"
#import "OFProfileController.h"
#import "OFControllerLoader.h"

@implementation OFHighScoreController

@synthesize leaderboard;

- (void)viewWillAppear:(BOOL)animated
{
	[super viewWillAppear:animated];
	self.title = leaderboard.name;
}

- (void)populateResourceMap:(OFResourceControllerMap*)resourceMap
{
	resourceMap->addResource([OFHighScore class], @"HighScore");
}

- (OFService*)getService
{
	return [OFHighScoreService sharedInstance];
}

- (void)showGlobalLeaderboard
{
	globalLeaderboard = YES;
	[self showLoadingScreen];
	[self doIndexActionOnSuccess:[self getOnSuccessDelegate] onFailure:[self getOnFailureDelegate]];
}

- (void)showFriendsLeaderboard
{
	globalLeaderboard = NO;
	[self showLoadingScreen];
	[self doIndexActionOnSuccess:[self getOnSuccessDelegate] onFailure:[self getOnFailureDelegate]];
}

- (NSString*)getTableHeaderControllerName
{
	return @"HighScoreHeader";
}

- (NSString*)getNoDataFoundMessage
{
	return globalLeaderboard ? [NSString stringWithFormat:@"There are no posted high scores for %@", leaderboard.name] : 
							   [NSString stringWithFormat:@"None of your friends have posted high scores for \"%@\"", leaderboard.name];
}

- (void)onCellWasClicked:(OFResource*)cellResource indexPathInTable:(NSIndexPath*)indexPath
{
	if ([cellResource isKindOfClass:[OFHighScore class]])
	{
		OFHighScore* highScoreResource = (OFHighScore*)cellResource;
		OFProfileController* profileController = (OFProfileController*)OFControllerLoader::load(@"Profile");
		profileController.userId = highScoreResource.user.resourceId;
		[self.navigationController pushViewController:profileController animated:YES];
	}
}

- (void)dealloc
{
	self.leaderboard = nil;
	[super dealloc];
}

- (void)doIndexActionOnSuccess:(const OFDelegate&)success onFailure:(const OFDelegate&)failure
{
	[self doIndexActionWithPage:1 onSuccess:success onFailure:failure];
}

- (void)doIndexActionWithPage:(unsigned int)oneBasedPageNumber onSuccess:(const OFDelegate&)success onFailure:(const OFDelegate&)failure
{
	[OFHighScoreService getPage:oneBasedPageNumber forLeaderboard:leaderboard.resourceId friendsOnly:!globalLeaderboard onSuccess:success onFailure:failure];	
}

@end
