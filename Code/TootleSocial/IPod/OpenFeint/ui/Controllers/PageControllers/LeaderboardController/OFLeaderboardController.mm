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
#import "OFLeaderboardController.h"
#import "OFResourceControllerMap.h"
#import "OFLeaderboard.h"
#import "OFControllerLoader.h"
#import "OFProfileController.h"
#import "OFLeaderboardService.h"
#import "OpenFeint+Settings.h"
#import "OFHighScoreController.h"

@implementation OFLeaderboardController

@synthesize applicationId;

- (void)dealloc
{
	OFSafeRelease(applicationId);
	[super dealloc];
}

- (void)populateResourceMap:(OFResourceControllerMap*)resourceMap
{
	resourceMap->addResource([OFLeaderboard class], @"Leaderboard");
}

- (OFService*)getService
{
	return [OFLeaderboardService sharedInstance];
}

- (void)onCellWasClicked:(OFResource*)cellResource indexPathInTable:(NSIndexPath*)indexPath
{
	OFHighScoreController* highScoreController = (OFHighScoreController*)OFControllerLoader::load(@"HighScore");
	highScoreController.leaderboard = (OFLeaderboard*)cellResource;
	[[self navigationController] pushViewController:highScoreController animated:YES];
}

- (NSString*)getNoDataFoundMessage
{
	return [NSString stringWithFormat:@"There are no leaderboards for %@", [OpenFeint applicationDisplayName]];
}

- (void)doIndexActionOnSuccess:(const OFDelegate&)success onFailure:(const OFDelegate&)failure;
{
	[OFLeaderboardService getLeaderboardsForApplication:applicationId onSuccess:success onFailure:failure];
}

@end
