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

#import "OFSocialNotificationService.h"
#import "OFSocialNotificationService+Private.h"
#import "OFHttpNestedQueryStringWriter.h"
#import "OFService+Private.h"
#import "OpenFeint+Private.h"
#import "OpenFeint+UserOptions.h"
#import "OpenFeint+Settings.h"
#import "OFUsersCredentialService.h"
#import "OFUsersCredential.h"
#import "OFPaginatedSeries.h"
#import "OFTableSectionDescription.h"
#import "OFUnlockedAchievement.h"
#import "OFAchievement.h"

@implementation OFSocialNotificationService (Private)

+ (void)_notificationSent
{
}
+ (void)_notificationFailed
{
}

+ (BOOL)canReceiveCallbacksNow
{
	return true;
}
+ (void)onFailure
{
}

+ (BOOL)_hasGlobalPermissionsOnLinkedCredentials:(NSMutableArray*)usersCredentials
{
	NSEnumerator* usersCredentialsEnumerator = [usersCredentials objectEnumerator];
	OFUsersCredential* userCredential = nil;
	while(userCredential = [usersCredentialsEnumerator nextObject])
	{
		if(userCredential.hasGlobalPermissions == true)
		{
			return true;
		}
	}
	return false;
}

+ (void)_requestPermissionToSendSocialNotification:(OFSocialNotification*)socialNotification withCredentialTypes:(NSArray*)credentials
{
	if([OpenFeint userHasRememberedChoiceForNotifications])
	{
		if([OpenFeint userAllowsNotifications])
		{
			[self sendWithoutRequestingPermissionWithSocialNotification:socialNotification];
		}
	}
	else
	{
		[OpenFeint launchRequestUserPermissionForSocialNotification:socialNotification withCredentialTypes:(NSArray*)credentials];
	}
}

+ (void)onSuccess:(OFPaginatedSeries*)usersCredentialsPaginatedResources withSocialNotification:(OFSocialNotification*)socialNotification
{
	OFTableSectionDescription* usersCredentialsTableSection = [usersCredentialsPaginatedResources.objects objectAtIndex:0];
	OFPaginatedSeries* usersCredentialsPaginated = usersCredentialsTableSection.page;
	NSMutableArray* usersCredentials = usersCredentialsPaginated.objects;
	if([self _hasGlobalPermissionsOnLinkedCredentials:usersCredentials])
	{
		[self _requestPermissionToSendSocialNotification:socialNotification withCredentialTypes:usersCredentials];
	}
}


+ (void)sendWithSocialNotification:(OFSocialNotification*)socialNotification
{
	[OFUsersCredentialService getIndexOnSuccess:OFDelegate(self, @selector(onSuccess:withSocialNotification:), socialNotification) 
									  onFailure:OFDelegate(self, @selector(onFailure)) 
				   onlyIncludeLinkedCredentials:true];
}

+ (void)sendWithAchievement:(OFUnlockedAchievement*)achievement
{
	NSString* notificationText = [NSString 
		stringWithFormat:@"I unlocked \"%@\" in \"%@\"!",
		achievement.achievement.title,
		[OpenFeint applicationShortDisplayName]];

	OFSocialNotification* notice = [[[OFSocialNotification alloc] 
		initWithText:notificationText 
		imageType:@"achievement_definitions" 
		imageId:achievement.achievement.resourceId] autorelease];

	notice.imageUrl = achievement.achievement.iconUrl;

	[OFSocialNotificationService sendWithSocialNotification:notice];
}

+ (void)sendWithoutRequestingPermissionWithSocialNotification:(OFSocialNotification*)socialNotification
{
	OFDelegate success = OFDelegate(self, @selector(_notificationSent));	
	OFDelegate failure = OFDelegate(self, @selector(_notificationFailed));
	
	OFPointer<OFHttpNestedQueryStringWriter> params = new OFHttpNestedQueryStringWriter;
	OFRetainedPtr<NSString> msg = socialNotification.text;
	OFRetainedPtr<NSString> image_type = socialNotification.imageType;
	OFRetainedPtr<NSString> image_name_or_id = socialNotification.imageIdentifier;
	params->io("msg", msg);
	params->io("image_type", image_type);
	if([socialNotification.imageType isEqualToString:@"notification_images"])
	{
		params->io("image_name", image_name_or_id);
	}
	else
	{
		params->io("image_id", image_name_or_id);
	}
	[[self sharedInstance]
	 postAction:@"notifications.xml"
	 withParameters:params
	 withSuccess:success
	 withFailure:failure
	 withRequestType:OFActionRequestBackground
	 withNotice:[NSString stringWithFormat:@"Publishing Game Event: %@", socialNotification.text]];
}

@end