////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///
///  This is beta software and is subject to changes without notice.
///
///  Do not distribute.
///
///  Copyright (c) 2009 Aurora Feint Inc. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#import "OFTableCellHelper.h"
@class OFImageView;

@interface OFPlayedGameCell : OFTableCellHelper
{
	UILabel* nameLabel;
	OFImageView* iconView;
	UIView* yourStatView;
	UIView* otherUsersStatView;
	UILabel* yourGamerScoreLabel;
	UILabel* otherUsersGamerScoreLabel;
	UIButton* getItNowButton;
	UIViewController* owner;
	NSString* clientApplicationId;
	UIImageView* yourGamerScoreIcon;
	UIImageView* otherUsersGamerScoreIcon;
}

@property (nonatomic, retain) IBOutlet OFImageView* iconView;
@property (nonatomic, retain) IBOutlet UILabel* nameLabel;
@property (nonatomic, retain) IBOutlet UIView* otherUsersStatView;
@property (nonatomic, retain) IBOutlet UIView* yourStatView;
@property (nonatomic, retain) IBOutlet UILabel* yourGamerScoreLabel;
@property (nonatomic, retain) IBOutlet UILabel* otherUsersGamerScoreLabel;
@property (nonatomic, retain) IBOutlet UIButton* getItNowButton;
@property (nonatomic, assign) UIViewController* owner;
@property (nonatomic, retain) IBOutlet UIImageView* yourGamerScoreIcon;
@property (nonatomic, retain) IBOutlet UIImageView* otherUsersGamerScoreIcon;

- (void)onResourceChanged:(OFResource*)resource;

- (IBAction)getGameNow;

@end