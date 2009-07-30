////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///
///  This is beta software and is subject to changes without notice.
///
///  Do not distribute.
///
///  Copyright (c) 2009 Aurora Feint Inc. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#import "OFChangeNameCompleteController.h"
#import "OFViewHelper.h"
#import "OpenFeint+UserOptions.h"
#import "OpenFeint+Private.h"

@implementation OFChangeNameCompleteController

- (void)viewWillAppear:(BOOL)animated
{
	[super viewWillAppear:animated];
	UILabel* newMessageLabel = (UILabel*)OFViewHelper::findViewByTag(self.view, 1);
	newMessageLabel.text = [NSString stringWithFormat:@"Your name is now %@.", [OpenFeint lastLoggedInUserName]];	
}

-(IBAction)clickedContinue
{
	[OpenFeint reloadInactiveTabBars];
	if (self.navigationController.parentViewController && self.navigationController.parentViewController.modalViewController != nil)
	{
		[self dismissModalViewControllerAnimated:YES];
	}
	else if ([self.navigationController.viewControllers count] > 2)
	{
		UIViewController* controllerToPopTo = [self.navigationController.viewControllers objectAtIndex:[self.navigationController.viewControllers count] - 3];
		[self.navigationController popToViewController:controllerToPopTo animated:YES];
	}
	else
	{
		[self.navigationController popToRootViewControllerAnimated:YES];
	}	
}

- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)toInterfaceOrientation
{
	return toInterfaceOrientation == [OpenFeint getDashboardOrientation];
}

@end