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
#import "OFRootController.h"
#import "OpenFeint+Private.h"
#import "OFViewHelper.h"

@implementation OFRootController

@synthesize isFullScreen;

- (void)presentModalViewController:(UIViewController *)modalViewController animated:(BOOL)animated
{
	if (self.modalViewController != nil)
		return;
		
	isFullScreen = YES;

	// if my view is not already in the top window, put it there now
	// so presentModalViewController will properly swap the new view with me
	UIWindow* topWindow = [OpenFeint getTopApplicationWindow];
	if (self.view.superview != topWindow)
	{
		[topWindow addSubview:self.view];
	}

	[super presentModalViewController:modalViewController animated:animated];

	UIView* modalView = nil;
	if ([OpenFeint isTargetAndSystemVersionThreeOh])
	{
		modalView = modalViewController.view;
	}
	else
	{
		modalView = modalViewController.view.superview;
	}

	// now re-add my view (the background)
	[self.view removeFromSuperview];
	[topWindow addSubview:self.view];
	[topWindow bringSubviewToFront:modalView];

	// assert that i'm covering the same area as the new modal
	self.view.bounds = modalView.bounds;
	self.view.center = modalView.center;
	self.view.transform = modalView.transform;
}

- (void)dismissModalViewControllerAnimated:(BOOL)animated
{
	if (self.modalViewController == nil)
		return;

	UIView* expectedParent = nil;
	if ([OpenFeint isTargetAndSystemVersionThreeOh])
	{
		expectedParent = [OpenFeint getTopApplicationWindow];
	}
	else
	{
		expectedParent = self.modalViewController.view.superview;
	}
	
	[super dismissModalViewControllerAnimated:animated];

	if (self.view.superview == expectedParent)
	{
		[self.view removeFromSuperview];
		if (![OpenFeint isTargetAndSystemVersionThreeOh])
		{
			[expectedParent removeFromSuperview];
		}
	}

	isFullScreen = NO;
}

- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)toInterfaceOrientation
{
	return toInterfaceOrientation == [OpenFeint getDashboardOrientation];
}

- (void)dealloc
{
	[super dealloc];
}

@end