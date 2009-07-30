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

#import "OpenFeint.h"
#import "OFNotificationStatus.h"
#import "OFSocialNotification.h"

@class OFActionRequest;
@class MPOAuthAPIRequestLoader;
struct sqlite3;

@interface OpenFeint (Private)

+ (OpenFeint*) sharedInstance;
+ (void) createSharedInstance;
+ (void) destroySharedInstance;
+ (id<OpenFeintDelegate>)getDelegate;
+ (UIInterfaceOrientation)getDashboardOrientation;
+ (BOOL)isInLandscapeMode;
+ (CGRect)getDashboardBounds;
+ (NSString*)getTransitionInSubtypeFromOrientation;
+ (NSString*)getTransitionOutSubtypeFromOrientation;
+ (NSString*)getResumeIconFileName;
+ (NSString*)getBannerImageName;

+ (void)loginWasAborted;
+ (void)loginWasCompleted;

+ (void)launchLoginFlowThenDismiss;
+ (void)launchLoginFlowToDashboard;
+ (void)launchLoginFlowForRequest:(OFActionRequest*)request;
+ (void)doBootstrap:(const OFDelegate&)onSuccess onFailure:(const OFDelegate&)onFailure;
+ (void)presentUserFeintApprovalModal:(BOOL)keepDashboardOpenOnApproval;
+ (void)presentConfirmAccountModal;

+ (void)launchGetExtendedCredentialsFlowForRequest:(OFActionRequest*)request withData:(NSData*)data;
+ (void)launchRequestUserPermissionForSocialNotification:(OFSocialNotification*)socialNotification withCredentialTypes:(NSArray*)credentials;
+ (void)launchDashboardWithTab:(int)startingTabIndex andController:(UIViewController*)startingController;

+ (UIWindow*)getTopApplicationWindow;
+ (UIView*)getTopApplicationView;
+ (UIView*) getTopLevelView;
+ (UIViewController*)getRootController;
+ (UINavigationController*)getActiveNavigationController;
+ (void)refreshProfileFrameInActiveTab;
+ (void)reloadInactiveTabBars;
+ (bool)isShowingFullScreen;
+ (bool)isDashboardHubOpen;
+ (void)presentRootControllerWithModal:(UIViewController*)modal;
+ (void)presentRootControllerWithTabbedDashboard;
+ (void)dismissRootController;
+ (void)destroyDashboard;

+ (void)allowErrorScreens:(BOOL)allowed;
+ (BOOL)areErrorScreensAllowed;

+ (void)displayUpgradeRequiredErrorMessage:(NSData*)data;
+ (void)displayErrorMessage:(NSString*)message;
+ (void)displayMustBeOnlineErrorMessage;
+ (void)displayServerMaintenanceNotice:(NSData*)data;

+ (void)dashboardWillAppear;
+ (void)dashboardDidAppear;
+ (void)dashboardWillDisappear;
+ (void)dashboardDidDisappear;

+ (void)setPollingFrequency:(NSTimeInterval)frequency;
+ (void)setPollingToDefaultFrequency;
+ (void)stopPolling;
+ (void)forceImmediatePoll;
+ (void)clearPollingCacheForClassType:(Class)resourceClassType;

+ (void)setupOfflineDatabase;
+ (void)teardownOfflineDatabase;
+ (struct sqlite3*)getOfflineDatabaseHandle;

+ (OFProvider*)provider;
+ (bool)isTargetAndSystemVersionThreeOh;

@end
