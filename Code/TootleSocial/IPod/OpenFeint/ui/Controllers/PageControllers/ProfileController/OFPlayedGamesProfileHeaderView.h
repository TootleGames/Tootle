////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///
///  This is beta software and is subject to changes without notice.
///
///  Do not distribute.
///
///  Copyright (c) 2009 Aurora Feint Inc. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

@class OFImageView;
@class OFUser;
@interface OFPlayedGamesProfileHeaderView : UIView
{
	OFImageView* yourProfilePictureView;
	OFImageView* otherUserProfilePictureView;
	UILabel* sectionNameLabel;
	
	NSString* userId;
}

- (IBAction)onClickedYourProfilePicture;
- (IBAction)onClickedOtherProfilePicture;

@property (nonatomic, retain) IBOutlet OFImageView* yourProfilePictureView;
@property (nonatomic, retain) IBOutlet OFImageView* otherUserProfilePictureView;
@property (nonatomic, retain) IBOutlet UILabel* sectionNameLabel;

- (void)setUser:(OFUser*)user;

@end