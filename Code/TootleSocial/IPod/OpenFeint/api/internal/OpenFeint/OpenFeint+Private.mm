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
#import "OpenFeint+Private.h"
#import "MPOAuthAPIRequestLoader.h"
#import "OFControllerLoader.h"
#import <QuartzCore/QuartzCore.h>
#import "OFPoller.h"
#import "OFProvider.h"
#import "OFPoller.h"
#import "OFDeadEndErrorController.h"
#import "OFReachability.h"
#import "OFServerMaintenanceNoticeController.h"
#import "OpenFeint+UserOptions.h"
#import "OFNotification.h"
#import "OFInputResponseOpenDashboard.h"
#import "OFActionRequest.h"
#import "OFNavigationController.h"
#import "OpenFeint+Settings.h"
#import "OFXmlDocument.h"
#import "OFTransactionalSaveFile.h"
#import <sqlite3.h>
#import "OFAchievementService+Private.h"
#import "OFImageCache.h"
#import "OFUserFeintApprovalController.h"
#import "OFDelegateChained.h"
#import "OFBackgroundView.h"
#import "OFUsersCredential.h"
#import "OFExtendedCredentialController.h"
#import "OFSocialNotificationController.h"
#import "OFProfileFramedView.h"
#import "OFTableControllerHelper.h"
#import "OFRootController.h"
#import "OFDoBootstrapController.h"
#import "IPhoneOSIntrospection.h"

#define OPTIONALLY_INVOKE_DELEGATE(_delegate, _selector)	\
	id<OpenFeintDelegate> delegate = _delegate;				\
	if([delegate respondsToSelector:@selector(_selector)])	\
	{														\
		[delegate performSelector:@selector(_selector)];	\
	}

#define OPTIONALLY_INVOKE_DELEGATE_WITH_PARAMETER(_delegate, _selector, _parameter)	\
	id<OpenFeintDelegate> delegate = _delegate;				\
	if([delegate respondsToSelector:@selector(_selector)])	\
	{														\
		[delegate performSelector:@selector(_selector) withObject:_parameter];	\
	}

namespace
{
	OpenFeint* gInstance = nil;
}

@implementation OpenFeint (Private)

+ (void) createSharedInstance
{
	NSAssert(gInstance == nil, @"Attempting to initialize OpenFeint a second time. This is not allowed.");
	gInstance = [[OpenFeint alloc] init];
}

+ (void)destroySharedInstance
{
	NSAssert(gInstance != nil, @"Attempting to shutdown OpenFeint when it has not been initialized. This is not allowed.");
	[gInstance release];
}

+ (void)loginWasCompleted
{
	[[OFNotification sharedInstance] showBackgroundNotice:[NSString stringWithFormat:@"Logged In To OpenFeint As %@", [OpenFeint lastLoggedInUserName]] andStatus:OFNotificationStatusSuccess andInputResponse:[[OFInputResponseOpenDashboard new] autorelease]];
	
	NSString* userId = [OpenFeint lastLoggedInUserId];
	[[OFAchievementService sharedInstance] performSelector:@selector(userDidLogIn:) withObject:userId];
}

+ (void)loginWasAborted
{
	[[OFNotification sharedInstance] showBackgroundNotice:@"Not Logged In To OpenFeint" andStatus:OFNotificationStatusFailure andInputResponse:nil];	
}

+ (void)launchLoginFlowAndShowDashboard:(bool)shouldShowDashboard withOptionalRequest:(OFActionRequest*)request
{
	const bool isAutomaticallyPrompting = shouldShowDashboard == false;
	const NSString* complicatedErrorOccurred = @"An error occurred. Unable to log in to OpenFeint.";
	
	if(![self isShowingFullScreen])
	{
		if(isAutomaticallyPrompting && ![self shouldAutomaticallyPromptLogin] && !request.failedNotAuthorized)
		{
			[self loginWasAborted];
			return;
		}

		[[OFNotification sharedInstance] showBackgroundNotice:complicatedErrorOccurred andStatus:OFNotificationStatusFailure andInputResponse:nil];
		return;
	}
	
	[self displayErrorMessage:complicatedErrorOccurred];
}

+ (void)launchLoginFlowThenDismiss
{
	[self launchLoginFlowAndShowDashboard:false withOptionalRequest:nil];
}

+ (void)launchLoginFlowToDashboard
{
	[self launchLoginFlowAndShowDashboard:true withOptionalRequest:nil];
}

+ (void)launchLoginFlowForRequest:(OFActionRequest*)request;
{
	[self launchLoginFlowAndShowDashboard:[OpenFeint isDashboardHubOpen] withOptionalRequest:request];
}

- (void) _succeededBootstrapping:(NSObject*)param nextCall:(OFDelegateChained*)nextCall
{
	mSuccessfullyBootstrapped = true;
	[nextCall invoke];
	[OpenFeint loginWasCompleted];
	OPTIONALLY_INVOKE_DELEGATE_WITH_PARAMETER([OpenFeint getDelegate], userLoggedIn:, (id)[OpenFeint lastLoggedInUserId]);	
}

- (void) _failedBootstrapping:(NSObject*)param nextCall:(OFDelegateChained*)nextCall
{
	if(mIsAuthenticationRequired)
	{
		[OpenFeint launchLoginFlowThenDismiss];
	}
	else
	{
		[OpenFeint loginWasAborted];
	}
	[nextCall invoke];
}

+ (void)doBootstrap:(const OFDelegate&)chainedOnSuccess onFailure:(const OFDelegate&)chainedOnFailure
{
	OpenFeint* instance = [OpenFeint sharedInstance];
	if ([instance->mProvider isAuthenticated])
	{
		[OpenFeint setUserApprovedFeint];
	}
	
	if([OpenFeint hasUserApprovedFeint])
	{
		OpenFeint* instance = [OpenFeint sharedInstance];
		[instance->mProvider loginAndBootstrap:OFDelegate(instance, @selector(_succeededBootstrapping:nextCall:), chainedOnSuccess) 
									 onFailure:OFDelegate(instance, @selector(_failedBootstrapping:nextCall:), chainedOnFailure)];
	}
	else if (![OpenFeint hasUserSetFeintAccess])
	{
		[OpenFeint presentUserFeintApprovalModal:NO];
	}
}

+ (void)presentUserFeintApprovalModal:(BOOL)keepDashboardOpenOnApproval
{
	[OpenFeint sharedInstance]->mApprovalLaunchedThroughDashboard = keepDashboardOpenOnApproval;
	
	// developer is overriding the approval screen
	if ([[OpenFeint getDelegate] respondsToSelector:@selector(showCustomOpenFeintApprovalScreen)] &&
		[[OpenFeint getDelegate] showCustomOpenFeintApprovalScreen])
	{
		return;
	}

	OFUserFeintApprovalController* modal = (OFUserFeintApprovalController*)OFControllerLoader::load(@"UserFeintApproval");

	OFNavigationController* navController = [[[OFNavigationController alloc] initWithRootViewController:modal] autorelease];
	[navController setNavigationBarHidden:YES animated:NO];
	[OpenFeint presentRootControllerWithModal:navController];
}

+ (void)presentConfirmAccountModal
{
	OFDoBootstrapController* approvalFlow = (OFDoBootstrapController*)OFControllerLoader::load(@"DoBootstrap");
	approvalFlow.keepDashboardOpenOnApproval = [OpenFeint sharedInstance]->mApprovalLaunchedThroughDashboard;
	
	if ([OpenFeint isShowingFullScreen])
	{
		UINavigationController* navController = [OpenFeint getActiveNavigationController];
		OFAssert(navController, "Must have a navigation controller in this case!");
		[navController pushViewController:approvalFlow animated:YES];
	}
	else
	{
		OFNavigationController* navController = [[[OFNavigationController alloc] initWithRootViewController:approvalFlow] autorelease];
		[navController setNavigationBarHidden:YES animated:NO];
		[OpenFeint presentRootControllerWithModal:navController];
	}
}

+ (BOOL) _extendedCredentialsRequiredFor:(const char*)credentialXmlPath withDocument:(OFXmlDocument*)errorDocument
{
	NSString* message = [errorDocument getElementValue:credentialXmlPath];
	return [message isEqualToString:@"extended_credentials"];
}

+ (void)launchGetExtendedCredentialsFlowForRequest:(OFActionRequest*)request withData:(NSData*)data
{
	OFExtendedCredentialController* modal = (OFExtendedCredentialController*)OFControllerLoader::load(@"ExtendedCredential");
	modal.request = request;
	OFNavigationController* ofNavController = [[[OFNavigationController alloc] initWithRootViewController:modal] autorelease];
	UIBarButtonItem* cancelButton = [[[UIBarButtonItem alloc] initWithTitle:@"Cancel" style:UIBarButtonItemStylePlain target:modal action:@selector(dismiss)] autorelease];
	[ofNavController.navigationBar.topItem setLeftBarButtonItem:cancelButton];
	[ofNavController.navigationBar.topItem setTitle:@"Additional Permissions"];
	OFXmlDocument* errorDocument = [OFXmlDocument xmlDocumentWithData:data];
	if([self _extendedCredentialsRequiredFor:"errors.TwitterCredential" withDocument:errorDocument])
	{
		modal.requireTwitterCredentials = YES;
	}
	if([self _extendedCredentialsRequiredFor:"errors.FbconnectCredential" withDocument:errorDocument])
	{
		modal.requireFacebookCredentials = YES;
	}
	[OpenFeint presentRootControllerWithModal:ofNavController];
}

+ (void)launchRequestUserPermissionForSocialNotification:(OFSocialNotification*)socialNotification withCredentialTypes:(NSArray*)credentials
{
	OFSocialNotificationController* modal = (OFSocialNotificationController*)OFControllerLoader::load(@"SocialNotification");
	modal.socialNotification = socialNotification;
	
	for (OFUsersCredential* credential in credentials)
	{
		if ([credential isFacebook])
			[modal addSocialNetworkIcon:[UIImage imageNamed:@"OFFacebookIcon.png"]];
		else if ([credential isTwitter])
			[modal addSocialNetworkIcon:[UIImage imageNamed:@"OFTwitterIcon.png"]];
	}
	
	OFNavigationController* ofNavController = [[[OFNavigationController alloc] initWithRootViewController:modal] autorelease];
	[OpenFeint presentRootControllerWithModal:ofNavController];
}

- (void)_setTabIndex:(NSArray*)arguments
{
	int tabIndex = 0;
	UIViewController* controllerToPush = nil;

	for (NSObject* arg in arguments)
	{
		if ([arg isKindOfClass:[NSNumber class]])
			tabIndex = [(NSNumber*)arg intValue];
		else if ([arg isKindOfClass:[UIViewController class]])
			controllerToPush = (UIViewController*)arg;
		else
			OFAssert(false, "Unrecognized argument type!");
	}

	OpenFeint* instance = [OpenFeint sharedInstance];
	UIViewController* topController = instance->mOFRootController.modalViewController;
	if([topController isKindOfClass:[UITabBarController class]])
	{
		UITabBarController* tabController = (UITabBarController*)topController;
		tabController.selectedIndex = tabIndex;
		
		if (controllerToPush)
		{
			UINavigationController* tabNavController = (UINavigationController*)[tabController selectedViewController];
			[tabNavController pushViewController:controllerToPush animated:NO];
		}
	}
}

+ (void)launchDashboardWithTab:(int)startingTabIndex andController:(UIViewController*)startingController
{
	[OpenFeint launchDashboard];

	// select the tab and push the controller on the next run loop. this makes things look correct in OS3.0.
	OpenFeint* instance = [OpenFeint sharedInstance];
	[instance 
		performSelector:@selector(_setTabIndex:) 
		withObject:[NSArray arrayWithObjects:[NSNumber numberWithInt:startingTabIndex], startingController, nil] 
		afterDelay:0.01f];
}

+ (void)transformViewToDashboardOrientation:(UIView*)view
{
	UIInterfaceOrientation dashboardOrientation = [OpenFeint sharedInstance]->mDashboardOrientation;

	CGAffineTransform newTransform = CGAffineTransformIdentity;
	CGRect bounds = view.bounds;
	CGPoint center = view.center;
	switch (dashboardOrientation) 
	{
		case UIInterfaceOrientationLandscapeRight:
			newTransform = CGAffineTransformMake(0, 1, -1, 0, 0, 0);
			bounds = CGRectMake(0.f, 0.f, bounds.size.height, bounds.size.width);
			break;
		case UIInterfaceOrientationLandscapeLeft:
			newTransform = CGAffineTransformMake(0, -1, 1, 0, 0, 0);
			bounds = CGRectMake(0.f, 0.f, bounds.size.height, bounds.size.width);
			break;
		case UIInterfaceOrientationPortraitUpsideDown:
			newTransform = CGAffineTransformMake(-1, 0, 0, -1, 0, 0);
			break;
		case UIInterfaceOrientationPortrait:
			newTransform = CGAffineTransformTranslate(newTransform, 0, 0);
			break;
		default:
			OFAssert(false, "invalid dashboard orientation used");
			break;
	}
	
	[view setTransform:newTransform];
	view.bounds = bounds;
	view.center = center;
}

+ (void)presentRootControllerWithModal:(UIViewController*)modal
{
	OpenFeint* instance = [OpenFeint sharedInstance];

	if ([OpenFeint isShowingFullScreen] || (instance->mOFRootController && instance->mIsDashboardDismissing))
	{
		instance->mQueuedRootModal = [modal retain];
		return;
	}

	instance->mIsDashboardDismissing = false;
	
	// jkw: Must be called before the root controller is loaded
	[self dashboardWillAppear];
	
	if (!instance->mOFRootController)
	{
		instance->mOFRootController = [OFControllerLoader::load(@"Root") retain];
	}

	if (![OpenFeint isTargetAndSystemVersionThreeOh])
	{
		[OpenFeint transformViewToDashboardOrientation:instance->mOFRootController.view];
	}
		
	CATransition* animation = [CATransition animation];
	animation.type = kCATransitionMoveIn;
	animation.subtype = [OpenFeint getTransitionInSubtypeFromOrientation];
	animation.duration = 0.5f;
	animation.timingFunction = [CAMediaTimingFunction functionWithName:kCAMediaTimingFunctionEaseInEaseOut];
	animation.delegate = self;
	
	UIWindow* topWindow = [OpenFeint getTopApplicationWindow];
	[topWindow addSubview:instance->mOFRootController.view];
	[[topWindow layer] addAnimation:animation forKey:nil];
	
	[instance->mOFRootController presentModalViewController:modal animated:NO];
}

+ (void)presentRootControllerWithTabbedDashboard
{
	UITabBarController* tabController = (UITabBarController*)OFControllerLoader::load(@"TabbedDashboard");
	if (![OpenFeint lastLoggedInUserHadFriendsOnBootup])
	{
		tabController.selectedIndex = 2;
	}
	[OpenFeint presentRootControllerWithModal:tabController];
}

+ (void)animationDidStop:(CAAnimation *)theAnimation finished:(BOOL)flag
{
	OpenFeint* instance = [OpenFeint sharedInstance];
	if(instance->mIsDashboardDismissing)
	{
		instance->mIsDashboardDismissing = false;
		
		if (instance->mQueuedRootModal)
		{
			[OpenFeint presentRootControllerWithModal:instance->mQueuedRootModal];
			OFSafeRelease(instance->mQueuedRootModal);
		}
		else
		{
			[OpenFeint destroyDashboard];
		}
	}
	else
	{
		[OpenFeint dashboardDidAppear];
	}
}

+ (void)dismissRootController
{
	if (![OpenFeint isShowingFullScreen])
	{
		return;
	}
	
	OpenFeint* instance = [OpenFeint sharedInstance];

	instance->mIsDashboardDismissing = true;
	
	CATransition* animation = [CATransition animation];
	animation.type = kCATransitionReveal;
	animation.subtype = [OpenFeint getTransitionOutSubtypeFromOrientation];
	animation.duration = 0.5f;
	animation.timingFunction = [CAMediaTimingFunction functionWithName:kCAMediaTimingFunctionEaseInEaseOut];
	animation.delegate = self;
	
	[instance->mOFRootController dismissModalViewControllerAnimated:NO];
	[[[OpenFeint getTopApplicationWindow] layer] addAnimation:animation forKey:nil];
		
	[self dashboardWillDisappear];
}
	
+ (UIWindow*)getTopApplicationWindow
{
	UIApplication* clientApp = [UIApplication sharedApplication];
	
	NSAssert([[clientApp windows] count] > 0, @"Your application must have at least one window set before launching OpenFeint");
	UIWindow* topWindow = [clientApp keyWindow];
	if (!topWindow)
	{
		OFLog(@"OpenFeint found no key window. Uses first available window instead."); 
		topWindow = [[clientApp windows] objectAtIndex:0];
	}
	
	return topWindow;
}

+ (UIView*)getTopApplicationView
{
	UIWindow* topWindow = [OpenFeint getTopApplicationWindow];
	return ([topWindow.subviews count] > 0) ? [topWindow.subviews objectAtIndex:0] : nil;
}

+ (id<OpenFeintDelegate>)getDelegate
{
	OpenFeint* instance = [OpenFeint sharedInstance];
	
	if(instance->mLaunchDelegate)
	{
		return instance->mLaunchDelegate;
	}
	
	return instance->mOriginalDelegate;
}

+ (UIInterfaceOrientation)getDashboardOrientation
{
	return [OpenFeint sharedInstance] ? [OpenFeint sharedInstance]->mDashboardOrientation : UIInterfaceOrientationPortrait;
}

+ (BOOL)isInLandscapeMode
{
	UIInterfaceOrientation activeOrientation = [OpenFeint getDashboardOrientation];
	return activeOrientation == UIInterfaceOrientationLandscapeLeft || activeOrientation == UIInterfaceOrientationLandscapeRight;
}

+ (CGRect)getDashboardBounds
{
	OpenFeint* instance = [OpenFeint sharedInstance];
	CGRect dashboardBounds = (instance && instance->mOFRootController) ? instance->mOFRootController.view.frame : CGRectZero;
	if ([OpenFeint isInLandscapeMode])
	{
		dashboardBounds = CGRectMake(dashboardBounds.origin.x, dashboardBounds.origin.y, dashboardBounds.size.height, dashboardBounds.size.width);
	}
	return dashboardBounds;
}

+ (NSString*)getTransitionInSubtypeFromOrientation
{
	NSString* transitionSubtype = kCATransitionFromTop;
	
	UIInterfaceOrientation dashboardOrientation = [OpenFeint sharedInstance]->mDashboardOrientation;
	switch (dashboardOrientation) 
	{
		case UIInterfaceOrientationLandscapeRight:
			transitionSubtype = kCATransitionFromLeft;
			break;
		case UIInterfaceOrientationLandscapeLeft:
			transitionSubtype = kCATransitionFromRight;
			break;
		case UIInterfaceOrientationPortraitUpsideDown:
			transitionSubtype = kCATransitionFromBottom;
			break;
		case UIInterfaceOrientationPortrait:
			transitionSubtype = kCATransitionFromTop;
			break;
		default:
			OFAssert(false, "invalid dashboard orientation used");
			break;
	}
	
	return transitionSubtype;
}

+ (NSString*)getTransitionOutSubtypeFromOrientation
{
	NSString* transitionSubtype = kCATransitionFromTop;
	
	UIInterfaceOrientation dashboardOrientation = [OpenFeint sharedInstance]->mDashboardOrientation;
	switch (dashboardOrientation) 
	{
		case UIInterfaceOrientationLandscapeRight:
			transitionSubtype = kCATransitionFromRight;
			break;
		case UIInterfaceOrientationLandscapeLeft:
			transitionSubtype = kCATransitionFromLeft;
			break;
		case UIInterfaceOrientationPortraitUpsideDown:
			transitionSubtype = kCATransitionFromTop;
			break;
		case UIInterfaceOrientationPortrait:
			transitionSubtype = kCATransitionFromBottom;
			break;
		default:
			OFAssert(false, "invalid dashboard orientation used");
			break;
	}
	
	return transitionSubtype;
}

+ (NSString*)getResumeIconFileName
{
	OpenFeint* instance = [OpenFeint sharedInstance];
	return instance->mResumeIconFileName;
}

+ (NSString*)getBannerImageName
{
	OpenFeint* instance = [OpenFeint sharedInstance];

	if ([OpenFeint isInLandscapeMode])
		return instance->mBannerImageNameLandscape;

	return instance->mBannerImageNamePortrait;
}

+ (void)destroyDashboard
{
	[self dashboardDidDisappear];

	OpenFeint* instance = [OpenFeint sharedInstance];

	OFSafeRelease(instance->mOFRootController);
	
	instance->mLaunchDelegate = nil;
	
	[[OFImageCache sharedInstance] purge];
}

+ (OpenFeint*) sharedInstance
{
	return gInstance;
}

+ (UIView*)getTopLevelView
{
	OpenFeint* instance = [OpenFeint sharedInstance];
	return instance ? [instance->mOFRootController view] : nil;
}

+ (UIViewController*)getRootController
{
	OpenFeint* instance = [OpenFeint sharedInstance];
	return instance ? instance->mOFRootController : nil;
}

+ (bool)isShowingFullScreen
{
	OpenFeint* instance = [OpenFeint sharedInstance];
	if (instance && instance->mOFRootController)
	{
		OFRootController* rootController = (OFRootController*)instance->mOFRootController;
		return rootController.isFullScreen && !instance->mIsDashboardDismissing;
	}

	return false;
}

+ (bool)isDashboardHubOpen
{
	return [self isShowingFullScreen] && [gInstance->mOFRootController.modalViewController isKindOfClass:[UITabBarController class]];
}

+ (UINavigationController*)getActiveNavigationController
{
	OpenFeint* instance = [OpenFeint sharedInstance];	
	
	UIViewController* visibleViewController = instance->mOFRootController.modalViewController;
	
	if([visibleViewController isKindOfClass:[UINavigationController class]])
	{
		return (UINavigationController*)visibleViewController;
	}
	else if([visibleViewController isKindOfClass:[UITabBarController class]])
	{
		UITabBarController* tabBarController = (UITabBarController*)visibleViewController;
		UIViewController* selectedTab = [tabBarController selectedViewController];
		
		if([selectedTab isKindOfClass:[UINavigationController class]])
		{
			return (UINavigationController*)selectedTab;
		}			
	}
		
	return nil;
}

+ (void)refreshProfileFrameInActiveTab
{
	OpenFeint* instance = [OpenFeint sharedInstance];	
	
	UIViewController* topController = instance->mOFRootController.modalViewController;
	if([topController isKindOfClass:[UITabBarController class]])
	{
		UITabBarController* tabController = (UITabBarController*)topController;
		UIViewController* tabPageController = tabController.selectedViewController;
		if ([tabPageController isKindOfClass:[UINavigationController class]])
		{
			UINavigationController* nestedNavController = (UINavigationController*)tabPageController;
			if ([nestedNavController.topViewController.view isKindOfClass:[OFProfileFramedView class]])
			{
				OFProfileFramedView* profileView = (OFProfileFramedView*)nestedNavController.topViewController.view;
				[profileView setController:nestedNavController.topViewController];
			}
		}
	}
}

+ (void)reloadInactiveTabBars
{
	OpenFeint* instance = [OpenFeint sharedInstance];	
	
	UIViewController* topController = instance->mOFRootController.modalViewController;
	if([topController isKindOfClass:[UITabBarController class]])
	{
		UITabBarController* tabController = (UITabBarController*)topController;
		for (UIViewController* tabPageController in tabController.viewControllers)
		{
			if (tabPageController != [tabController selectedViewController] && [tabPageController isKindOfClass:[UINavigationController class]])
			{
				UINavigationController* nestedNavController = (UINavigationController*)tabPageController;
				[nestedNavController popToRootViewControllerAnimated:NO];
				if ([nestedNavController.topViewController isKindOfClass:[OFTableControllerHelper class]])
				{
					OFTableControllerHelper* tableController = (OFTableControllerHelper*)nestedNavController.topViewController;
					[tableController reloadDataFromServer];
					if ([tableController.view isKindOfClass:[OFProfileFramedView class]])
					{
						OFProfileFramedView* profileView = (OFProfileFramedView*)tableController.view;
						[profileView setController:tableController];
					}
				}
			}
		}
	}
}

+ (void)_delayedPushViewController:(UIViewController*)errorController inNavController:(UINavigationController*)activeNavController
{
	[OpenFeint sharedInstance]->mIsErrorPending = NO;
	[activeNavController pushViewController:errorController animated:YES];
}

+ (void)displayErrorMessage:(UIViewController*)errorController inNavController:(UINavigationController*)activeNavController
{
	if ([OpenFeint sharedInstance]->mAllowErrorScreens == NO)
		return;
		
	UIViewController* currentViewController = [activeNavController visibleViewController];
	UIViewController* incomingViewController = (UIViewController*)[activeNavController.viewControllers lastObject];
	
	const bool isTopAnError = [currentViewController isKindOfClass:[OFDeadEndErrorController class]];
	const bool isAnimatingInAnError = [incomingViewController isKindOfClass:[OFDeadEndErrorController class]];
	if(isAnimatingInAnError || isTopAnError || [OpenFeint sharedInstance]->mIsErrorPending)
	{
		return;
	}		

	// citron note: The navigation controller system doesn't behave properly if you push
	//				a view, pop a view, and push again all while it's trying to animate one of its views.
	//				By adding a slight delay here, we prevent the user from spamming actions which cause
	//				visual artifiacts in the navigation bar.
	SEL delayedPushSelector = @selector(_delayedPushViewController:inNavController:);
	NSInvocation* delayedPush = [NSInvocation invocationWithMethodSignature:[self methodSignatureForSelector:delayedPushSelector]];
	[delayedPush setSelector:delayedPushSelector];
	[delayedPush setTarget:self];
	[delayedPush setArgument:&errorController atIndex:2];
	[delayedPush setArgument:&activeNavController atIndex:3];	
	[delayedPush retainArguments];	
	const float enoughTimeForViewControllerAnimationsToPlay = 0.1f;
	[delayedPush performSelector:@selector(invoke) withObject:nil afterDelay:enoughTimeForViewControllerAnimationsToPlay];
	
	[OpenFeint sharedInstance]->mIsErrorPending = YES;
}

+ (void)allowErrorScreens:(BOOL)allowed
{
	OpenFeint* instance = [OpenFeint sharedInstance];
	if (instance)
	{
		instance->mAllowErrorScreens = allowed;
	}
}

+ (BOOL)areErrorScreensAllowed
{
	OpenFeint* instance = [OpenFeint sharedInstance];
	if (instance)
	{
		return instance->mAllowErrorScreens;
	}
	
	return NO;
}

+ (void)displayUpgradeRequiredErrorMessage:(NSData*)data
{
	if([OpenFeint isShowingFullScreen])
	{
		OFXmlDocument* errorDocument = [OFXmlDocument xmlDocumentWithData:data];
		NSString* message = [errorDocument getElementValue:"error_message"];
		message = [NSString stringWithFormat:message, [OpenFeint applicationDisplayName]];
		
		UIViewController* viewController = [OFDeadEndErrorController deadEndErrorWithMessage:message];
		viewController.navigationItem.hidesBackButton = YES;

		UINavigationController* activeNavController = [self getActiveNavigationController];
		[OpenFeint displayErrorMessage:viewController inNavController:activeNavController];
	}
}

+ (void)displayMustBeOnlineErrorMessage
{
	if([OpenFeint isShowingFullScreen])
	{
		UINavigationController* activeNavController = [self getActiveNavigationController];
		[self displayErrorMessage:[OFDeadEndErrorController mustBeOnlineErrorWithMessage:@""] inNavController:activeNavController];
	}
}

+ (void)displayServerMaintenanceNotice:(NSData*)data
{
	if([OpenFeint isShowingFullScreen])
	{
		UINavigationController* activeNavController = [self getActiveNavigationController];
		[self displayErrorMessage:[OFServerMaintenanceNoticeController maintenanceControllerWithHtmlData:data] inNavController:activeNavController];
	}
}

+ (void)displayErrorMessage:(NSString*)message
{
	if(!OFReachability::Instance()->isGameServerReachable())
	{
		[self displayMustBeOnlineErrorMessage];
	}
	else
	{
		if([OpenFeint isShowingFullScreen])
		{
			UINavigationController* activeNavController = [self getActiveNavigationController];
			[self displayErrorMessage:[OFDeadEndErrorController deadEndErrorWithMessage:message] inNavController:activeNavController];
		}
	}
}

+ (OFProvider*)provider
{
	return [self sharedInstance]->mProvider;
}

+ (bool)isTargetAndSystemVersionThreeOh
{
#ifdef __IPHONE_3_0
	return is3PointOhSystemVersion();
#else
	return false;	
#endif
}

+ (void) dashboardDidAppear
{
	[[OpenFeint sharedInstance]->mPoller pollNow];
	OPTIONALLY_INVOKE_DELEGATE([OpenFeint getDelegate], dashboardDidAppear);	
}

+ (void)dashboardDidDisappear
{
	OPTIONALLY_INVOKE_DELEGATE([OpenFeint getDelegate], dashboardDidDisappear);
}

+ (void) dashboardWillAppear
{	
	[OpenFeint sharedInstance]->mPreviousOrientation = [UIApplication sharedApplication].statusBarOrientation;
	[OpenFeint sharedInstance]->mPreviousStatusBarStyle = [UIApplication sharedApplication].statusBarStyle;
	[OpenFeint sharedInstance]->mPreviousStatusBarHidden = [UIApplication sharedApplication].statusBarHidden;
	
	[[UIApplication sharedApplication] setStatusBarStyle:OpenFeintStatusBarStyle animated:YES];
	[[UIApplication sharedApplication] setStatusBarHidden:NO animated:YES];
	[[UIApplication sharedApplication] setStatusBarOrientation:[OpenFeint sharedInstance]->mDashboardOrientation];	
	
	OPTIONALLY_INVOKE_DELEGATE([OpenFeint getDelegate], dashboardWillAppear);
}

+ (void)dashboardWillDisappear
{
	UIApplication* clientApp = [UIApplication sharedApplication];
	
	[clientApp setStatusBarOrientation:[OpenFeint sharedInstance]->mPreviousOrientation animated:YES];
	[clientApp setStatusBarStyle:[OpenFeint sharedInstance]->mPreviousStatusBarStyle animated:YES];
	[clientApp setStatusBarHidden:[OpenFeint sharedInstance]->mPreviousStatusBarHidden animated:YES];
	
	OPTIONALLY_INVOKE_DELEGATE([OpenFeint getDelegate], dashboardWillDisappear);
}

+ (void)setPollingFrequency:(NSTimeInterval)frequency
{
	[[OpenFeint sharedInstance]->mPoller changePollingFrequency:frequency];
}

+ (void)setPollingToDefaultFrequency
{
	[[OpenFeint sharedInstance]->mPoller resetToDefaultPollingFrequency];
}

+ (void)stopPolling
{
	[[OpenFeint sharedInstance]->mPoller stopPolling];
}

+ (void)forceImmediatePoll
{
	[[OpenFeint sharedInstance]->mPoller pollNow];
}

+ (void)clearPollingCacheForClassType:(Class)resourceClassType
{
	[[OpenFeint sharedInstance]->mPoller clearCacheForResourceClass:resourceClassType];
}

+ (void)setupOfflineDatabase
{
	if (SQLITE_OK != sqlite3_open([OFTransactionalSaveFile::getSavePathForFile(@"feint_offline") UTF8String], &[OpenFeint sharedInstance]->mOfflineDatabaseHandle))
	{
		OFAssert(false, "Failed to initialize offline database!");
		[OpenFeint sharedInstance]->mOfflineDatabaseHandle = nil;
	}
}

+ (void)teardownOfflineDatabase
{
	if (SQLITE_BUSY == sqlite3_close([OpenFeint sharedInstance]->mOfflineDatabaseHandle))
	{
		OFAssert(false, "Not all SQLite memory has been released in offline database!");
	}
}

+ (struct sqlite3*)getOfflineDatabaseHandle
{
	return [OpenFeint sharedInstance]->mOfflineDatabaseHandle;
}

@end
