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
#import "OFHighScoreCell.h"
#import "OFViewHelper.h"
#import "OFHighScore.h"
#import "OFUser.h"
#import "OFImageView.h"
#import "OpenFeint+UserOptions.h"

@implementation OFHighScoreCell

- (void)onResourceChanged:(OFResource*)resource
{
	OFHighScore* highScore = (OFHighScore*)resource;
		
	OFImageView* profilePictureView = (OFImageView*)OFViewHelper::findViewByTag(self, 1);
	[profilePictureView useProfilePictureFromUser:highScore.user];
	
	UILabel* rankLabel = (UILabel*)OFViewHelper::findViewByTag(self, 2);
	rankLabel.text = [NSString stringWithFormat:@"%d.", highScore.rank];

	UILabel* nameLabel = (UILabel*)OFViewHelper::findViewByTag(self, 3);
	nameLabel.text = highScore.user.name;

	UILabel* lastPlayedGameLabel = (UILabel*)OFViewHelper::findViewByTag(self, 4);
	lastPlayedGameLabel.text = [NSString stringWithFormat:@"Last Played %@", highScore.user.lastPlayedGameName];
	
	UILabel* scoreLabel = (UILabel*)OFViewHelper::findViewByTag(self, 5);
	scoreLabel.text = [NSString stringWithFormat:@"%qi", highScore.score];
}

@end
