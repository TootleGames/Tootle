////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///
///  Copyright (c) 2009 Aurora Feint Inc.
///
///  This library is free software; you can redistribute it and/or
///  modify it under the terms of the GNU Lesser General Public
///  License as published by the Free Software Foundation; either
///  
///  version 3 of the License, or (at your option) any later version.
///  
///  This library is distributed in the hope that it will be useful,
///  
///  but WITHOUT ANY WARRANTY; without even the implied warranty of
///  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
///  Lesser General Public License for more details.
///  
///  
///  You should have received a copy of the GNU Lesser General Public
///  License along with this library; if not, write to the Free Software
///  
///  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
///
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#import "OFDependencies.h"
#import "OFAchievementNotificationView.h"
#import "OFControllerLoader.h"
#import "OFAchievement.h"
#import "OFImageView.h"

@implementation OFAchievementNotificationView

@synthesize achievementIcon, achievementDetailText, achievementValueText;

+ (void)showAchievementNotice:(OFAchievement*)unlockedAchievement inView:(UIView*)containerView withInputResponse:(OFNotificationInputResponse*)inputResponse
{
	OFAchievementNotificationView* view = (OFAchievementNotificationView*)OFControllerLoader::loadView(@"AchievementNotificationView");
	[view configureWithAchievement:unlockedAchievement inView:containerView withInputResponse:inputResponse];
}

- (void)_iconFinishedDownloading
{
	[self _present];
	float const kDismissalDelay = 5.0f;
	[self performSelector:@selector(_dismiss) withObject:nil afterDelay:kDismissalDelay];
	[achievementIcon setImageDownloadFinishedDelegate:OFDelegate()];
}

- (void)configureWithAchievement:(OFAchievement*)unlockedAchievement inView:(UIView*)containerView withInputResponse:(OFNotificationInputResponse*)inputResponse
{
	mInputResponse = [inputResponse retain];
	
	[self _setPresentationView:containerView];

	notice.text = @"Achievement Unlocked!";
	
	[backgroundImage setContentMode:UIViewContentModeScaleToFill];
	[backgroundImage setImage:[backgroundImage.image stretchableImageWithLeftCapWidth:(backgroundImage.image.size.width - 2) topCapHeight:0]];
	
	[achievementIcon setDefaultImage:[UIImage imageNamed:@"OpenFeintUnlockedAchievementIcon.png"]];

	OFDelegate showAndDismissDelegate(self, @selector(_iconFinishedDownloading));

	if (unlockedAchievement)
	{
		achievement = [unlockedAchievement retain];
		[achievementIcon setImageDownloadFinishedDelegate:showAndDismissDelegate];
		achievementIcon.imageUrl = unlockedAchievement.iconUrl;
		achievementDetailText.text = [NSString stringWithFormat:@"'%@'", achievement.title];
		achievementValueText.text = [NSString stringWithFormat:@"%d", achievement.gamerscore];
	}
	else
	{
		CGRect noticeFrame = notice.frame;
		noticeFrame.origin = CGPointMake(65.0f, 37.0f);
		[notice setFrame:noticeFrame];
		
		statusIndicator.hidden = YES;
		achievementDetailText.hidden = YES;
		achievementValueText.hidden = YES;

		achievement = nil;
		showAndDismissDelegate.invoke();
	}
}

- (void)dealloc 
{
	OFSafeRelease(achievement);
	OFSafeRelease(achievementIcon);
	OFSafeRelease(achievementDetailText);
	OFSafeRelease(achievementValueText);
    [super dealloc];
}

@end