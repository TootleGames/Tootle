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
#import "OFComparisonLeadingCell.h"
#import "OFImageView.h"
#import "OFUser.h"
#import "OFProfileController.h"
#import "OpenFeint+UserOptions.h"

@implementation OFComparisonLeadingCell

@synthesize headerLabel, yourProfilePictureView, otherUserProfilePictureView, leftIconView;

- (void)populate:(OFUser*)pageOwner 
		  header:(NSString*)header
	 leftIconUrl:(NSString*)leftIconUrl
leftIconDefaultImage:(NSString*)leftIconDefaultImage
{
	if (leftIconUrl || leftIconDefaultImage)
	{
		if (leftIconDefaultImage)
		{
			[self.leftIconView setDefaultImage:[UIImage imageNamed:leftIconDefaultImage]];
		}
		self.leftIconView.imageUrl = leftIconUrl;
		float headerRightX = self.headerLabel.frame.origin.x + self.headerLabel.frame.size.width;
		CGRect newHeaderRect = self.headerLabel.frame;
		newHeaderRect.origin.x = self.leftIconView.frame.origin.x + self.leftIconView.frame.size.width + 10.f;
		newHeaderRect.size.width = headerRightX - newHeaderRect.origin.x;
		self.headerLabel.frame = newHeaderRect;
	}
	else
	{
		self.leftIconView.hidden = YES;
	}
	
	[self.yourProfilePictureView useLocalPlayerProfilePictureDefault];
	self.yourProfilePictureView.imageUrl = [OpenFeint lastLoggedInUserProfilePictureUrl];
	self.yourProfilePictureView.useFacebookOverlay = [OpenFeint lastLoggedInUserUsesFacebookProfilePicture];
	self.headerLabel.text = header;
	
	pageOwnerId = [pageOwner.resourceId retain];
	
	bool myPage = !pageOwner || [pageOwnerId isEqualToString:[OpenFeint lastLoggedInUserId]];
	if (myPage)
	{
		self.otherUserProfilePictureView.hidden = YES;
	}
	else
	{
		[self.otherUserProfilePictureView useProfilePictureFromUser:pageOwner];
		self.otherUserProfilePictureView.hidden = NO;
	}
}

- (void)dealloc
{
	self.headerLabel = nil;
	self.yourProfilePictureView = nil;
	self.otherUserProfilePictureView = nil;
	self.leftIconView = nil;
	OFSafeRelease(pageOwnerId);
	[super dealloc];
}

- (IBAction)onClickedLeftIcon
{
}

- (IBAction)onClickedYourProfilePicture
{
	[OFProfileController showProfileForUser:[OpenFeint lastLoggedInUserId]];
}

- (IBAction)onClickedOtherProfilePicture
{
	[OFProfileController showProfileForUser:pageOwnerId];
}

@end
