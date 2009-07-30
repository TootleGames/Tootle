////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///
///  This is beta software and is subject to changes without notice.
///
///  Do not distribute.
///
///  Copyright (c) 2009 Aurora Feint Inc. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#import "OFUserFeintApprovalController.h"
#import "OpenFeint+UserOptions.h"
#import "OpenFeint+Private.h"
#import "OpenFeint+Settings.h"
#import "OFLoadingController.h"
#import "OFReachability.h"

@implementation OFUserFeintApprovalController

@synthesize gameNameLabel;

- (void)dismiss
{
	[OpenFeint dismissDashboard];
}

- (void)viewWillAppear:(BOOL)animated
{
	gameNameLabel.text = [OpenFeint applicationDisplayName];
	
	[super viewWillAppear:animated];
}

-(IBAction)clickedUseFeint
{
	[OpenFeint userDidApproveFeint:YES];
}

-(IBAction)clickedDontUseFeint
{
	[OpenFeint userDidApproveFeint:NO];
	[OpenFeint dismissDashboard];
}

- (bool)canReceiveCallbacksNow
{
	return true;
}

- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)toInterfaceOrientation
{
	return toInterfaceOrientation == [OpenFeint getDashboardOrientation];
}

- (void)dealloc
{
	OFSafeRelease(gameNameLabel);
	[super dealloc];
}

@end