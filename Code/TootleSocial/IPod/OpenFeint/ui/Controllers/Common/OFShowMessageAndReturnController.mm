////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///
///  This is beta software and is subject to changes without notice.
///
///  Do not distribute.
///
///  Copyright (c) 2009 Aurora Feint Inc. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#import "OFShowMessageAndReturnController.h"
#import "OpenFeint+Private.h"

@implementation OFShowMessageAndReturnController

@synthesize messageLabel;
@synthesize controllerToPopTo;
@synthesize continueButton;

-(IBAction)clickedContinue
{
	if (self.navigationController.parentViewController && self.navigationController.parentViewController.modalViewController != nil)
	{
		[self dismissModalViewControllerAnimated:YES];
	}
	else if (self.controllerToPopTo)
	{
		[self.navigationController popToViewController:self.controllerToPopTo animated:YES];
		self.controllerToPopTo = nil;
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

- (void)dealloc
{
	self.controllerToPopTo = nil;
	self.continueButton = nil;
	self.messageLabel = nil;
	[super dealloc];
}

@end
