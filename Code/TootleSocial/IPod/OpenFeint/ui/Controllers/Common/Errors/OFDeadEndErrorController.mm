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
#import "OFDeadEndErrorController.h"
#import "OFControllerLoader.h"
#import "OpenFeint.h"
#import "OpenFeint+Private.h"

@implementation OFDeadEndErrorController

@synthesize message = mMessage;
@synthesize messageView = mMessageView;


+ (id)mustBeOnlineErrorWithMessage:(NSString*)errorMessage
{
	OFDeadEndErrorController* deadEndError = (OFDeadEndErrorController*)OFControllerLoader::load(@"MustBeOnlineError");
	deadEndError.message = [NSString stringWithFormat:errorMessage];
	return deadEndError;
}

+ (id)deadEndErrorWithMessage:(NSString*)errorMessage
{
	OFDeadEndErrorController* deadEndError = (OFDeadEndErrorController*)OFControllerLoader::load(@"DeadEndError");
	deadEndError.message = [NSString stringWithFormat:errorMessage];
	return deadEndError;
}

- (void)viewDidLoad
{
	[super viewDidLoad];

	self.navigationItem.rightBarButtonItem = [[[UIBarButtonItem alloc]	
											   initWithBarButtonSystemItem:UIBarButtonSystemItemCancel 
											   target:self
											   action:@selector(dismiss)] autorelease];
	
	CGRect myRect = CGRectMake(0.0f, 0.0f, [UIScreen mainScreen].bounds.size.width, [UIScreen mainScreen].bounds.size.height * 0.4f);
	self.view.frame = myRect;
}

- (void)viewWillAppear:(BOOL)animated
{
	[super viewWillAppear:animated];
	self.title = @"Error";
	mMessageView.text = mMessage;
	mMessageView.font = [UIFont systemFontOfSize:10.0f];
}

- (void)viewDidDisappear:(BOOL)animated
{
	if (![[[OpenFeint getActiveNavigationController] visibleViewController] isKindOfClass:[OFDeadEndErrorController class]])
	{
		[OpenFeint refreshProfileFrameInActiveTab];
	}
}

- (void)dismiss
{
	[OpenFeint dismissDashboard];
}

- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)toInterfaceOrientation
{
	return toInterfaceOrientation == [OpenFeint getDashboardOrientation];
}

- (void)dealloc 
{
	self.message = nil;
	
    [super dealloc];
}


- (void)registerActionsNow
{
}

@end