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
#import "OFProfileControllerHeaderController.h"
#import "OFViewHelper.h"
#import "OpenFeint+Private.h"

@implementation OFProfileControllerHeaderController

@synthesize profileController;

- (void)resizeView:(UIView*)parentView
{
	CGRect buttonRect = OFViewHelper::findViewByTag(self.view, 1).frame;
	float viewHeight = buttonRect.origin.y + buttonRect.size.height + buttonRect.origin.y;
	CGRect myRect = CGRectMake(0.0f, 0.0f, parentView.frame.size.width, viewHeight);
	self.view.frame = myRect;
}

- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)toInterfaceOrientation
{
	return toInterfaceOrientation == [OpenFeint getDashboardOrientation];
}

- (void)dealloc
{
	self.profileController = nil;
	[super dealloc];
}

@end