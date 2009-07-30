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
#import "OpenFeintDelegate.h"
#import "OFControllerLoader.h"
#import "OpenFeintSettings.h"
#import "OFReachability.h"
#import "OFProvider.h"
#import "OpenFeint+Private.h"
#import "OpenFeint+UserOptions.h"
#import <QuartzCore/QuartzCore.h>
#import "OFPoller.h"
#import "OFSettings.h"
#import "OpenFeint+Private.h"
#import "OFNotification.h"
#import "OpenFeint+Settings.h"
#import "OFNavigationController.h"

#import "OFService+Overridables.h"
#import "OFBootstrapService.h"
#import "OFUserSettingService.h"
#import "OFHighScoreService.h"
#import "OFLeaderboardService.h"
#import "OFApplicationDescriptionService.h"
#import "OFChatRoomDefinitionService.h"
#import "OFChatRoomInstanceService.h"
#import "OFChatMessageService.h"
#import "OFProfileService.h"
#import "OFUserService.h"
#import "OFAchievementService.h"
#import "OFUsersCredentialService.h"
#import "OFFriendsService.h"
#import "OFSocialNotificationService.h"

@implementation OpenFeint

+ (NSUInteger)versionNumber
{
	return 7162009;
}

+ (void) initializeWithProductKey:(NSString*)productKey 
						andSecret:(NSString*)productSecret 
				   andDisplayName:(NSString*)displayName
					  andSettings:(NSDictionary*)settings 
					  andDelegate:(id<OpenFeintDelegate>)delegate
{
	[OpenFeint createSharedInstance];
		
	OFControllerLoader::setAssetFileSuffix(@"Of");
	OFControllerLoader::setClassNamePrefix(@"OF");

	OpenFeint* instance = [OpenFeint sharedInstance];
	instance->mOriginalDelegate = delegate;
	instance->mOFRootController = nil;
	instance->mDisplayName = [displayName copy];
	instance->mIsDashboardDismissing = false;
	instance->mIsAuthenticationRequired = false;
	[self intiailizeUserOptions];
	[self intiailizeSettings];

	instance->mIsAuthenticationRequired = [(NSNumber*)[settings objectForKey:OpenFeintSettingRequireAuthorization] boolValue];
	
	NSNumber* dashboardOrientation = [settings objectForKey:OpenFeintSettingDashboardOrientation];
	instance->mDashboardOrientation = dashboardOrientation ? (UIInterfaceOrientation)[dashboardOrientation intValue] : UIInterfaceOrientationPortrait;
	
	NSString* resumeIcon = [settings objectForKey:OpenFeintSettingResumeIcon];
	resumeIcon = resumeIcon ? resumeIcon : @"OpenFeintDefaultResumeIcon.png";
	instance->mResumeIconFileName = [resumeIcon copy];
	
	NSString* bannerImagePortrait = [settings objectForKey:OpenFeintSettingGamePageBannerPortrait];
	instance->mBannerImageNamePortrait = [bannerImagePortrait copy];

	NSString* bannerImageLandscape = [settings objectForKey:OpenFeintSettingGamePageBannerLandscape];
	instance->mBannerImageNameLandscape = [bannerImageLandscape copy];
	
	NSString* shortName = [settings objectForKey:OpenFeintSettingShortDisplayName];
	shortName = shortName ? shortName : displayName;
	instance->mShortDisplayName = [shortName copy];
	
	OFSettings::Initialize();
	OFReachability::Initialize();
			
	instance->mProvider = [[OFProvider providerWithProductKey:productKey andSecret:productSecret] retain];

	instance->mPoller = [[OFPoller alloc] initWithProvider:instance->mProvider];

	[OpenFeint setupOfflineDatabase];

	// Initialize OFServices
	[OFBootstrapService initializeService];
	[OFUserSettingService initializeService];
	[OFHighScoreService initializeService];
	[OFSocialNotificationService initializeService];
	[OFLeaderboardService initializeService];
	[OFApplicationDescriptionService initializeService];
	[OFChatRoomDefinitionService initializeService];
	[OFChatRoomInstanceService initializeService];
	[OFChatMessageService initializeService];	
	[OFProfileService initializeService];
	[OFUserService initializeService];
	[OFAchievementService initializeService];
	[OFUsersCredentialService initializeService];
	[OFFriendsService initializeService];
	
	[[OFHighScoreService sharedInstance] registerPolledResources:instance->mPoller];
	[[OFChatMessageService sharedInstance] registerPolledResources:instance->mPoller];	
			
	OFDelegate noopSuccess;
	OFDelegate noopFailure;
	[self doBootstrap:noopSuccess onFailure:noopFailure];
	
	NSLog(@"Using OpenFeint version %d.%@", [self versionNumber], OFSettings::Instance()->getServerUrl());
}

+ (void) shutdown
{
	// Shutdown services
	[OFFriendsService shutdownService];
	[OFUsersCredentialService shutdownService];
	[OFAchievementService shutdownService];
	[OFUserService shutdownService];
	[OFProfileService shutdownService];
	[OFChatMessageService shutdownService];	
	[OFChatRoomInstanceService shutdownService];
	[OFChatRoomDefinitionService shutdownService];
	[OFApplicationDescriptionService shutdownService];
	[OFLeaderboardService shutdownService];
	[OFHighScoreService shutdownService];
	[OFSocialNotificationService shutdownService];
	[OFUserSettingService shutdownService];
	[OFBootstrapService shutdownService];
	
	[OpenFeint teardownOfflineDatabase];
	[OpenFeint destroySharedInstance];
}

+ (void) setDashboardOrientation:(UIInterfaceOrientation)orientation
{
	OpenFeint* instance = [OpenFeint sharedInstance];
	if (instance)
	{
		instance->mDashboardOrientation = orientation;
	}
}

+ (void) launchDashboardWithDelegate:(id<OpenFeintDelegate>)delegate
{
	OpenFeint* instance = [OpenFeint sharedInstance];
	instance->mLaunchDelegate = delegate;
		
	if(instance->mSuccessfullyBootstrapped && [instance->mProvider isAuthenticated])
	{
		[OpenFeint presentRootControllerWithTabbedDashboard];
	}
	else if (![OpenFeint hasUserApprovedFeint])
	{
		[OpenFeint presentUserFeintApprovalModal:YES];
	}
	else
	{			
		[self launchLoginFlowToDashboard];
	}	
}

+ (void)launchDashboard
{
	[self launchDashboardWithDelegate:nil];
}

+ (bool)canReceiveCallbacksNow
{
	return YES;
}

+ (void)dismissDashboard
{
	[OpenFeint dismissRootController];
}

- (void) dealloc
{
	OFSafeRelease(mDisplayName);
	OFSafeRelease(mShortDisplayName);
	OFSafeRelease(mResumeIconFileName);
	OFSafeRelease(mBannerImageNamePortrait);
	OFSafeRelease(mBannerImageNamePortrait);
	OFSafeRelease(mProvider);
	OFSafeRelease(mPoller);
	OFSafeRelease(mQueuedRootModal);

	if(OFSettings::Instance())
	{
		OFSettings::Instance()->Shutdown();
	}
	
	if(OFReachability::Instance())
	{
		OFReachability::Instance()->Shutdown();
	}
	
	[super dealloc];
}

+(BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation withSupportedOrientations:(UIInterfaceOrientation*)nullTerminatedOrientations andCount:(unsigned int)numOrientations
{
	if([OpenFeint isShowingFullScreen])
	{
		return NO;
	}
	
	for(unsigned int i = 0; i < numOrientations; ++i)
	{
		if(interfaceOrientation == nullTerminatedOrientations[i])
		{
			return YES;
		}
	}
	
	return NO;
}

- (bool)canReceiveCallbacksNow
{
	return true;
}

+ (void)applicationWillResignActive
{
	OpenFeint* instance = [OpenFeint sharedInstance];

	if(!instance)
	{
		return;
	}
	
	instance->mPollingFrequencyBeforeResigningActive = [instance->mPoller getPollingFrequency];
	[instance->mPoller stopPolling];
}

+ (void)applicationDidBecomeActive
{
	OpenFeint* instance = [OpenFeint sharedInstance];
	
	if(!instance)
	{
		return;
	}

	[instance->mPoller changePollingFrequency:instance->mPollingFrequencyBeforeResigningActive];
}

+ (void)userDidApproveFeint:(BOOL)approved
{
	if (approved)
	{
		[OpenFeint setUserApprovedFeint];
		[OpenFeint presentConfirmAccountModal];
	}
	else
	{
		[OpenFeint setUserDeniedFeint];
	}
}

@end
