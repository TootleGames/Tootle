////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///
///  This is beta software and is subject to changes without notice.
///
///  Do not distribute.
///
///  Copyright (c) 2009 Aurora Feint Inc. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#import "OFTableSequenceControllerHelper.h"

@class OFLeaderboard;

@interface OFHighScoreController : OFTableSequenceControllerHelper
{
@package
	OFLeaderboard* leaderboard;
	BOOL globalLeaderboard;
}

- (void)showGlobalLeaderboard;
- (void)showFriendsLeaderboard;

- (void)onCellWasClicked:(OFResource*)cellResource indexPathInTable:(NSIndexPath*)indexPath;

@property (nonatomic, retain) OFLeaderboard* leaderboard;

@end
