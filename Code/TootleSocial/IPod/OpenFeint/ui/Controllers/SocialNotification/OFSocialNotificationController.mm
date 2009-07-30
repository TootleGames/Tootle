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

#import "OFSocialNotificationController.h"
#import "OFSocialNotificationService.h"
#import "OpenFeint+UserOptions.h"
#import "OpenFeint+Settings.h"
#import "OpenFeint+Private.h"
#import "OFSocialNotificationService+Private.h"
#import "OFImageView.h"
#import "OFImageUrl.h"
#import "OFPaginatedSeries.h"

@implementation OFSocialNotificationController

@synthesize socialNotification = mSocialNotification;
@synthesize notificationText = mNotificationText;
@synthesize applicationLabel = mApplicationLabel;
@synthesize notificationImage = mNotificationImage;
@synthesize rememberSwitch = mRememberSwitch;
@synthesize networkIcon1 = mNetworkIcon1;
@synthesize networkIcon2 = mNetworkIcon2;

-(void)addSocialNetworkIcon:(UIImage*)networkIcon
{
	if (mNetworkIcon1.hidden)
	{
		mNetworkIcon1.image = networkIcon;
		mNetworkIcon1.hidden = NO;
	}
	else if (mNetworkIcon2.hidden)
	{
		mNetworkIcon2.image = networkIcon;
		mNetworkIcon2.hidden = NO;
	}
	else
	{
		OFAssert(false, "Too many networks!");
	}
}

- (void)_imageUrlDownloaded:(OFPaginatedSeries*)resources
{
	OFImageUrl* url = [resources.objects objectAtIndex:0];
	mNotificationImage.imageUrl = url.url;
}

- (void)_imageUrlFailed
{
	// TODO: do we need some default image?
}

- (void)viewWillAppear:(BOOL)animated 
{
    [super viewWillAppear:animated];

	mNotificationImage.useSharpCorners = YES;
	
	if (mSocialNotification.imageUrl != nil)
	{
		mNotificationImage.imageUrl = mSocialNotification.imageUrl;
	}
	else
	{
		[mNotificationImage showLoadingIndicator];
		[OFSocialNotificationService 
			getImageUrlForNotificationImageNamed:mSocialNotification.imageIdentifier 
			onSuccess:OFDelegate(self, @selector(_imageUrlDownloaded:))
			onFailure:OFDelegate(self, @selector(_imageUrlFailed))];
	}

	mNotificationText.text = mSocialNotification.text;
	mApplicationLabel.text = [NSString stringWithFormat:@"Allow %@ to publish this event to your streams?", [OpenFeint applicationDisplayName]];
}

-(void)_rememberChoice:(BOOL)choice
{
	if(mRememberSwitch.on)
	{
		[OpenFeint setUserHasRememberedChoiceForNotifications:YES];
		[OpenFeint setUserAllowsNotifications:choice];
	}
}

-(void)dismiss
{
	[OpenFeint dismissDashboard];
}

-(void)yesButtonClicked:(UIButton*)sender
{
	[OFSocialNotificationService sendWithoutRequestingPermissionWithSocialNotification:mSocialNotification];
	[self _rememberChoice:YES];
	[self dismiss];
}

-(void)noButtonClicked:(UIButton*)sender
{
	[self _rememberChoice:NO];
	[self dismiss];
}

- (bool)canReceiveCallbacksNow
{
	return true;
}

- (void)didReceiveMemoryWarning 
{
    [super didReceiveMemoryWarning]; // Releases the view if it doesn't have a superview
    // Release anything that's not essential, such as cached data
}

- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)toInterfaceOrientation
{
	return toInterfaceOrientation == [OpenFeint getDashboardOrientation];
}

- (void)dealloc 
{
	self.socialNotification = nil;
	self.notificationText = nil;
	self.notificationImage = nil;
	self.applicationLabel = nil;
	self.rememberSwitch = nil;
	self.networkIcon1 = nil;
	self.networkIcon2 = nil;
    [super dealloc];
}


@end
