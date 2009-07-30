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

@interface OFUserCell : OFTableCellHelper
{
	OFImageView* profilePictureView;
	UILabel* nameLabel;
	UILabel* lastPlayedGameLabel;
	UILabel* gamerScoreLabel;
}

@property (nonatomic, retain) IBOutlet OFImageView* profilePictureView;
@property (nonatomic, retain) IBOutlet UILabel* nameLabel;
@property (nonatomic, retain) IBOutlet UILabel* lastPlayedGameLabel;
@property (nonatomic, retain) IBOutlet UILabel* gamerScoreLabel;

- (void)onResourceChanged:(OFResource*)resource;

@end