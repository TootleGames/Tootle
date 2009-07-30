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
@class OFUser;

@interface OFComparisonLeadingCell : OFTableCellHelper
{
	UILabel* headerLabel;
	OFImageView* yourProfilePictureView;
	OFImageView* otherUserProfilePictureView;
	OFImageView* leftIconView;
	
	NSString* pageOwnerId;
}

- (void)populate:(OFUser*)pageOwner 
		  header:(NSString*)header
	 leftIconUrl:(NSString*)leftIconUrl
leftIconDefaultImage:(NSString*)leftIconDefaultImage;

- (IBAction)onClickedLeftIcon;
- (IBAction)onClickedYourProfilePicture;
- (IBAction)onClickedOtherProfilePicture;

@property (nonatomic, retain) IBOutlet UILabel* headerLabel;
@property (nonatomic, retain) IBOutlet OFImageView* yourProfilePictureView;
@property (nonatomic, retain) IBOutlet OFImageView* otherUserProfilePictureView;
@property (nonatomic, retain) IBOutlet OFImageView* leftIconView;

@end