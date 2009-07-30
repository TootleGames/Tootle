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

#import "OFDependencies.h"
#import "OFProfileFramedNavigationController.h"
#import "OFProfileFramedView.h"
#import "OFProfileFrame.h"
#import "OFControllerLoader.h"

#import "OpenFeint+Private.h"

@implementation OFProfileFramedNavigationController

- (void)pushViewController:(UIViewController *)viewController animated:(BOOL)animated
{
	NSString* userId = nil;
	if ([viewController conformsToProtocol:@protocol(OFProfileFrame)])
	{
		UIViewController< OFProfileFrame >* controller = viewController;
		userId = [controller getProfileUserId];

		UIView* contentView = viewController.view;

		OFProfileFramedView* profileFramedView = (OFProfileFramedView*)OFControllerLoader::loadView(@"ProfileFramedView");
		[profileFramedView setController:controller];
		[profileFramedView setContentView:contentView];
		[viewController setView:profileFramedView];
	}

	[super pushViewController:viewController animated:animated];
}

- (UIViewController *)popViewControllerAnimated:(BOOL)animated
{
	return [super popViewControllerAnimated:animated];
}

- (NSArray *)popToViewController:(UIViewController *)viewController animated:(BOOL)animated
{
	return [super popToViewController:viewController animated:animated];
}

- (NSArray *)popToRootViewControllerAnimated:(BOOL)animated
{
	return [super popToRootViewControllerAnimated:animated];
}

- (void)navigationController:(UINavigationController *)navigationController didShowViewController:(UIViewController *)viewController animated:(BOOL)animated
{
	[super navigationController:navigationController didShowViewController:viewController animated:animated];
}

@end