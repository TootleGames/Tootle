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
#import "OFPlayedGameCell.h"
#import "OFViewHelper.h"
#import "OFImageView.h"
#import "OFPlayedGame.h"
#import "OFUserGameStat.h"
#import "OFApplicationDescriptionController.h"
#import "OpenFeint+UserOptions.h"

@implementation OFPlayedGameCell

@synthesize iconView, 
			nameLabel, 
			otherUsersStatView, 
			yourStatView, 
			yourGamerScoreLabel, 
			otherUsersGamerScoreLabel, 
			getItNowButton, 
			owner,
			yourGamerScoreIcon,
			otherUsersGamerScoreIcon;

- (void)onResourceChanged:(OFResource*)resource
{
	OFPlayedGame* playedGame = (OFPlayedGame*)resource;
	
	OFAssert([playedGame.userGameStats count] > 0, "all played games should have user stats");
	if ([playedGame.userGameStats count] == 0)
	{
		return;
	}
	
	OFSafeRelease(clientApplicationId);
	clientApplicationId = [[NSString stringWithFormat:@"%d", playedGame.applicationId] retain];
	nameLabel.text = playedGame.name;
	[iconView setDefaultImage:[UIImage imageNamed:@"OFDefaultApplicationIcon.png"]];
	iconView.imageUrl = playedGame.iconUrl;
	
	OFUserGameStat* firstStat = [playedGame.userGameStats objectAtIndex:0];
	OFUserGameStat* secondStat = ([playedGame.userGameStats count] > 1) ? [playedGame.userGameStats objectAtIndex:1] : nil;
	OFUserGameStat* yourStats = [firstStat.userId isEqualToString:[OpenFeint lastLoggedInUserId]] ? firstStat : secondStat;
	OFUserGameStat* otherUsersStats = (yourStats == firstStat) ? secondStat : firstStat;

	yourGamerScoreLabel.text = [NSString stringWithFormat:@"%u", yourStats.userGamerScore];
	bool showGetItNow = !yourStats.userHasGame && playedGame.iconUrl;
	self.yourStatView.hidden = showGetItNow;
	self.yourStatView.userInteractionEnabled = !showGetItNow;
	self.getItNowButton.hidden = !showGetItNow;
	self.getItNowButton.userInteractionEnabled = showGetItNow;
	
	CGRect getItNowRect = self.getItNowButton.frame;
	getItNowRect.size = CGSizeMake(50.f, 50.f);
	self.getItNowButton.frame = getItNowRect;
	yourGamerScoreIcon.image = [UIImage imageNamed:@"OpenFeintGamerScoreIcon.png"];
	otherUsersGamerScoreIcon.image = [UIImage imageNamed:@"OpenFeintGamerScoreIcon.png"];
	if (otherUsersStats)
	{
		otherUsersStatView.hidden = NO;
		otherUsersGamerScoreLabel.text = [NSString stringWithFormat:@"%u", otherUsersStats.userGamerScore];	
		if (!showGetItNow)
		{
			if (otherUsersStats.userGamerScore > yourStats.userGamerScore)
			{
				otherUsersGamerScoreIcon.image = [UIImage imageNamed:@"OpenFeintGamerScoreIconWinner.png"];
			}
			else if (otherUsersStats.userGamerScore < yourStats.userGamerScore)
			{
				yourGamerScoreIcon.image = [UIImage imageNamed:@"OpenFeintGamerScoreIconWinner.png"];
			}
		}
	}
	else
	{
		otherUsersStatView.hidden = YES;
	}
}

- (IBAction)getGameNow
{
	OFApplicationDescriptionController* iPromoteController = [OFApplicationDescriptionController applicationDescriptionForId:clientApplicationId];
	[owner.navigationController pushViewController:iPromoteController animated:YES];
}

- (void)dealloc
{
	OFSafeRelease(clientApplicationId);
	self.nameLabel = nil;
	self.iconView = nil;
	self.otherUsersStatView = nil;
	self.yourGamerScoreLabel = nil;
	self.otherUsersGamerScoreLabel = nil;
	self.getItNowButton = nil;
	self.yourGamerScoreIcon = nil;
	self.otherUsersGamerScoreIcon = nil;
	[super dealloc];
}

@end
