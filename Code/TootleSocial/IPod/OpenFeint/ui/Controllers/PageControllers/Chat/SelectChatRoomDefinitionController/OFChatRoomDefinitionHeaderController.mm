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
#import "OFChatRoomDefinitionHeaderController.h"
#import "OFViewHelper.h"
#import "OFChatRoomInstance.h"
#import "OFChatRoomInstanceService.h"
#import "OpenFeint+Private.h"

@implementation OFChatRoomDefinitionHeaderController

@synthesize chatRoomDefinitionController;
@synthesize feintLogoView;

- (void)loadLastLoadedRoom
{
	OFDelegate success(self, @selector(updateButton));
	OFDelegate failure;
	[OFChatRoomInstanceService loadLastRoomJoined:success onFailure:failure];
}

- (void)updateButton
{
	UIButton* button = (UIButton*)OFViewHelper::findViewByTag(self.view, 1);
	OFChatRoomInstance* lastRoom = [OFChatRoomInstanceService getCachedLastRoomJoined];
	if (lastRoom)
	{
		NSString* buttonTitle = [NSString stringWithFormat:@"%@ (%d/%d)", lastRoom.roomName, lastRoom.numUsersInRoom, lastRoom.maxNumUsersInRoom];
		[button setTitle:buttonTitle forState:UIControlStateNormal];
		[button setTitle:buttonTitle forState:UIControlStateHighlighted];
		[button setEnabled:true];
	}
	else
	{
		[button setTitle:@"None" forState:UIControlStateDisabled];
		[button setEnabled:false];
	}	
}

- (void)updateBeforeDisplaying
{
	[self updateButton];
	[self loadLastLoadedRoom];
}

- (void)hideFeintLogo
{
	[feintLogoView setHidden:YES];
	
	CGRect labelRect = OFViewHelper::findViewByTag(self.view, 2).frame;
	CGRect buttonRect = OFViewHelper::findViewByTag(self.view, 1).frame;
	float otherViewHeights = buttonRect.origin.y + buttonRect.size.height - labelRect.origin.y;
	
	CGRect myRect = CGRectMake(0.f, 0.f, [OpenFeint getDashboardBounds].size.width, otherViewHeights);
	self.view.frame = myRect;
}

- (void)viewDidLoad
{
	[super viewDidLoad];
	
	CGRect labelRect = OFViewHelper::findViewByTag(self.view, 2).frame;
	CGRect buttonRect = OFViewHelper::findViewByTag(self.view, 1).frame;
	float otherViewHeights = buttonRect.origin.y + buttonRect.size.height - labelRect.origin.y;

	NSString* bannerImageName = @"OpenFeintLogoStripe.png";
	if ([OpenFeint isInLandscapeMode])
	{
		bannerImageName = @"OpenFeintLogoStripeLandscape.png";
	}

	UIImage* bannerImage = [UIImage imageNamed:bannerImageName];
	feintLogoView.image = bannerImage;

	CGRect logoRect = feintLogoView.frame;
	logoRect.size = bannerImage.size;
	feintLogoView.frame = logoRect;

	CGRect myRect = CGRectMake(0.f, 0.f, [OpenFeint getDashboardBounds].size.width, bannerImage.size.height + otherViewHeights);
	self.view.frame = myRect;
	
	[self updateBeforeDisplaying];
}

- (bool)canReceiveCallbacksNow
{
	return self.chatRoomDefinitionController != nil;
}

- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)toInterfaceOrientation
{
	return toInterfaceOrientation == [OpenFeint getDashboardOrientation];
}

- (void)dealloc
{
	self.chatRoomDefinitionController = nil;
	[super dealloc];
}

@end