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
#import "OFAchievementListCell.h"
#import "OFViewHelper.h"
#import "OFAchievement.h"
#import "OFImageView.h"
#import "OpenFeint+UserOptions.h"

@implementation OFAchievementListCell

@synthesize	titleLabel,
			descriptionLabel,
			yourIconContainer,
			otherUsersIconContainer,
			yourUnlockedIcon,
			otherUsersUnlockedIcon,
			gamerScoreContainer,
			gamerScoreLabel;

- (void)onResourceChanged:(OFResource*)resource
{	
	OFAchievement* achievement = (OFAchievement*)resource;

	if (achievement.isUnlocked)
	{
		[yourUnlockedIcon setDefaultImage:[UIImage imageNamed:@"OpenFeintUnlockedAchievementIcon.png"]];
		yourUnlockedIcon.imageUrl = achievement.iconUrl;
	}
	else
	{
		[yourUnlockedIcon setImage:[UIImage imageNamed:@"OpenFeintLockedAchievementIcon.png"]];
	}
	
	if (achievement.comparedToUserId && 
		![achievement.comparedToUserId isEqualToString:@""] && 
		![achievement.comparedToUserId isEqualToString:[OpenFeint lastLoggedInUserId]])
	{
		if (achievement.isUnlockedByComparedToUser)
		{
			if (achievement.isUnlocked)
			{
				[otherUsersUnlockedIcon setDefaultImage:[UIImage imageNamed:@"OpenFeintUnlockedAchievementIcon.png"]];
				otherUsersUnlockedIcon.imageUrl = achievement.iconUrl;
			}
			else
			{
				[otherUsersUnlockedIcon setImage:[UIImage imageNamed:@"OpenFeintUnlockedAchievementIcon.png"]];
			}
		}
		else
		{
			[otherUsersUnlockedIcon setImage:[UIImage imageNamed:@"OpenFeintLockedAchievementIcon.png"]];
		}
	}
	else
	{
		otherUsersIconContainer.hidden = YES;
		CGRect gamerScoreFrame = gamerScoreContainer.frame;
		gamerScoreFrame.origin.x = yourIconContainer.frame.origin.x - gamerScoreFrame.size.width - 5.f;
		gamerScoreContainer.frame = gamerScoreFrame;
		CGRect titleFrame = titleLabel.frame;
		titleFrame.size.width += otherUsersIconContainer.frame.size.width;
		titleLabel.frame = titleFrame;
		CGRect descriptionFrame = descriptionLabel.frame;
		descriptionFrame.size.width = gamerScoreFrame.origin.x - descriptionFrame.origin.x - 10.f;
		descriptionLabel.frame = descriptionFrame;
	}
	
	if (achievement.isSecret && !achievement.isUnlocked)
	{
		titleLabel.text = @"Secret";
		descriptionLabel.text = @"You must unlock this achievement to view its description.";
	}
	else
	{
		titleLabel.text = achievement.title;
		descriptionLabel.text = achievement.description;
		CGRect descriptionFrame = descriptionLabel.frame;
		if([achievement.description length])
		{
			descriptionFrame.size = [achievement.description sizeWithFont:descriptionLabel.font constrainedToSize:CGSizeMake(descriptionFrame.size.width, FLT_MAX)];
			if (descriptionFrame.size.height > descriptionLabel.frame.size.height)
			{
				descriptionLabel.frame = descriptionFrame;
			}
		}
		
		float endY = descriptionFrame.origin.y + descriptionFrame.size.height + 10.f;
		endY = MAX(endY, yourIconContainer.frame.origin.y + yourIconContainer.frame.size.height + yourIconContainer.frame.origin.y);
		CGRect myRect = self.frame;
		myRect.size.height = endY;
		self.frame = myRect;
		[self layoutSubviews];
	}
	
	gamerScoreLabel.text = [NSString stringWithFormat:@"%d", achievement.gamerscore];
}

- (void)dealloc
{
	self.titleLabel = nil;
	self.descriptionLabel = nil;
	self.yourIconContainer = nil;
	self.otherUsersIconContainer = nil;
	self.yourUnlockedIcon = nil;
	self.otherUsersUnlockedIcon = nil;
	self.gamerScoreContainer = nil;
	self.gamerScoreLabel = nil;

	[super dealloc];
}
@end
