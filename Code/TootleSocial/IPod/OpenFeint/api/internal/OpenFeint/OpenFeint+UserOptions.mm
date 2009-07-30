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


#import "OpenFeint+UserOptions.h"

static const NSString* OpenFeintUserOptionShouldAutomaticallyPromptLogin = @"OpenFeintSettingShouldAutomaticallyPromptLogin";
static const NSString* OpenFeintUserOptionLastLoggedInUserId = @"OpenFeintSettingLastLoggedInUserId";
static const NSString* OpenFeintUserOptionLastLoggedInUserProfilePictureUrl = @"OpenFeintUserOptionLastLoggedInUserProfilePictureUrl";
static const NSString* OpenFeintUserOptionLastLoggedInUserUsesFacebookProfilePicture = @"OpenFeintUserOptionLastLoggedInUserUsesFacebookProfilePicture";
static const NSString* OpenFeintUserOptionLastLoggedInUserName = @"OpenFeintSettingLastLoggedInUserName";
static const NSString* OpenFeintUserOptionLastLoggedInUserHasSetName = @"OpenFeintSettingLastLoggedInUserHasSetName";
static const NSString* OpenFeintUserOptionLastLoggedInUserHadSetNameOnBootup = @"OpenFeintSettingLastLoggedInUserHadSetNameOnBootup";
static const NSString* OpenFeintUserOptionLastLoggedInUserNonDeviceCredential = @"OpenFeintSettingLastLoggedInUserHasNonDeviceCredential";
static const NSString* OpenFeintUserOptionLastLoggedInUserIsNewUser = @"OpenFeintSettingsLastLoggedInUserIsNewUser";
static const NSString* OpenFeintUserOptionUserFeintApproval = @"OpenFeintUserOptionUserFeintApproval";
static const NSString* OpenFeintUserOptionClientApplicationId = @"OpenFeintSettingClientApplicationId";
static const NSString* OpenFeintUserOptionClientApplicationIconUrl = @"OpenFeintSettingClientApplicationIconUrl";
static const NSString* OpenFeintUserOptionUserHasRememberedChoiceForNotifications = @"OpenFeintUserOptionUserHasRememberedChoiceForNotifications";
static const NSString* OpenFeintUserOptionUserAllowsNotifications = @"OpenFeintUserOptionUserAllowsNotifications";

@implementation OpenFeint (UserOptions)

+ (void)intiailizeUserOptions
{
	NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
	NSDictionary *appDefaults = [NSDictionary dictionaryWithObjectsAndKeys:
								 @"YES", OpenFeintUserOptionShouldAutomaticallyPromptLogin,
								 @"0", OpenFeintUserOptionLastLoggedInUserId,
								 nil
								 ];

	[defaults registerDefaults:appDefaults];
}

+ (void)setDontAutomaticallyPromptLogin
{
	[[NSUserDefaults standardUserDefaults] setObject:@"NO" forKey:OpenFeintUserOptionShouldAutomaticallyPromptLogin];
}

+ (bool)shouldAutomaticallyPromptLogin
{
	return [[NSUserDefaults standardUserDefaults] boolForKey:OpenFeintUserOptionShouldAutomaticallyPromptLogin];
}

+ (void)setUserApprovedFeint
{
	[[NSUserDefaults standardUserDefaults] setObject:@"YES" forKey:OpenFeintUserOptionUserFeintApproval];
}

+ (void)setUserDeniedFeint
{
	[[NSUserDefaults standardUserDefaults] setObject:@"NO" forKey:OpenFeintUserOptionUserFeintApproval];
}

+ (bool)hasUserSetFeintAccess
{
	return [[NSUserDefaults standardUserDefaults] stringForKey:OpenFeintUserOptionUserFeintApproval] != nil;
}

+ (bool)hasUserApprovedFeint
{
	return [[[NSUserDefaults standardUserDefaults] stringForKey:OpenFeintUserOptionUserFeintApproval] isEqualToString:@"YES"];
}

+ (void)setLastLoggedInUserId:(NSString*)userId
{
	[[NSUserDefaults standardUserDefaults] setObject:userId forKey:OpenFeintUserOptionLastLoggedInUserId];
}

+ (NSString*)lastLoggedInUserId
{
	return [[NSUserDefaults standardUserDefaults] stringForKey:OpenFeintUserOptionLastLoggedInUserId];
}

+ (void)setLastLoggedInUserProfilePictureUrl:(NSString*)profilePictureUrl
{
	[[NSUserDefaults standardUserDefaults] setObject:profilePictureUrl forKey:OpenFeintUserOptionLastLoggedInUserProfilePictureUrl];
}

+ (NSString*)lastLoggedInUserProfilePictureUrl
{
	return [[NSUserDefaults standardUserDefaults] stringForKey:OpenFeintUserOptionLastLoggedInUserProfilePictureUrl];
}

+ (void)setLastLoggedInUserUsesFacebookProfilePicture:(BOOL)usesFacebookProfilePicture
{
	NSString* value = usesFacebookProfilePicture ? @"YES" : @"NO";
	[[NSUserDefaults standardUserDefaults] setObject:value forKey:OpenFeintUserOptionLastLoggedInUserUsesFacebookProfilePicture];
}

+ (BOOL)lastLoggedInUserUsesFacebookProfilePicture
{
	return [[[NSUserDefaults standardUserDefaults] stringForKey:OpenFeintUserOptionLastLoggedInUserUsesFacebookProfilePicture] isEqualToString:@"YES"];
}

+ (void)setLastLoggedInUserName:(NSString*)userName
{
	[[NSUserDefaults standardUserDefaults] setObject:userName forKey:OpenFeintUserOptionLastLoggedInUserName];
}

+ (NSString*)lastLoggedInUserName
{
	return [[NSUserDefaults standardUserDefaults] stringForKey:OpenFeintUserOptionLastLoggedInUserName];
}

+ (void)setLoggedInUserHasSetName:(BOOL)hasSetName
{
	[[NSUserDefaults standardUserDefaults] setObject:[NSNumber numberWithBool:hasSetName] forKey:OpenFeintUserOptionLastLoggedInUserHasSetName];
}

+ (BOOL)lastLoggedInUserHasSetName
{
	NSNumber* asNumber = [[NSUserDefaults standardUserDefaults] objectForKey:OpenFeintUserOptionLastLoggedInUserHasSetName];
	return asNumber && [asNumber boolValue];
}

+ (void)setLoggedInUserHadFriendsOnBootup:(BOOL)hadFriends
{
	[[NSUserDefaults standardUserDefaults] setObject:[NSNumber numberWithBool:hadFriends] forKey:OpenFeintUserOptionLastLoggedInUserHadSetNameOnBootup];
}

+ (BOOL)lastLoggedInUserHadFriendsOnBootup
{
	NSNumber* asNumber = [[NSUserDefaults standardUserDefaults] objectForKey:OpenFeintUserOptionLastLoggedInUserHadSetNameOnBootup];
	return asNumber && [asNumber boolValue];
}

+ (void)setLoggedInUserHasNonDeviceCredential:(BOOL)hasNonDeviceCredential
{
	[[NSUserDefaults standardUserDefaults] setObject:[NSNumber numberWithBool:hasNonDeviceCredential] forKey:OpenFeintUserOptionLastLoggedInUserNonDeviceCredential];
}

+ (BOOL)loggedInUserHasNonDeviceCredential
{
	NSNumber* asNumber = [[NSUserDefaults standardUserDefaults] objectForKey:OpenFeintUserOptionLastLoggedInUserNonDeviceCredential];
	return asNumber && [asNumber boolValue];
}

+ (void)setLoggedInUserIsNewUser:(BOOL)isNewUser
{
	[[NSUserDefaults standardUserDefaults] setObject:[NSNumber numberWithBool:isNewUser] forKey:OpenFeintUserOptionLastLoggedInUserIsNewUser];
}

+ (BOOL)loggedInUserIsNewUser
{
	NSNumber* asNumber = [[NSUserDefaults standardUserDefaults] objectForKey:OpenFeintUserOptionLastLoggedInUserIsNewUser];
	return asNumber && [asNumber boolValue];
}

+ (void)setClientApplicationId:(NSString*)clientApplicationId
{
	[[NSUserDefaults standardUserDefaults] setObject:clientApplicationId forKey:OpenFeintUserOptionClientApplicationId];
}

+ (NSString*)clientApplicationId
{
	return [[NSUserDefaults standardUserDefaults] stringForKey:OpenFeintUserOptionClientApplicationId];
}

+ (void)setClientApplicationIconUrl:(NSString*)clientApplicationIconUrl
{
	[[NSUserDefaults standardUserDefaults] setObject:clientApplicationIconUrl forKey:OpenFeintUserOptionClientApplicationIconUrl];
}

+ (NSString*)clientApplicationIconUrl
{
	return [[NSUserDefaults standardUserDefaults] stringForKey:OpenFeintUserOptionClientApplicationIconUrl];
}

+ (void)setUserHasRememberedChoiceForNotifications:(BOOL)hasRememberedChoice
{
	[[NSUserDefaults standardUserDefaults] setBool:hasRememberedChoice
											forKey:OpenFeintUserOptionUserHasRememberedChoiceForNotifications];
}

+ (BOOL)userHasRememberedChoiceForNotifications
{
	return [[NSUserDefaults standardUserDefaults] boolForKey:OpenFeintUserOptionUserHasRememberedChoiceForNotifications];
}

+ (void)setUserAllowsNotifications:(BOOL)allowed
{
	[[NSUserDefaults standardUserDefaults] setBool:allowed
											forKey:OpenFeintUserOptionUserAllowsNotifications];
}

+ (BOOL)userAllowsNotifications
{
	return [[NSUserDefaults standardUserDefaults] boolForKey:OpenFeintUserOptionUserAllowsNotifications];
}




@end