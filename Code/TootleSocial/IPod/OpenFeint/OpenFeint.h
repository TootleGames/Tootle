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

#pragma once

#ifndef __cplusplus
#	error "OpenFeint requires Objective-C++. In XCode, you can enable this by changing your file's extension to .mm"
#endif

#import "OFDependencies.h"
#import "OpenFeintDelegate.h"
#import "OpenFeintSettings.h"
#import "OFCallbackable.h"

@class OFProvider;
@class OFPoller;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///
/// Public OpenFeint API
///
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
@interface OpenFeint : NSObject<UIActionSheetDelegate, OFCallbackable>
{
@private
	id<OpenFeintDelegate> mOriginalDelegate;
	id<OpenFeintDelegate> mLaunchDelegate;
	bool mIsDashboardDismissing;
	UIViewController* mQueuedRootModal;
	UIViewController* mOFRootController;
	NSString* mDisplayName;
	NSString* mShortDisplayName;
	NSString* mResumeIconFileName;
	NSString* mBannerImageNamePortrait;
	NSString* mBannerImageNameLandscape;
	OFProvider* mProvider;
	OFPoller* mPoller;
	UIInterfaceOrientation mPreviousOrientation;
	UIInterfaceOrientation mDashboardOrientation;
	UIStatusBarStyle mPreviousStatusBarStyle;
	bool mPreviousStatusBarHidden;
	NSTimeInterval mPollingFrequencyBeforeResigningActive;
	bool mIsAuthenticationRequired;
	struct sqlite3* mOfflineDatabaseHandle;
	bool mIsErrorPending;
	bool mApprovalLaunchedThroughDashboard;
	bool mAllowErrorScreens;
	bool mSuccessfullyBootstrapped;
}

////////////////////////////////////////////////////////////
///
/// @param productKey is copied. This is your unique product key you received when registering your application.
/// @param productSecret is copied. This is your unique product secret you received when registering your application.
/// @param displayName is copied.
/// @param settings is copied. The available settings are defined as OpenFeintSettingXXXXXXXXXXXX. See OpenFeintSettings.h
/// @param delegate is not retained. This will most likely be the same object as your UIApplicationDelegate.
///
/// @note This will begin the application authorization process.
///
////////////////////////////////////////////////////////////
+ (void) initializeWithProductKey:(NSString*)productKey 
						andSecret:(NSString*)secret
				   andDisplayName:(NSString*)displayName
					  andSettings:(NSDictionary*)settings 
					  andDelegate:(id<OpenFeintDelegate>)delegate;

////////////////////////////////////////////////////////////
///
/// Shuts down OpenFeint
///
////////////////////////////////////////////////////////////
+ (void) shutdown;

////////////////////////////////////////////////////////////
///
/// Launches the OpenFeint Dashboard view at the top of your application's keyed window.
///
/// @note:	If the player has not yet authorized your app, they will be prompted to setup an 
///			account or authorize your application before accessing the OpenFeint dashboard
///
////////////////////////////////////////////////////////////
+ (void) launchDashboard;

////////////////////////////////////////////////////////////
///
/// @see launchDashboard
/// 
/// @param delegate The delegate that is used for this launch. The original delegate will be restored
///					after dismissing the dashboard for use in future calls to launchDashboard.
///
////////////////////////////////////////////////////////////
+ (void) launchDashboardWithDelegate:(id<OpenFeintDelegate>)delegate;

////////////////////////////////////////////////////////////
///
/// Sets what orientation the dashboard and notifications will show in.
///
////////////////////////////////////////////////////////////
+ (void) setDashboardOrientation:(UIInterfaceOrientation)orientation;

////////////////////////////////////////////////////////////
///
/// Removes the OpenFeint Dashboard from your application's keyed window.
///
////////////////////////////////////////////////////////////
+ (void) dismissDashboard;

////////////////////////////////////////////////////////////
///
/// Sets what orientation the dashboard and notifications will show in.
///
////////////////////////////////////////////////////////////
+ (void) setDashboardOrientation:(UIInterfaceOrientation)orientation;

////////////////////////////////////////////////////////////
///
/// @return The version of the OpenFeint client library in use.
///
////////////////////////////////////////////////////////////
+ (NSUInteger)versionNumber;

////////////////////////////////////////////////////////////
///
/// If your application is using a non-portrait layout, you must invoke and return this instead 
/// of directly returning your supported orientations from shouldAutorotateToInterfaceOrientation.
///
/// @example
///		const unsigned int numOrientations = 2;
///		UIInterfaceOrientation myOrientations[numOrientations] = { UIInterfaceOrientationLandscapeLeft, UIInterfaceOrientationLandscapeRight };
///		return [OpenFeint shouldAutorotateToInterfaceOrientation:interfaceOrientation withSupportedOrientations:myOrientations andCount:numOrientations];
///
////////////////////////////////////////////////////////////
+(BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation withSupportedOrientations:(UIInterfaceOrientation*)nullTerminatedOrientations andCount:(unsigned int)numOrientations;

////////////////////////////////////////////////////////////
///
/// These allow OpenFeint to turn off/on background processes when the user is 
/// not interacting with your application. 
///
////////////////////////////////////////////////////////////
+ (void)applicationWillResignActive;
+ (void)applicationDidBecomeActive;

////////////////////////////////////////////////////////////
///
/// Call this method ONLY if overriding the OpenFeint disclosure screen via the delegate method
/// -(BOOL)willLaunchOpenFeintDisclosureScreen. You must pass in whether or not the user has chosen
/// to use OpenFeint in this application.
///
////////////////////////////////////////////////////////////
+ (void)userDidApproveFeint:(BOOL)approved;

////////////////////////////////////////////////////////////
///
/// Call this method ONLY if overriding the OpenFeint disclosure screen via the delegate method
/// -(BOOL)willLaunchOpenFeintDisclosureScreen. You must pass in whether or not the user has chosen
/// to use OpenFeint in this application.
///
////////////////////////////////////////////////////////////
+ (void)userDidApproveFeint:(BOOL)approved;

@end
