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
#import "OFLeaderboardCell.h"
#import "OFViewHelper.h"
#import "OFLeaderboard.h"

@implementation OFLeaderboardCell

- (void)onResourceChanged:(OFResource*)resource
{
	OFLeaderboard* leaderboard = (OFLeaderboard*)resource;

	UILabel* nameLabel = (UILabel*)OFViewHelper::findViewByTag(self, 1);
	nameLabel.text = leaderboard.name;
}

@end
