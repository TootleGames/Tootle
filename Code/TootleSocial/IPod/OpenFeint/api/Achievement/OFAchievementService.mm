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
#import "OFAchievement.h"
#import "OFAchievementService.h"
#import "OFService+Private.h"
#import "OFHttpNestedQueryStringWriter.h"
#import "OpenFeint+Private.h"
#import "OFReachability.h"
#import "OpenFeint+UserOptions.h"
#import "OFAchievementService+Private.h"
#import "OFNotification.h"
#import "OFUnlockedAchievement.h"
#import "OFAchievement.h"
#import "OpenFeint+Settings.h"
#import "OFSocialNotificationService+Private.h"
#import "OFAchievementListController.h"
#import "OFInputResponseOpenDashboard.h"
#import "OFControllerLoader.h"

OPENFEINT_DEFINE_SERVICE_INSTANCE(OFAchievementService)

@implementation OFAchievementService

OPENFEINT_DEFINE_SERVICE(OFAchievementService);

- (void) populateKnownResources:(OFResourceNameMap*)namedResources
{
	namedResources->addResource([OFAchievement getResourceName], [OFAchievement class]);
	namedResources->addResource([OFUnlockedAchievement getResourceName], [OFUnlockedAchievement class]);
}

+ (void) getAchievementsForApplication:(NSString*)applicationId 
						comparedToUser:(NSString*)comparedToUserId 
								  page:(NSUInteger)pageIndex
							 onSuccess:(OFDelegate const&)onSuccess 
							 onFailure:(OFDelegate const&)onFailure
{
	OFPointer<OFHttpNestedQueryStringWriter> params = new OFHttpNestedQueryStringWriter;
	if ([applicationId length] > 0 && ![applicationId isEqualToString:@"@me"])
	{
		params->io("by_app", applicationId);
	}
	
	if (comparedToUserId)
	{
		params->io("compared_to_user_id", comparedToUserId);
	}
	
	params->io("page", pageIndex);
	int per_page = 25;
	params->io("per_page", per_page);
	
	bool kGetUnlockedInfo = true;
	params->io("get_unlocked_info", kGetUnlockedInfo);

	[[self sharedInstance] 
		getAction:@"client_applications/@me/achievement_definitions.xml"
		withParameters:params
		withSuccess:onSuccess
		withFailure:onFailure
		withRequestType:OFActionRequestForeground
		withNotice:@"Downloading Achievement Information"];
}

- (OFNotificationInputResponse*)_createAchievementInputResponse
{
	OFAchievementListController* achievementList = (OFAchievementListController*)OFControllerLoader::load(@"AchievementList");
	achievementList.applicationName = [OpenFeint applicationDisplayName];
	achievementList.applicationId = [OpenFeint clientApplicationId];
	achievementList.applicationIconUrl = [OpenFeint clientApplicationIconUrl];
	achievementList.doesUserHaveApplication = YES;
	
	OFNotificationInputResponse* inputResponse = [[[OFInputResponseOpenDashboard alloc] 
		initWithTabIndex:3 
		andController:achievementList] autorelease];
		
	return inputResponse;
}

- (void)_onAchievementUnlocked:(OFPaginatedSeries*)page
{
	OFAssert([page count] == 1, "Only expecting one unlocked achievement!");
	
	OFUnlockedAchievement* unlockedAchievement = [page objectAtIndex:0];
	if (unlockedAchievement.result == kUnlockResult_Success)
	{
		OFNotificationInputResponse* inputResponse = [self _createAchievementInputResponse];
		[[OFNotification sharedInstance] showAchievementNotice:unlockedAchievement.achievement withInputResponse:inputResponse];
		[OFSocialNotificationService sendWithAchievement:unlockedAchievement];
	}
}

- (void)_onAchievementUnlockFailed:(OFPaginatedSeries*)page forAchievement:(NSString*)achievementId
{
	NSString* lastLoggedInUser = [OpenFeint lastLoggedInUserId];
	if ([lastLoggedInUser longLongValue] > 0)
	{
		bool success = [OFAchievementService offlineUnlockAchievement:achievementId forUser:lastLoggedInUser];
		if (success)
		{
			OFNotificationInputResponse* inputResponse = [self _createAchievementInputResponse];
			[[OFNotification sharedInstance] showAchievementNotice:nil withInputResponse:inputResponse];
		}
	}
}

+ (void) unlockAchievement:(NSString*)achievementId
{
	OFDelegate onSuccess([self sharedInstance], @selector(_onAchievementUnlocked:));
	OFDelegate onFailure([self sharedInstance], @selector(_onAchievementUnlockFailed:forAchievement:), achievementId);

	OFPointer<OFHttpNestedQueryStringWriter> params = new OFHttpNestedQueryStringWriter;
	OFRetainedPtr<NSString> resourceId = achievementId;
	params->io("achievement_definition_id", resourceId);

	[[self sharedInstance] 
		postAction:@"users/@me/unlocked_achievements.xml"
		withParameters:params
		withSuccess:onSuccess
		withFailure:onFailure
		withRequestType:OFActionRequestSilent
		withNotice:nil];
}

@end