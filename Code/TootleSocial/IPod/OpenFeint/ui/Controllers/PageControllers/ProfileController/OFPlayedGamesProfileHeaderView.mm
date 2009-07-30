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
#import "OFImageView.h"
#import "OFPlayedGamesProfileHeaderView.h"
#import "OpenFeint+UserOptions.h"
#import "OFUser.h"
#import "OFProfileController.h"

@implementation OFPlayedGamesProfileHeaderView

@synthesize yourProfilePictureView, otherUserProfilePictureView, sectionNameLabel;

- (void)setUser:(OFUser*)user
{
	bool localUsersPage = !user || [user.resourceId isEqualToString:[OpenFeint lastLoggedInUserId]];
	if (localUsersPage && user)
	{
		[yourProfilePictureView useProfilePictureFromUser:user];
	}
	else
	{
		[yourProfilePictureView useLocalPlayerProfilePictureDefault];
		NSString* imageUrl =[OpenFeint lastLoggedInUserProfilePictureUrl];
		yourProfilePictureView.imageUrl = imageUrl;
		yourProfilePictureView.useFacebookOverlay = [OpenFeint lastLoggedInUserUsesFacebookProfilePicture];
	}
	
	
	otherUserProfilePictureView.hidden = localUsersPage;
	if (!localUsersPage)
	{
		[otherUserProfilePictureView useProfilePictureFromUser:user];
		userId = [user.resourceId retain];
	}
}

- (void)dealloc
{
	self.sectionNameLabel = nil;
	self.yourProfilePictureView = nil;
	self.otherUserProfilePictureView = nil;
	OFSafeRelease(userId);
	[super dealloc];
}

- (IBAction)onClickedYourProfilePicture
{
	[OFProfileController showProfileForUser:[OpenFeint lastLoggedInUserId]];
}

- (IBAction)onClickedOtherProfilePicture
{
	[OFProfileController showProfileForUser:userId];
}

@end
