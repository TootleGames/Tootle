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
#import "OFNavigationController.h"
#import "OFReachability.h"
#import "OpenFeint+Private.h"
#import "OFColors.h"
#import "OFViewHelper.h"
#import "OFChatRoomController.h"
#import "IPhoneOSIntrospection.h"

const UIBarStyle OpenFeintUIBarStyle = UIBarStyleBlackOpaque;
const UIStatusBarStyle OpenFeintStatusBarStyle = UIStatusBarStyleBlackOpaque;
const UIActionSheetStyle OpenFeintActionSheetStyle = UIActionSheetStyleBlackOpaque;

static const float gTabBarSlideDuration = 0.325f;

@implementation OFNavigationController

@synthesize isInHiddenTab = mInHiddenTab;

- (float)_getStatusBarOffset
{
	if(is3PointOhSystemVersion())
	{
		CGRect frame = [UIApplication sharedApplication].statusBarFrame;
		if ([OpenFeint isInLandscapeMode])
		{
			return frame.size.width;
		}
		else
		{
			return frame.size.height;
		}
	}

	return 0.f;
}

- (float)_getNavigationBarOffset
{
	if (self.navigationBarHidden)
		return 0.f;
	else
		return self.navigationBar.frame.size.height;
}

- (void)slideTabBarInController:(UIViewController*)viewController down:(BOOL)down animated:(BOOL)animated
{
	UITabBarController* tabController = viewController.tabBarController;
	
	UITabBar* tabBar = (UITabBar*)OFViewHelper::findViewByClass(tabController.view, [UITabBar class]);
	if (tabBar)
	{
		CGRect tabBarFrame = tabBar.frame;
		float internalViewHeight = 0.0f;

		if (animated)
		{
			[UIView beginAnimations:nil context:nil];
			[UIView setAnimationDuration:gTabBarSlideDuration];
		}

		for (UIView* view in tabController.view.subviews)
		{
			CGRect viewFrame = view.frame;

			if ([view isKindOfClass:[UITabBar class]])
			{
				viewFrame.origin.y += down ? tabBarFrame.size.height : -tabBarFrame.size.height;
			}
			else
			{
				viewFrame.size.height += down ? tabBar.frame.size.height : -tabBar.frame.size.height;
				internalViewHeight = viewFrame.size.height;
			}

			[view setFrame:viewFrame];
		}

		CGRect frame = viewController.view.frame;
		frame.size.height = internalViewHeight - [self _getNavigationBarOffset] - [self _getStatusBarOffset];
		[viewController.view setFrame:frame];
		
		if (animated)
		{
			[UIView commitAnimations];
		}
	}
}

- (void)pushViewController:(UIViewController*)viewController animated:(BOOL)animated
{
	UIViewController* oldTop = self.topViewController;
	
	[super pushViewController:viewController animated:animated];

	if ([oldTop isKindOfClass:[OFChatRoomController class]])
		mIsTabBarHidden = NO;
	else if ([viewController isKindOfClass:[OFChatRoomController class]])
		mIsTabBarHidden = YES;
}

- (UIViewController *)popViewControllerAnimated:(BOOL)animated
{
	UIViewController* newTop = nil;
	if ([self.viewControllers count] >= 2)
	{
		newTop = [self.viewControllers objectAtIndex:([self.viewControllers count] - 2)];
	}
	
	if (newTop && [newTop isKindOfClass:[OFChatRoomController class]])
		mIsTabBarHidden = YES;
	else if ([self.topViewController isKindOfClass:[OFChatRoomController class]])
		mIsTabBarHidden = NO;
	
	return [super popViewControllerAnimated:animated];
}

- (void)addBackgroundAndOverlay
{
	const float kDropShadowHeight = 5.f;
	CGRect actualNavBarFrame = CGRectMake(0.f, 0.f, self.navigationBar.frame.size.width, self.navigationBar.frame.size.height + kDropShadowHeight);
	
	mNavBarBackgroundView = [[UIImageView alloc] initWithImage:[UIImage imageNamed:@"OpenFeintNavBarBackgroundGradient.png"]];
	mNavBarBackgroundView.frame = actualNavBarFrame;
	mNavBarBackgroundView.autoresizingMask = UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleHeight;
	mNavBarBackgroundView.userInteractionEnabled = NO;
	
	[self.navigationBar addSubview:mNavBarBackgroundView];
	[self.navigationBar sendSubviewToBack:mNavBarBackgroundView];
	
	mNavBarOverlayView = [[UIImageView alloc] initWithImage:[UIImage imageNamed:@"OpenFeintNavBarOverlayGradient.png"]];
	mNavBarOverlayView.frame = actualNavBarFrame;
	mNavBarOverlayView.userInteractionEnabled = NO;
	mNavBarOverlayView.autoresizingMask = UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleHeight;
	[self.navigationBar addSubview:mNavBarOverlayView];	
	
}

- (void)viewDidLoad
{
	self.delegate = self;
	self.navigationBar.barStyle = OpenFeintUIBarStyle;
	
	
	// Tint color is only applied to back button
	//self.navigationBar.tintColor = OFColors::middleGreen;
	//[self addBackgroundAndOverlay];
	
}

- (void)addTitleLabelToViewController:(UIViewController*)viewController
{
	UILabel* titleLabel = [[[UILabel alloc] init] autorelease];
	titleLabel.font = [UIFont boldSystemFontOfSize:18.f];
	titleLabel.text = viewController.title;
	titleLabel.textColor = OFColors::darkGreen;
	titleLabel.shadowColor = OFColors::brightGreen;
	titleLabel.lineBreakMode = UILineBreakModeMiddleTruncation;
	const float shadowOffset = 2.f;
	titleLabel.shadowOffset = CGSizeMake(-shadowOffset, shadowOffset);
	titleLabel.backgroundColor = [UIColor clearColor];
	titleLabel.frame = [titleLabel textRectForBounds:CGRectMake(0.f, 0.f, 250.f, 30.f) limitedToNumberOfLines:1];
	viewController.navigationItem.titleView = titleLabel;
}

- (void)navigationController:(UINavigationController *)navigationController willShowViewController:(UIViewController *)viewController animated:(BOOL)animated
{
	//[self addTitleLabelToViewController:viewController];
	[self performSelector:@selector(_orderViewDepths) withObject:nil afterDelay:0.05]; 
}

- (void)navigationController:(UINavigationController *)navigationController didShowViewController:(UIViewController *)viewController animated:(BOOL)animated
{
	if (mIsTabBarHidden != mWasTabBarHidden)
	{
		[self slideTabBarInController:viewController down:mIsTabBarHidden animated:animated];
	}

	mWasTabBarHidden = mIsTabBarHidden;
}

- (void)_orderViewDepths
{
	[self.navigationBar sendSubviewToBack:mNavBarBackgroundView];
	[self.navigationBar bringSubviewToFront:mNavBarOverlayView];
	UIView* titleView = self.visibleViewController.navigationItem.titleView;
	[titleView.superview bringSubviewToFront:titleView];
}

- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation
{
	return interfaceOrientation == [OpenFeint getDashboardOrientation];
}

- (void)dealloc
{
	OFSafeRelease(mNavBarBackgroundView);
	OFSafeRelease(mNavBarOverlayView);
	[super dealloc];
}

@end
