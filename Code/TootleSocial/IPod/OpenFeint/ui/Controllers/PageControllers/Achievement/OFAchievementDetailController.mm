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
#import "OFAchievementDetailController.h"
#import "OFAchievement.h"
#import "OFViewHelper.h"
#import "OFImageView.h"
#import "OpenFeint+Private.h"

@implementation OFAchievementDetailController

@synthesize userId, achievement;

- (NSString*)getProfileUserId
{
	return userId;
}

- (void)dealloc
{
	self.achievement = nil;
	[super dealloc];
}

- (void)viewWillAppear:(BOOL)animated
{
	OFAssert(achievement != nil, "Must have an achievement!");

	OFImageView* iconView = (OFImageView*)OFViewHelper::findViewByTag(self.view, 1);
	if (achievement.isUnlocked)
	{
		iconView.imageUrl = achievement.iconUrl;
	}
	else
	{
		[iconView setImage:[UIImage imageNamed:@"OpenFeintLockedAchievementIcon.png"]];
	}
	
	UILabel* titleLabel = (UILabel*)OFViewHelper::findViewByTag(self.view, 2);
	NSString* titleText = @"Secret";
	
	UILabel* descriptionLabel = (UILabel*)OFViewHelper::findViewByTag(self.view, 3);
	NSString* descriptionText = @"This is a secret achievement. You must unlock this achievement to learn more about it.";
	
	if (!achievement.isSecret || achievement.isUnlocked)
	{
		titleText = achievement.title;
		descriptionText = achievement.description;
	}

	titleLabel.text = titleText;
	
	CGSize textSize = [descriptionText 
		sizeWithFont:[UIFont systemFontOfSize:14.0f] 
		constrainedToSize:descriptionLabel.frame.size 
		lineBreakMode:UILineBreakModeTailTruncation];
	
	CGRect labelRect = descriptionLabel.frame;
	labelRect.size = textSize;
	[descriptionLabel setFrame:labelRect];

	descriptionLabel.numberOfLines = 0;
	descriptionLabel.text = descriptionText;
	
	UILabel* gamerscoreLabel = (UILabel*)OFViewHelper::findViewByTag(self.view, 4);
	gamerscoreLabel.text = [NSString stringWithFormat:@"%d", achievement.gamerscore];
}

- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)toInterfaceOrientation
{
	return toInterfaceOrientation == [OpenFeint getDashboardOrientation];
}

@end