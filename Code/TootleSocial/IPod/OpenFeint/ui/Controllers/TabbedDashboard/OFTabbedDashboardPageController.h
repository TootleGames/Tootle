////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///
///  This is beta software and is subject to changes without notice.
///
///  Do not distribute.
///
///  Copyright (c) 2009 Aurora Feint Inc. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#import "OFProfileFramedNavigationController.h"

@interface OFTabbedDashboardPageController : OFProfileFramedNavigationController

+ (UIViewController*)pageWithController:(NSString*)controllerName;
+ (UIViewController*)pageWithInstantiatedController:(UIViewController*)controller;

@end
