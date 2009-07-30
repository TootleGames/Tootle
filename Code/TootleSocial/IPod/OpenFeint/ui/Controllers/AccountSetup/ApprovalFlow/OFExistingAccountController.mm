////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// 
///  Copyright 2009 Aurora Feint, Inc.
/// 
///  Licensed under the Apache License, Version 2.0 (the "License");
///  you may not use this file except in compliance with the License.
///  You may obtain a copy of the License at
///  
///  	http://www.apache.org/licenses/LICENSE-2.0
///  	
///  Unless required by applicable law or agreed to in writing, software
///  distributed under the License is distributed on an "AS IS" BASIS,
///  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
///  See the License for the specific language governing permissions and
///  limitations under the License.
/// 
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "OFExistingAccountController.h"

#import "OpenFeint+Private.h"
#import "OpenFeint+UserOptions.h"
#import "OFControllerLoader.h"
#import "OFAccountLoginController.h"

@interface OFExistingAccountController ()
- (void)popBackToMe;
- (void)dismiss;
@end

@implementation OFExistingAccountController

@synthesize keepDashboardOpenOnApproval,
welcomeBackLabel,
changeNameLabel;

- (void)viewWillAppear:(BOOL)animated
{
	self.navigationItem.hidesBackButton = YES;
	
	welcomeBackLabel.text = [NSString stringWithFormat:@"Welcome Back %@!", [OpenFeint lastLoggedInUserName]];
	changeNameLabel.hidden = [OpenFeint lastLoggedInUserHasSetName];
	[super viewWillAppear:animated];
}

- (void)customAnimateNavigationControllerWithFakeOrientationSupport:(BOOL)animatingIn
{
	if ([OpenFeint isInLandscapeMode])
	{
		if (animatingIn)
		{
			[[self navigationController] setNavigationBarHidden:NO animated:NO];
		}
		else
		{
			[[self navigationController] setNavigationBarHidden:YES animated:YES];
		}
		
		CGRect navBarFrame = [self navigationController].navigationBar.frame;
		navBarFrame.size.width = self.view.frame.size.width;
		navBarFrame.origin.y = animatingIn ? -navBarFrame.size.height : 0.f;
		[self navigationController].navigationBar.frame = navBarFrame;
		
		[UIView beginAnimations:nil context:nil];
		[UIView setAnimationDuration:0.5f];
		navBarFrame.origin.y = animatingIn ? 0.f : -navBarFrame.size.height;
		[self navigationController].navigationBar.frame = navBarFrame;
		[UIView commitAnimations];
	}
	else
	{
		[[self navigationController] setNavigationBarHidden:!animatingIn animated:YES];
	}
}

- (void)viewDidAppear:(BOOL)animated
{
	[self customAnimateNavigationControllerWithFakeOrientationSupport:NO];
	[super viewDidAppear:animated];
}

- (void)viewDidDisappear:(BOOL)animated
{
	[self customAnimateNavigationControllerWithFakeOrientationSupport:YES];
	[super viewDidDisappear:animated];
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
	self.welcomeBackLabel = nil;
	[super dealloc];
}

- (void)popBackToMe
{
	[[self navigationController] popToViewController:self animated:YES];
}

- (void)dismiss
{
	if (!hasBeenDismissed)
	{
		[OpenFeint allowErrorScreens:YES];
		[OpenFeint dismissDashboard];
		
		if (keepDashboardOpenOnApproval)
		{
			[OpenFeint presentRootControllerWithTabbedDashboard];
		}
		
		hasBeenDismissed = YES;		
	}
}

- (IBAction)_ok
{
	[self dismiss];
}

- (IBAction)_thisIsntMe
{
	OFAccountLoginController* accountFlowController = (OFAccountLoginController*)OFControllerLoader::load(@"OpenFeintAccountLogin");
	[accountFlowController setCancelDelegate:OFDelegate(self, @selector(popBackToMe))];
	[accountFlowController setCompletionDelegate:OFDelegate(self, @selector(dismiss))];
	[[self navigationController] pushViewController:accountFlowController animated:YES];
}

@end