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

#include "OFDoBootstrapController.h"

#import "OpenFeint+UserOptions.h"
#import "OpenFeint+Private.h"
#import "OFReachability.h"
#import "OFControllerLoader.h"

#import "OFNewAccountController.h"
#import "OFExistingAccountController.h"

@interface OFDoBootstrapController ()
- (void)dismiss;
- (void)_bootstrapSucceded;
- (void)_bootstrapFailed;
@end

@implementation OFDoBootstrapController

@synthesize keepDashboardOpenOnApproval, messageLabel, activityIndicator;

- (void)viewWillAppear:(BOOL)animated
{
	[OpenFeint allowErrorScreens:NO];

	self.navigationItem.hidesBackButton = YES;
	
	if (!OFReachability::Instance()->isGameServerReachable())
	{
		activityIndicator.hidden = YES;
		messageLabel.text = @"You are offline. OpenFeint requires an internet connection.";
	}
	else
	{
		[OpenFeint doBootstrap:OFDelegate(self, @selector(_bootstrapSucceded)) onFailure:OFDelegate(self, @selector(_bootstrapFailed))];
	}

	[super viewWillAppear:animated];
}

- (void)viewDidAppear:(BOOL)animated
{
	[[self navigationController] setNavigationBarHidden:YES animated:YES];
	[super viewDidAppear:animated];
}

- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)toInterfaceOrientation
{
	return toInterfaceOrientation == [OpenFeint getDashboardOrientation];
}

- (bool)canReceiveCallbacksNow
{
	return true;
}

- (void)dealloc
{
	[super dealloc];
}

- (void)dismiss
{
	[OpenFeint allowErrorScreens:YES];
	[OpenFeint dismissDashboard];
}

- (void)_bootstrapSucceded
{
	bool newAccount = [OpenFeint loggedInUserIsNewUser];
	if (newAccount)
	{
		OFNewAccountController* accountController = (OFNewAccountController*)OFControllerLoader::load(@"NewAccount");
		accountController.keepDashboardOpenOnApproval = keepDashboardOpenOnApproval;
		[[self navigationController] pushViewController:accountController animated:NO];
	}
	else
	{
		OFExistingAccountController* accountController = (OFExistingAccountController*)OFControllerLoader::load(@"ExistingAccount");
		accountController.keepDashboardOpenOnApproval = keepDashboardOpenOnApproval;
		[[self navigationController] pushViewController:accountController animated:NO];
	}
}

- (void)_bootstrapFailed
{
	activityIndicator.hidden = YES;
	messageLabel.text = @"Failed connecting to OpenFeint.";
}

- (IBAction)_skip
{
	[self dismiss];
}

@end
