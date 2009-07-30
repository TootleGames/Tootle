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
#import "OpenFeint.h"

@interface OpenFeint (UserOptions)

+ (void)intiailizeUserOptions;

+ (void)setDontAutomaticallyPromptLogin;
+ (bool)shouldAutomaticallyPromptLogin;

+ (void)setUserApprovedFeint;
+ (void)setUserDeniedFeint;
+ (bool)hasUserSetFeintAccess;
+ (bool)hasUserApprovedFeint;

+ (void)setLastLoggedInUserId:(NSString*)userId;
+ (NSString*)lastLoggedInUserId;

+ (void)setLastLoggedInUserProfilePictureUrl:(NSString*)profilePictureUrl;
+ (NSString*)lastLoggedInUserProfilePictureUrl;

+ (void)setLastLoggedInUserUsesFacebookProfilePicture:(BOOL)usesFacebookProfilePicture;
+ (BOOL)lastLoggedInUserUsesFacebookProfilePicture;

+ (void)setLastLoggedInUserName:(NSString*)userName;
+ (NSString*)lastLoggedInUserName;
+ (void)setLoggedInUserHasSetName:(BOOL)hasSetName;
+ (BOOL)lastLoggedInUserHasSetName;
+ (void)setLoggedInUserHasNonDeviceCredential:(BOOL)hasNonDeviceCredential;
+ (BOOL)loggedInUserHasNonDeviceCredential;
+ (void)setLoggedInUserIsNewUser:(BOOL)isNewUser;
+ (BOOL)loggedInUserIsNewUser;
+ (void)setLoggedInUserHadFriendsOnBootup:(BOOL)hadFriends;
+ (BOOL)lastLoggedInUserHadFriendsOnBootup;

+ (void)setClientApplicationId:(NSString*)clientApplicationId;
+ (NSString*)clientApplicationId;

+ (void)setClientApplicationIconUrl:(NSString*)clientApplicationIconUrl;
+ (NSString*)clientApplicationIconUrl;

+ (void)setUserHasRememberedChoiceForNotifications:(BOOL)hasRememberedChoice;
+ (BOOL)userHasRememberedChoiceForNotifications;

+ (void)setUserAllowsNotifications:(BOOL)choice;
+ (BOOL)userAllowsNotifications;

@end

