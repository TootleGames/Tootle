////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///
///  This is beta software and is subject to changes without notice.
///
///  Do not distribute.
///
///  Copyright (c) 2009 Aurora Feint Inc. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#import "OFTableCellHelper.h"
@class OFImageView;

@interface OFAchievementListCell : OFTableCellHelper
{
	UILabel* titleLabel;
	UILabel* descriptionLabel;
	UIView* yourIconContainer;
	UIView* otherUsersIconContainer;
	OFImageView* yourUnlockedIcon;
	OFImageView* otherUsersUnlockedIcon;
	UIView* gamerScoreContainer;
	UILabel* gamerScoreLabel;
}

@property (nonatomic, retain) IBOutlet UILabel* titleLabel;
@property (nonatomic, retain) IBOutlet UILabel* descriptionLabel;
@property (nonatomic, retain) IBOutlet UIView* yourIconContainer;
@property (nonatomic, retain) IBOutlet UIView* otherUsersIconContainer;
@property (nonatomic, retain) IBOutlet OFImageView* yourUnlockedIcon;
@property (nonatomic, retain) IBOutlet OFImageView* otherUsersUnlockedIcon;
@property (nonatomic, retain) IBOutlet UIView* gamerScoreContainer;
@property (nonatomic, retain) IBOutlet UILabel* gamerScoreLabel;

- (void)onResourceChanged:(OFResource*)resource;

@end
