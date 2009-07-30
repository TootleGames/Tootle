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
#import "OFTabbedDashboardPageController.h"
#import "OpenFeint.h"
#import "OFControllerLoader.h"
#import "OpenFeint+Settings.h"
#import "OFResumeView.h"

@implementation OFTabbedDashboardPageController

+ (UIViewController*)pageWithController:(NSString*)controllerName
{
	OFTabbedDashboardPageController* page = [[OFTabbedDashboardPageController new] autorelease];
	[page pushViewController:OFControllerLoader::load(controllerName) animated:NO];
	return page;
}

+ (UIViewController*)pageWithInstantiatedController:(UIViewController*)controller
{
	OFTabbedDashboardPageController* page = [[OFTabbedDashboardPageController new] autorelease];
	[page pushViewController:controller animated:NO];
	return page;
}

- (void)viewDidLoad
{
	[super viewDidLoad];
}

- (void)navigationController:(UINavigationController *)navigationController willShowViewController:(UIViewController *)viewController animated:(BOOL)animated
{	
	//OFResumeView* customResumeView = [[[OFResumeView alloc] initWithAction:@selector(_dismissDashboard) andTarget:self] autorelease];
	//UIBarButtonItem* resumeBarButton = [[[UIBarButtonItem alloc] initWithCustomView:customResumeView] autorelease];
	viewController.navigationItem.rightBarButtonItem = [[[UIBarButtonItem alloc]	
														 initWithTitle:@"Resume"
														 style:UIBarButtonItemStyleBordered
														 target:self
														 action:@selector(_dismissDashboard)] autorelease];
	[super navigationController:navigationController willShowViewController:viewController animated:animated];
}

- (void)navigationController:(UINavigationController *)navigationController didShowViewController:(UIViewController *)viewController animated:(BOOL)animated
{	
	[super navigationController:navigationController didShowViewController:viewController animated:animated];
}

- (void)_orderViewDepths
{
	[super _orderViewDepths];
	if (self.visibleViewController.navigationItem.rightBarButtonItem)
	{
		[self.navigationBar bringSubviewToFront:self.visibleViewController.navigationItem.rightBarButtonItem.customView];
	}
}
	
- (void)_dismissDashboard
{
	[OpenFeint dismissDashboard];
}

@end
